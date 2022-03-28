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

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git

# Checkout repo
RUN git clone https://github.com/luketokheim/shadowmocap-sdk-cpp.git

# Configure build and download dependencies (from vcpkg manifest)
RUN cd shadowmocap-sdk-cpp && \
    cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build all apps
RUN cd shadowmocap-sdk-cpp/build && \
    cmake --build . --config Release

# Run unit tests
RUN cd shadowmocap-sdk-cpp/build && \
    ctest -C Release
