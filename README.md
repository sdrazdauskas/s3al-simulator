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

| Option | Description | Example |
|--------|-------------|---------|
| `--verbose` | Enable verbose logging to console | `--verbose` |
| `--memory <size>` | Set memory size (K/M/G suffix) | `--memory 2M` |
| `--help` | Show help message | `--help` |

**Example:**

```bash
# Docker - 4MB memory, verbose logging
docker run --rm -it s3al/simulator /app/s3al_sim --memory 4M --verbose
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
