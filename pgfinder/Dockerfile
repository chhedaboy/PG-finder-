FROM ubuntu:24.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

# Build fresh inside Render
RUN cmake -S . -B build && \
    cmake --build build

CMD ["sh", "-c", "./build/server ${PORT:-10000}"]