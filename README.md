# s3al

## Native build

```powershell
# First time or after changing CMakeLists.txt
cmake -S . -B build

cmake --build build
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

# Run with persistent logs (volume mount)
docker run --rm -it -v ./logs:/app/logs s3al/simulator
```
