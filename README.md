# s3al

## Quick Start

### Native Build

```bash
# Install the dependencies
./install.sh
```
Then

```bash
# First time or after changing CMakeLists.txt
cmake -S . -B build

# Build
cmake --build build

# Run (Windows)
.\build\Debug\s3al_sim.exe [OPTIONS]

# Run (Linux)
./build/s3al_sim [OPTIONS]
```

### Docker

```bash
# Build and automatically clean up dangling images
docker build -t s3al/simulator:latest . ; docker image prune -f

# Run
docker run --rm -it s3al/simulator /app/s3al_sim [OPTIONS]
```

## Available Options

Both native and Docker builds support the same command-line options:

### General Options

| Option | Short | Description | Default | Example |
|--------|-------|-------------|---------|---------|
| `--verbose` | `-v` | Enable verbose logging to console | Off | `--verbose` |
| `--memory <size>` | `-m` | Set memory size (K/KB, M/MB, G/GB suffix) | 1M (1048576 bytes) | `--memory 2M` |
| `--help` | `-h` | Show help message | - | `--help` |

### Scheduler Options

| Option | Short | Description | Default | Example |
|--------|-------|-------------|---------|---------|
| `--scheduler <algo>` | `-s` | Scheduling algorithm: `fcfs`, `rr` (roundrobin), `priority` | fcfs | `--scheduler rr` |
| `--quantum <n>` | `-q` | Time quantum for RoundRobin (in cycles) | 5 | `--quantum 3` |
| `--cycles <n>` | `-c` | CPU cycles per scheduler tick | 1 | `--cycles 2` |
| `--tick-ms <n>` | `-t` | Milliseconds between scheduler ticks | 100 | `--tick-ms 50` |

**Examples:**

```bash
# Docker - 4MB memory, verbose logging
docker run --rm -it s3al/simulator /app/s3al_sim --memory 4M --verbose

# Native - RoundRobin scheduler with quantum 3
./build/s3al_sim --scheduler rr --quantum 3

# Docker - Priority scheduler with fast CPU (2 cycles/tick, 50ms ticks)
docker run --rm -it s3al/simulator /app/s3al_sim -s priority -c 2 -t 50 -v
```

## Docker Persistent Storage

Mount volumes to persist logs and data across runs:

```bash
# Persistent logs only
docker run --rm -it -v ./logs:/app/logs s3al/simulator

# Persistent data only  
docker run --rm -it -v ./data:/app/data s3al/simulator

# Both logs and data
docker run --rm -it -v ./logs:/app/logs -v ./data:/app/data s3al/simulator /app/s3al_sim --verbose
```

## Tests
As project requires libraries that might not exist on your system, you can run the tests using docker:
```bash
docker build --target build -t s3al-build . ; podman run --rm s3al-build bash -c "cd /app/build && ctest --output-on-failure"
```
