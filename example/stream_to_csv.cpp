#include <shadowmocap.hpp>

#include <asio/co_spawn.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <asio/io_context.hpp>
#include <asio/stream_file.hpp>

#include <chrono>
#include <iostream>
#include <string>

namespace net = shadowmocap::net;
using net::ip::tcp;

// Utility class to store all of the options to run our data stream.
class command_line_options {
public:
  command_line_options();

  /**
    Read the command line tokens and load them into this state. Returns 0 if
    successful, -1 if the command line options are invalid, or 1 if the help
    message should be printed out.
  */
  int parse(int argc, char **argv);

  /**
    Print command line usage help for this program. Returns 1 which is intended
    to be the main return code as well.
  */
  int print_help(std::ostream *out, char *program_name);

  /**
    Store an error or informational message about why the parse phase failed.
    This will be shown to the user with additional help so they can correct the
    input parameters.
  */
  std::string message;

  /**
    Stream the formatted data to a file, defaults to empty which writes to
    std::cout.
  */
  std::string filename;

  /**
    Read N frames and then stop sampling, defaults to 0 which indicates no limit
    and to keep streaming for as long as possible.
  */
  int frames;

  /** IP address to connect to, defaults to "127.0.0.1". */
  std::string host;

  /** Service to connect to, defaults to port "32076". */
  std::string service;

  /** Print this string in between every column, defaults to ",". */
  std::string separator;

  /** Print this string in between every row, defaults to "\n". */
  std::string newline;

  /**
    Set to true to print out string channel names in the 0th row, defaults to
    false.
  */
  bool header;
}; // class command_line_options

net::awaitable<void> read_shadowmocap_datastream_frames(
  const command_line_options &options, shadowmocap::datastream<tcp> &stream,
  asio::stream_file &file)
{
  constexpr auto NumChannel = 8;

  std::ostringstream line;
  line.setf(std::ios_base::fixed, std::ios_base::floatfield);
  line.precision(3);

  int num_frames = 0;
  for (;;) {
    stream.deadline = std::max(
      stream.deadline,
      std::chrono::steady_clock::now() + std::chrono::seconds(1));

    auto message = co_await read_message<std::string>(stream);

    int column = 0;

    auto view = shadowmocap::make_message_view<NumChannel>(message);
    for (auto &item : view) {
      for (auto &value : item.data) {
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

    co_await net::async_write(file, net::buffer(str), net::use_awaitable);

    // Optionally stop after N frames of data.
    if ((options.frames > 0) && (++num_frames >= options.frames)) {
      break;
    }
  }
}

net::awaitable<void> read_shadowmocap_datastream(
  const command_line_options &options, const tcp::endpoint &endpoint)
{
  using namespace shadowmocap;
  using namespace net::experimental::awaitable_operators;

  auto stream = co_await open_connection(endpoint);

  const std::string xml = "<configurable><Lq/><c/></configurable>";
  co_await write_message(stream, xml);

  asio::stream_file file(
    co_await net::this_coro::executor, options.filename,
    asio::stream_file::write_only | net::stream_file::create |
      net::stream_file::truncate);

  co_await(
    read_shadowmocap_datastream_frames(options, stream, file) ||
    watchdog(stream.deadline));
}

bool stream_data_to_csv(const command_line_options &options)
{
  try {
    net::io_context ctx;

    auto endpoint =
      *shadowmocap::tcp::resolver(ctx).resolve(options.host, options.service);

    co_spawn(
      ctx, read_shadowmocap_datastream(options, endpoint),
      [](auto ptr) {
        // Error function for to propagate exceptions to the caller
        if (ptr) {
          std::rethrow_exception(ptr);
        }
      });

    ctx.run();

    return true;
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
  }

  return false;
}

int main(int argc, char **argv)
{
  command_line_options options;
  if (options.parse(argc, argv) != 0) {
    return options.print_help(&std::cerr, *argv);
  }

  // Stream frames to a CSV spreadsheet file.
  if (!stream_data_to_csv(options)) {
    return -1;
  }

  return 0;
}

command_line_options::command_line_options()
  : message(), filename("out.csv"), frames(), host("127.0.0.1"),
    service("32076"), separator(","), newline("\n"), header(false)
{
}

int command_line_options::parse(int argc, char **argv)
{
  for (int i = 1; i < argc; ++i) {
    const std::string arg(argv[i]);
    if ("--file" == arg) {
      ++i;
      if (i < argc) {
        filename = argv[i];
      } else {
        message = "Missing required argument for --file";
        return -1;
      }
    } else if ("--frames" == arg) {
      ++i;
      if (i < argc) {
        frames = std::stoi(argv[i]);
      } else {
        message = "Missing required argument for --frames";
        return -1;
      }
    } else if ("--header" == arg) {
      header = true;
    } else if ("--help" == arg) {
      return 1;
    } else {
      message = "Unrecognized option \"" + arg + "\"";
      return -1;
    }
  }

  return 0;
}

int command_line_options::print_help(std::ostream *out, char *program_name)
{
  if (!message.empty()) {
    *out << message << newline << newline;
  }

  *out << "Usage: " << program_name << " [options...]" << newline << newline
       << "Allowed options:" << newline << "  --help         show help message"
       << newline << "  --file arg     output file" << newline
       << "  --frames N     read N frames" << newline
       << "  --header       show channel names in the first row" << newline
       << newline;

  return 1;
}
