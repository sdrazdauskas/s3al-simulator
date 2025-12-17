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
| `--log-level <level>` | `-l` | Set minimum log level: `debug`, `info`, `warning`, `error` | debug | `--log-level info` |
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

# Native - RoundRobin scheduler with quantum 3, info-level logging
./build/s3al_sim --scheduler rr --quantum 3 --log-level info

# Docker - Priority scheduler with fast CPU (2 cycles/tick, 50ms ticks)
docker run --rm -it s3al/simulator /app/s3al_sim -s priority -c 2 -t 50 -v

# Native - Error-level logging only (minimal output)
./build/s3al_sim -l error -v
```
## Shell

You can access all the available shell commands by typing inside CLI:
```bash
help
```
You can check the usage of all commands by typing out the command without arguments:
```
cat
Usage: cat <fileName> [fileName...]

cat hello.txt

=== contents of hello.txt ===
Hello world!
=============================
```
We support the usage of redirection operators and command chaining:

### Redirection operators:
```bash
# > (Output redirection) Sends a command's output to a file, creating or overwriting the file.
echo hello world > hello.txt
# >> (Append Redirection): Adds a command's output to the end of a file, preserving existing content.
echo hello >> hello.txt
# < (Input Redirection): Reads input for a command from a file instead of the keyboard.
echo < hello.txt
```
### Command chaining:
```bash
# | (Pipe): Connects the output of one command to the input of another.
echo 5555 | add 5
# && (AND operator): Allows you to execute multiple commands in sequence
echo hello && echo world
```
## Scripting
You can use any file that you've created as a script file, as long as it has valid Lua syntax inside:
```bash
touch filename # I create an empty file
texed filename # I edit the file and add some script lines
./filename # I run the file as a Lua script
```
All of our scripting is done through embedded Lua, to run a script file type inside CLI:
```bash
# Every file will be counted as a script as long as it starts with ./
./filename
```
You can use our shell commands inside the Lua script by typing this:
```bash
# You can see the usage in the examples below
sh("command arg")
```
Examples of script files:
```bash
# A simple loop that adds up the integers
local sum = 0
for i = 1, 5 do
sum = sum + i
end
print("sum", sum)
```
```bash
# Goes through loop and creates files using our shell command
# Result: file1,file2,file3,file4,file5
for i = 1, 5 do
sh("touch file" .. i)
end
```
```bash
# We create a file named number2025 and redirect echo input to the file
local n = 2025
sh("touch number" .. n)
sh("echo " .. n .. " > number" .. n)
```
```bash
# You can use user input that is entered through our OS CLI
local secretNumber = 10
print("Guess the secret number")
local guess = tonumber(io.read())
if guess == secretNumber then
print("Correct")
else
print("Wrong. The number was " .. secretNumber)
end
```
## Text editor
We are using a vim-like text editor that can be accessed with:
```bash
texed filename
```
### Available modes:
| Key | Mode   | Action                           |
|-----|--------|----------------------------------|
| i   | Insert | Pressing i enables insert mode   |
| Esc | Normal | Pressing Esc enables normal mode |
| :   | Command| Pressing : enters command-line mode |
### Saving / exiting:

| Command | Required Mode | Action              |
|---------|---------------|---------------------|
| :w      | Normal        | Save file           |
| :q      | Normal        | Quit editor         |
| :wq     | Normal        | Save and quit       |
| :q!     | Normal        | Quit without saving |
| :help   | Normal        | See help message    |
### Navigation inside Normal mode

| Key | Required mode | Action           |
|-----|---------------|------------------|
| ←                  | Insert        | Move left        |
| ↓                  | Insert        | Move down        |
| ↑                  | Insert        | Move up          |
| →                  | Insert        | Move right       |
| Del                | Insert        | Delete character |
| h                  | Normal        | Move left        |
| j                  | Normal        | Move down        |
| k                  | Normal        | Move up          |
| l                  | Normal        | Move right       |
| x                  | Normal        | Delete character |





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
docker build --target build -t s3al-build . ; docker run --rm s3al-build bash -c "cd /app/build && ctest --output-on-failure"
```
