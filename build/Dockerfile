#
# docker build -t shadowmocap-builder -f Dockerfile .
#
FROM ubuntu

# Install build dependencies from package repo
RUN apt-get update && apt-get install -y \
  build-essential \
  g++-10 \
  git \
  python3

# Checkout repo and submodules
RUN git clone --recurse-submodules \
  https://github.com/motion-workshop/shadowmocap-sdk-cpp.git

# Build all applications
RUN cd shadowmocap-sdk-cpp/build && CXX=g++-10 make all

# Run unit tests
# RUN cd shadowmocap-sdk-cpp/build && CXX=g++-10 make test
