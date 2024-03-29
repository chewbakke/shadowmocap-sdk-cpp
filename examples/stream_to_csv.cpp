#include <shadowmocap.hpp>

#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/stream_file.hpp>
#include <asio/write.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

using shadowmocap::tcp;

// Utility class to store all of the options to run our data stream.
struct command_line_options {
    /**
      Read the command line tokens and load them into this state. Returns 0 if
      successful, -1 if the command line options are invalid, or 1 if the help
      message should be printed out.
    */
    int parse(int argc, char* argv[]);

    /**
      Print command line usage help for this program. Returns 1 which is
      intended to be the main return code as well.
    */
    int print_help(std::ostream* out, char* program_name);

    /**
      Store an error or informational message about why the parse phase failed.
      This will be shown to the user with additional help so they can correct
      the input parameters.
    */
    std::string message;

    /**
      Stream the formatted data to this file.
    */
    std::string filename = "out.csv";

    /**
      Read N frames and then stop sampling, defaults to 0 which indicates no
      limit and to keep streaming for as long as possible.
    */
    int frames = 0;

    /** Connect to this IP address. */
    std::string host = "127.0.0.1";

    /** Service or port to connect to. */
    std::string service = "32076";

    /** Print this string in between every column. */
    std::string separator = ",";

    /** Print this string in between every row. */
    std::string newline = "\n";

    /**
      Set to true to print out string channel names in the 0th row.
    */
    bool header = true;
};

asio::awaitable<void> read_shadowmocap_datastream_frames(
    command_line_options options, shadowmocap::datastream stream,
    std::chrono::steady_clock::time_point& deadline)
{
    using namespace shadowmocap;
    using namespace std::chrono_literals;

    constexpr auto ItemMask = channel::Lq | channel::c;
    constexpr auto ItemSize = get_channel_mask_dimension(ItemMask);

    asio::stream_file file(
        co_await asio::this_coro::executor, options.filename,
        asio::stream_file::write_only | asio::stream_file::create |
            asio::stream_file::truncate);

    std::ostringstream line;
    line.setf(std::ios_base::fixed, std::ios_base::floatfield);
    line.precision(3);

    int num_frames = 0;
    for (;;) {
        extend_deadline_for(deadline, 1s);

        auto message = co_await read_message(stream);

        int column = 0;

        if (options.header && !stream.names_.empty()) {
            std::vector<std::string> channel_names;
            channel_names.reserve(ItemSize);

            for (auto c : {channel::Lq, channel::c}) {
                // Something like "Lq" or "a"
                const std::string prefix = get_channel_name(c);

                // Expand with axes "w", "x", "y", and "z"
                const auto dim = get_channel_dimension(c);
                if (dim == 1) {
                    channel_names.push_back(prefix);
                } else {
                    if (dim == 4) {
                        channel_names.push_back(prefix + "w");
                    }

                    for (const auto& axis : {"x", "y", "z"}) {
                        channel_names.push_back(prefix + axis);
                    }
                }
            }

            for (auto& name : stream.names_) {
                for (auto& channel_name : channel_names) {
                    if (column++ > 0) {
                        line << options.separator;
                    }

                    // Something like "Hips.ax" or "LeftLeg.Lqw"
                    line << name << "." << channel_name;
                }
            }

            line << options.newline;
            column = 0;

            stream.names_.clear();
        }

        auto view = make_message_list<ItemSize>(message);
        for (auto& item : view) {
            for (auto& value : item.data) {
                if (column++ > 0) {
                    line << options.separator;
                }

                line << value;
            }
        }

        line << options.newline;

        // Capture this row as a string and reset for the next line.
        auto str = line.str();
        line.str("");

        co_await asio::async_write(
            file, asio::buffer(str), asio::use_awaitable);

        // Optionally stop after N frames of data.
        if ((options.frames > 0) && (++num_frames >= options.frames)) {
            break;
        }
    }
}

asio::awaitable<void> read_shadowmocap_datastream(
    command_line_options options, tcp::endpoint endpoint)
{
    using namespace shadowmocap;
    using namespace asio::experimental::awaitable_operators;
    using namespace std::chrono_literals;

    auto stream = co_await open_connection(endpoint);

    // Request a list of channels for this data stream
    {
        const auto xml = make_channel_message(channel::Lq | channel::c);
        co_await write_message(stream, xml);
    }

    std::chrono::steady_clock::time_point deadline{};
    extend_deadline_for(deadline, 1s);

    co_await (
        read_shadowmocap_datastream_frames(
            std::move(options), std::move(stream), deadline) ||
        watchdog(deadline));
}

bool stream_data_to_csv(command_line_options options)
{
    try {
        asio::io_context ctx;

        auto endpoint = *shadowmocap::tcp::resolver(ctx).resolve(
            options.host, options.service);

        co_spawn(
            ctx,
            read_shadowmocap_datastream(
                std::move(options), std::move(endpoint)),
            [](auto ptr) {
                // Propagate exception from the coroutine
                if (ptr) {
                    std::rethrow_exception(ptr);
                }
            });

        ctx.run();

        return true;
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
    }

    return false;
}

int main(int argc, char* argv[])
{
    command_line_options options;
    if (options.parse(argc, argv) != 0) {
        return options.print_help(&std::cerr, *argv);
    }

    // Stream frames to a CSV spreadsheet file.
    if (!stream_data_to_csv(std::move(options))) {
        return -1;
    }

    return 0;
}

int command_line_options::parse(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--file") {
            ++i;
            if (i < argc) {
                filename = argv[i];
            } else {
                message = "Missing required argument for --file";
                return -1;
            }
        } else if (arg == "--frames") {
            ++i;
            if (i < argc) {
                frames = std::stoi(argv[i]);
            } else {
                message = "Missing required argument for --frames";
                return -1;
            }
        } else if (arg == "--header") {
            header = true;
        } else if (arg == "--help") {
            return 1;
        } else {
            message = "Unrecognized option \"" + arg + "\"";
            return -1;
        }
    }

    return 0;
}

int command_line_options::print_help(std::ostream* out, char* program_name)
{
    if (!message.empty()) {
        *out << message << newline << newline;
    }

    *out << "Usage: " << program_name << " [options...]" << newline << newline
         << "Allowed options:" << newline
         << "  --help         show help message" << newline
         << "  --file arg     output file" << newline
         << "  --frames N     read N frames" << newline
         << "  --header       show channel names in the first row" << newline
         << newline;

    return 1;
}
