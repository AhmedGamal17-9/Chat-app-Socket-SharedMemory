# Chat-app-Socket-SharedMemory

A single-executable Windows desktop application for chatting via LAN (Sockets) or Locally (Shared Memory).

## Features

- **Socket Mode**: Client/Server TCP chat over LAN.
- **Shared Memory Mode**: Local peer-to-peer chat on the same machine using Windows File Mapping.
- **Native Win32**: Built using pure Windows API (Unicode) for maximum performance and zero dependencies.
- **Single Executable**: No external DLLs or dependencies (statically linked standard libs).


---

## 📝 Development Setup

### 1. Clone the Repository

Clone the project to your local machine:

```bash
git clone <repository-url>
cd Chat-app-Socket-SharedMemory
```

### 2. Antivirus Configuration

> [!NOTE] > **Recommendation**: After cloning and before building/running, it is recommended to create an exception for the project folder in Windows Defender or your antivirus software. This prevents false positives common with custom socket/shared-memory applications.

### 3. Prerequisites

- **OS**: Windows 10 or Windows 11.
- **Build Environment**: [MSYS2](https://www.msys2.org/) with `ucrt64`.

**Setting up MSYS2:**

1. Install MSYS2.
2. Open the "MSYS2 UCRT64" terminal.
3. Update system: `pacman -Syu` then `pacman -Su`.
4. Install toolchain:

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
   ```

5. Verify installation:
   ```bash
   gcc --version
   g++ --version
   gdb --version
   ```

## 🛠️ Tech Stack

| Layer               | Tech                   |
| :------------------ | :--------------------- |
| **Language**        | C++17                  |
| **GUI**             | Native Win32 API (GDI) |
| **Version Control** | Git + GitHub           |

---

## 📂 Project Structure

```text
Chat-app-Socket-SharedMemory/
├── src/
│   ├── Gui.cpp / .h          # User Interface logic (Win32)
│   ├── Network.cpp / .h      # Winsock2 TCP implementation
│   ├── SharedMemory.cpp / .h # Windows Named Shared Memory & Semaphores
│   ├── Logger.h              # Simple singleton logger
│   └── main.cpp              # Entry point
├── ChatSuite.exe             # Compiled executable
├── build.bat                 # One-click build script
└── README.md                 # Project documentation
```

---

## 🤝 Collaboration Workflow

### Branch Naming

- **New Features**: `feature/feature-name`
- **Bug Fixes**: `fix/bug-name`
- **Hotfixes**: `hotfix/issue-name`

### Git Flow

- **`main`**: Always stable, production-ready code.
- **`dev`**: Staging branch for integration testing.
- **`feature/*`**: Individual feature branches created from `dev`.

### Commit Style

Please follow [Conventional Commits](https://www.conventionalcommits.org/):

- `feat: add new chat color`
- `fix: resolve socket connection timeout`
- `docs: update readme`
- `refactor: optimize gui rendering`

### Code Review

- All changes must be submitted via Pull Request (PR) to the `dev` branch.
- At least one code review approval is required before merging.

---

## 🏗️ Build Instructions

**Option 1: One-Click Build**
Double-click `build.bat` in the project root.

**Option 2: Manual Build**
Run the following command in your terminal:

```bash
g++ -std=c++17 -municode -mwindows src/*.cpp -o ChatSuite.exe -lws2_32 -lgdi32 -lcomctl32 -static-libgcc -static-libstdc++
```

---

## 🚀 Running the Application

Double-click `ChatSuite.exe` to start.

### Socket Mode (LAN)

1. Launch two instances (can be on different machines).
2. **Server**: Select [Socket Mode] -> [Server], enter Port (e.g., 8888), click **Start**.
3. **Client**: Select [Socket Mode] -> [Client], enter Server IP and Port, click **Start**.

### Shared Memory Mode (Local)

1. Launch two instances on the _same_ machine.
2. Select [Shared Memory Mode] in both.
3. **Instance 1**: Click **Person A**.
4. **Instance 2**: Click **Person B**.
5. Use the **"Active Writer"** toggle to switch who has permission to type.
