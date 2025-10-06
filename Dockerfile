# syntax=docker/dockerfile:1

FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -G Ninja \
    && cmake --build build

FROM ubuntu:24.04 AS runtime

WORKDIR /app
COPY --from=build /app/build /app/build
COPY --from=build /app/src /app/src
COPY --from=build /app/CMakeLists.txt /app/CMakeLists.txt

CMD ["/bin/bash"]
