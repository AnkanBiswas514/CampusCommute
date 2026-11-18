# CampusCompute — Intelligent Job Scheduling & Resource Management

> **Level 3 Mini-Project** — Operating Systems (UE25CS242A)  
> A command-line prototype in **C** demonstrating core OS concepts: CPU scheduling, concurrent logging, inter-process communication, Banker's safety analysis, worker-failure detection, and resource limits.

---

## 📑 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Build & Run](#build--run)
- [Demo Commands](#demo-commands)
- [Sample Output](#sample-output)
- [Submission Evidence](#submission-evidence)
- [License](#license)

---

## 🎯 Overview

A university operates a shared computing lab where students and researchers run CPU-intensive programs. During peak hours, jobs compete for CPU time and shared resources. **CampusCompute** is a C prototype that implements and demonstrates:

| Requirement | OS Concept | Implementation |
|-------------|-----------|----------------|
| FR1 | Job management & lifecycle | `job_t` struct with ID, arrival, burst, priority, status |
| FR2 | CPU scheduling | FCFS, Round Robin, non-preemptive Priority |
| FR3 | Concurrent logger | POSIX threads, bounded queue, mutex + condition variables |
| FR4 | Inter-process communication | POSIX shared memory + named semaphore |
| FR5 | Resource-safety analysis | Banker's safety algorithm |
| FR6 | Worker-failure detection | `fork()` + `waitpid()` |
| FR7 | Resource limits | `setrlimit(RLIMIT_CPU)` |
| FR8 | CLI & error handling | Documented menu, usage messages, independently testable |

All output is **computed live by working algorithms** — no hard-coded or mocked results.

---

## 📁 Project Structure

```
CampusCompute/
├── campuscompute.h      # Shared data structures & prototypes
├── main.c               # CLI menu & command dispatch (FR8)
├── scheduler.c          # FCFS / Round Robin / Priority (FR1/FR2)
├── logger.c             # pthreads producer-consumer logger (FR3)
├── ipc.c                # Shared memory + named semaphore IPC (FR4)
├── banker.c             # Banker's safety algorithm (FR5)
├── worker.c             # fork/waitpid failure detection (FR6)
├── limits.c             # setrlimit resource-limit demo (FR7)
├── Makefile             # Build: make / make clean / make run
├── commands.txt         # All demo commands with labels
├── CampusCompute_Project.pptx  # Project presentation
└── README.md            # This file
```

---

## 🛠 Prerequisites

| Tool | Purpose |
|------|---------|
| `gcc` (or `clang`) | C compiler (C11) |
| `make` | Build automation |
| `pthread` | POSIX threads (standard on Linux/macOS/WSL) |
| POSIX shm / semaphores | `shm_open`, `mmap`, `sem_open` (standard on Linux/macOS/WSL) |

Verify tools are available:
```bash
gcc --version
make --version
```

---

## ▶ Build & Run

### Build from source
```bash
make
```
Produces the executable `./campuscompute`.

### Clean
```bash
make clean
```

### Run the full integrated demo
```bash
./campuscompute all
```

---

## 📋 Demo Commands

See `commands.txt` for the complete, labelled command list.

| Command | Feature | Description |
|---------|---------|-------------|
| `./campuscompute help` | CLI | Show usage & available commands |
| `./campuscompute fcfs` | FR2 | First-Come First-Served scheduling |
| `./campuscompute rr [q]` | FR2 | Round Robin (default quantum = 2) |
| `./campuscompute priority` | FR2 | Non-preemptive Priority scheduling |
| `./campuscompute schedule` | FR2 | Run all three schedulers + comparison |
| `./campuscompute logger [log] [producers] [msgs]` | FR3 | Concurrent producer-consumer logger |
| `./campuscompute ipc [log]` | FR4 | Shared memory + named semaphore IPC |
| `./campuscompute banker` | FR5 | Banker's safe & unsafe state tests |
| `./campuscompute worker [0\|1]` | FR6 | Worker normal (0) vs simulated failure (1) |
| `./campuscompute limits` | FR7 | `setrlimit(RLIMIT_CPU)` demo → SIGXCPU |
| `./campuscompute all` | ALL | Integrated execution of every feature |

---

## 🖥 Sample Output

**FCFS Scheduling:**
```
=== FCFS ===
Execution order: 1 -> 2 -> 3 -> 4 -> 5
JobID   Arrival   Burst   Priority  Start   Completion  Turnaround
1       0         7       3         0       7           7
2       2         4       1         7       11          9
...
Average Turnaround Time: 10.00
Average Waiting Time:    6.00
```

**Banker's Algorithm (safe state):**
```
Safety result: SAFE
Safe sequence: P1 -> P3 -> P4 -> P0 -> P2
```

**Banker's Algorithm (unsafe state):**
```
Safety result: UNSAFE
No safe sequence exists -> resource state is UNSAFE.
```

**Worker Failure Detection:**
```
[Parent] child exited normally with code 0
[Parent] normal completion confirmed.
[Parent] detected SIMULATED WORKER FAILURE (non-zero exit 42).
```

**Resource Limits:**
```
[Observed effect] Child terminated by signal 24 (SIGXCPU=24)
```

---

## 📸 Submission Evidence (S01–S24)

Per the project checklist, capture numbered screenshots from **your own terminal** with your full SRN in the prompt:

```bash
export SRN='YOUR_FULL_SRN'
export PS1='[$SRN] \w\$ '
```

Required screenshots: S01 (environment) → S24 (submission ZIP). Combine them with one-line explanations into a single PDF named `YOUR_SRN.pdf`.

---

## 📄 License

Academic project — Operating Systems coursework.

---

**Built with C on Linux / Ubuntu / WSL. Compatible with macOS (clang + pthread).**