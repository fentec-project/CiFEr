FROM ubuntu:20.04

# install dependencies and build tools
RUN apt update && apt install --no-install-recommends -qq -y \
    build-essential \
    libgmp-dev \
    libsodium-dev \
    libprotobuf-c-dev \
    git \
    python \
    python3-pip \
    vim \
    pkg-config && \
    rm -rf /var/lib/apt/lists/* && \
    pip3 install cmake==3.18

# copy the source code into the container
COPY . /CiFEr/

# install amcl dependency
WORKDIR /CiFEr/external/amcl
RUN ./setup_amcl.sh

# install CiFEr
WORKDIR /CiFEr/build
RUN cmake -DCMAKE_C_COMPILER=gcc .. && \
    make && \
    make install && \
    ldconfig

WORKDIR /CiFEr/example
CMD gcc example.c -o example.out -lgmp -lcifer && ./example.out
