FROM ubuntu:20.04

# install dependencies and build tools
RUN apt update --fix-missing
RUN apt install -y build-essential libgmp-dev libsodium-dev git python python3-pip vim
RUN pip3 install cmake==3.18

# copy the source code into the container
COPY . /CiFEr/

# install amcl dependency
WORKDIR /CiFEr/external/amcl
RUN ./setup_amcl.sh

# install CiFEr
WORKDIR /CiFEr
RUN mkdir build_files
WORKDIR /CiFEr/build_files
RUN cmake -DCMAKE_C_COMPILER=gcc ..
RUN make
RUN make install
RUN ldconfig
WORKDIR /CiFEr/example
CMD gcc example.c -o example.out -lgmp -lcifer && ./example.out
