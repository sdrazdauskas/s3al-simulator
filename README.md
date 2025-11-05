# s3al

## Native build

```powershell
# First time or after changing CMakeLists.txt
cmake -S . -B build

cmake --build build

# Run the simulator
.\build\Debug\s3al_sim.exe

# or with logging to cout
.\build\Debug\s3al_sim.exe --verbose
```

## Docker image

Build the containerized toolchain and project:

```powershell
# Build and automatically clean up dangling images
docker build -t s3al/simulator:latest . ; docker image prune -f
```

Or build and clean separately:

```powershell
docker build -t s3al/simulator:latest .
docker image prune -f
```

Launch an interactive shell in the image:

```powershell
# Run without saving logs
docker run --rm -it s3al/simulator

# Run with verbose logging (logs to console and file)
docker run --rm -it s3al/simulator /app/s3al_sim --verbose

# Run with persistent logs (volume mount)
docker run --rm -it -v ./logs:/app/logs s3al/simulator

# Run with persistent data (saves/loads storage state)
docker run --rm -it -v ./data:/app/data s3al/simulator

# Run with both persistent logs and data
docker run --rm -it -v ./logs:/app/logs -v ./data:/app/data s3al/simulator

# Run with verbose logging and persistent data
docker run --rm -it -v ./logs:/app/logs -v ./data:/app/data s3al/simulator /app/s3al_sim --verbose
```
