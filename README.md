# Desktop Communication Suite

A single-executable Windows desktop application for chatting via LAN (Sockets) or Locally (Shared Memory).

## Features

- **Socket Mode**: Client/Server TCP chat over LAN.
- **Shared Memory Mode**: Local peer-to-peer chat on the same machine using Windows File Mapping.
- **Native Win32**: Built using pure Windows API (Unicode).
- **Single Executable**: No external DLLs or dependencies (statically linked standard libs).

## Requirements

- **OS**: Windows 10 or Windows 11.
- **Build Tools**: MSYS2 with `ucrt64` environment.

### MSYS2 Installation

1. Install [MSYS2](https://www.msys2.org/).
2. Open "MSYS2 UCRT64" terminal.
3. Update packages:
   ```bash
   pacman -Syu
   ```
4. Install GCC toolchain:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
   ```

## Build Instructions

Double-click `build.bat` or run it from the command line:

```cmd
build.bat
```

This will produce `ChatSuite.exe`.

Alternatively, run the command manually:

```cmd
g++ -std=c++17 -municode -mwindows src/*.cpp -o ChatSuite.exe -lws2_32 -lgdi32 -lcomctl32 -static-libgcc -static-libstdc++
```

## Running the Application

Double-click `ChatSuite.exe`.

### Socket Mode (LAN)

1. Launch two instances (on same or different machines).
2. **Server**: Select "Socket Mode" -> "Server", enter Port (e.g. 8888), Click Start.
3. **Client**: Select "Socket Mode" -> "Client", enter IP and Port, Click Start.

### Shared Memory Mode (Local)

1. Launch two instances on the same machine.
2. Select "Shared Memory Mode" in both.
3. **Instance 1**: Click "Person A".
4. **Instance 2**: Click "Person B".
5. Use the "Active Writer" toggle to switch who can type.
