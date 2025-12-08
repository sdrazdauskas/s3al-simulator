# syntax=docker/dockerfile:1

FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        libncurses-dev \
        lua5.4 \
        liblua5.4-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -G Ninja \
    && cmake --build build

FROM ubuntu:24.04 AS runtime

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libncurses6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /app/build/s3al_sim /app/s3al_sim

CMD ["/app/s3al_sim"]
