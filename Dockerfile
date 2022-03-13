#
# docker build -t shadowmocap-builder -f Dockerfile .
#
FROM ubuntu:devel

# Install build dependencies from package repo
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    curl \
    git \
    pkg-config \
    python3 \
    zip

# Install vcpkg and use it to download dependencies
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
    ./vcpkg/vcpkg install benchmark boost-asio boost-test

# Checkout repo
RUN git clone -b develop \
    https://github.com/luketokheim/shadowmocap-sdk-cpp.git

# Configure build
RUN cmake -B build -S shadowmocap-sdk-cpp \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build all applications
RUN cmake --build build

# Run unit tests
# RUN cd shadowmocap-sdk-cpp/build && CXX=g++-10 make test
