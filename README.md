# Agent Monitoring & CLI (C / TUI / Linux & macOS)

High-performance, interactive CLI/TUI monitoring agent written in **C** for Linux and macOS. 
It combines system resource monitoring (enhanced `htop`-like experience) with deep disk subsystem analytics (Disk IO, IOPS, latency) and interactive manual process/task control.

## Features
- **Cross-Platform:** Native metrics collection layer for both Linux (`/proc`, `statvfs`) and macOS (`sysctl`, `mach`, `proc_info`).
- **Interactive TUI:** Rich terminal UI powered by `ncurses` / `ncursesw` with custom sparklines, graphs, and keyboard navigation.
- **Disk Analytics:** Dedicated view for IOPS, throughput (B/s), and partition space usage.
- **Process Manager:** Real-time process listing with sorting, filtering, and signal handling (`SIGTERM`, `SIGKILL`, etc.).
- **JSON Export:** `--json` CLI mode for automation and external integration with agents/LLMs.

## Project Structure
```text
.
├── Makefile             # Platform-aware build script (Linux & macOS)
├── include/             # Header files and API definitions
├── src/                 # Source code (UI, core, platform HAL)
│   ├── sys/             # Hardware Abstraction Layer (Linux & Darwin)
│   └── ui/              # TUI ncurses rendering engine
└── tests/               # Unit and integration tests
```

## Building & Running
```bash
make
./bin/agent
```

## Running Tests
```bash
make test
```

## Installation & Artifact Pipeline
To install the binary globally to `/usr/local/bin`:
```bash
sudo make install
```

To uninstall:
```bash
sudo make uninstall
```

### CI/CD Pipeline
The repository includes a GitHub Actions CI pipeline (`.github/workflows/ci.yml`) that automatically:
1. Builds the project on both **Ubuntu Linux** and **macOS**.
2. Executes all HAL and TUI test suites (`make test`).
3. Verifies CLI flag outputs (`./bin/agent --json`).
4. Validates global system installation (`make install`).