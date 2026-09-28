FROM ubuntu:24.04 AS build

RUN apt-get update && \
    apt-get install -y --no-install-recommends g++ cmake && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt ./
COPY backend ./backend

RUN cmake -S . -B build && cmake --build build

FROM ubuntu:24.04

WORKDIR /app

COPY --from=build /app/build/server ./server
COPY backend/data ./backend/data
COPY frontend ./frontend

ENV PORT=8080
EXPOSE 8080

CMD ["sh", "-c", "./server \"${PORT:-8080}\""]