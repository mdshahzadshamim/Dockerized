FROM ubuntu:24.04

RUN apt update && apt install -y \
    cmake \
    g++ \
    git \
    wget \
    make \
    libpq-dev \
    postgresql-client \
    pkg-config \
    build-essential

WORKDIR /tmp
RUN git clone https://github.com/jtv/libpqxx.git
WORKDIR /tmp/libpqxx
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20
RUN cmake --build build --target install

WORKDIR /app

COPY . .

RUN cmake -S . -B build
RUN cmake --build build

CMD ["./build/Tasks"]
