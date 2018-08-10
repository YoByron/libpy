# DOCKER-VERSION 1.3.0
FROM ubuntu:16.04
MAINTAINER Quantopian Inc.

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y software-properties-common wget
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-6.0 main" >> \
    /etc/apt/sources.list.d/llvm.list && apt-get update
RUN add-apt-repository ppa:deadsnakes/ppa
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update && apt-get install -y \
    doxygen \
    g++-7 \
    gcc-7 \
    clang-tools-6.0 \
    gfortran \
    git \
    libatlas-base-dev \
    libblas-dev \
    libffi-dev \
    libssl-dev \
    make \
    openssh-client \
    python3.6-dev \
    python3.6-venv \
    tzdata \
    util-linux \
    valgrind

RUN mkdir -p /virtualenvs
RUN mkdir -p /src
RUN python3.6 -m venv /virtualenvs/libpy
RUN mkdir -p ~/.ssh && ssh-keyscan github.com >> ~/.ssh/known_hosts

ENV VENV_HOME=/virtualenvs
ENV VENV_ACTIVATE=$VENV_HOME/libpy/bin/activate

WORKDIR /src/
COPY . ./libpy
COPY ./etc/Makefile.jenkins ./libpy/Makefile.local

# clean up by resetting DEBIAN_FRONTEND to its default value:
ENV DEBIAN_FRONTEND newt