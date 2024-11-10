# Design-Implementation-of-an-API-Simulator
# API Simulator: Fork/Exec System Calls

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Memory Partitions](#memory-partitions)
- [Data Structures](#data-structures)
  - [Memory Partitions Table](#1-memory-partitions-table)
  - [Process Control Block (PCB) Table](#2-process-control-block-pcb-table)
  - [External Files List](#3-external-files-list)
- [System Calls](#system-calls)
  - [Fork](#1-fork)
  - [Exec](#2-execfile_name)
- [Logging and Output](#logging-and-output)
- [Assumptions](#assumptions)
- [Test Scenarios](#test-scenarios)
  - [Mandatory Tests](#mandatory-tests)
  - [Custom Tests](#custom-tests)
- [Input Files](#input-files)
- [Usage](#usage)
- [Setup Instructions](#setup-instructions)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

This project is an API simulator that emulates basic operating system behaviors, focusing on the `fork` and `exec` system calls. It manages memory partitions, process control blocks (PCBs), and external files to simulate process creation and execution in a controlled environment.

## Features

- **Fixed Memory Partitions**: Simulates 100 MB of user space divided into six fixed-size partitions for process allocation.
- **Process Control Block (PCB)**: Tracks essential information for each process, such as PID, CPU/I/O details, remaining CPU time, and memory partition.
- **External File Loading**: Loads program details from an external file list, mimicking persistent storage.
- **Simulated System Calls**: Implements `fork` and `exec` system calls with behavior similar to real operating systems.
- **Logging**: Records all activities related to `fork` and `exec`, including execution times.
- **Scheduler**: Placeholder scheduler function that gets called after system calls.

## Memory Partitions

The simulator divides 100 MB of user space into six fixed partitions:

1. **Partition 1**: 40 MB
2. **Partition 2**: 25 MB
3. **Partition 3**: 15 MB
4. **Partition 4**: 10 MB
5. **Partition 5**: 8 MB
6. **Partition 6**: 2 MB (Reserved for `init` process)

Each partition is tracked in a data structure to manage allocation status and the process occupying it.

## Data Structures

### 1. Memory Partitions Table

- **Fields**:
  - **Partition Number**: Unsigned int
  - **Size**: Unsigned int
  - **Status**: String (`free`, `init`, or program name)
- **Purpose**: Manages the allocation status of each memory partition.

### 2. Process Control Block (PCB) Table

- **Fields**:
  - **PID**: Process ID
  - **CPU/I/O Information**
  - **Remaining CPU Time**
  - **Partition Number**: Memory partition allocated to the process
  - **Additional Fields**: As required by the simulator
- **Purpose**: Keeps track of processes and their execution details.

### 3. External Files List

- **Fields**:
  - **Program Name**: String (max 20 characters)
  - **Size**: Unsigned int (Memory size when loaded)
- **Purpose**: Simulates persistent storage containing programs available for execution.

## System Calls

### 1. `fork()`

- **Simulation Steps**:
  - Triggers a system call interrupt (`SYSCALL`).
  - ISR (Interrupt Service Routine) copies the PCB of the parent to create a child process.
  - Calls `scheduler()` function (currently outputs "scheduler called").
  - Returns from ISR.

### 2. `exec(file_name)`

- **Simulation Steps**:
  - Triggers a system call interrupt (`SYSCALL`).
  - ISR searches for the program in the external files list to get its size.
  - Finds an appropriate free memory partition using the best-fit policy.
  - Updates the partition status to occupied and records the process using it.
  - Updates the PCB with new process information.
  - Calls `scheduler()` function.
  - Returns from ISR.

## Logging and Output

- **Activity Logging**: All `fork` and `exec` activities are logged with each step assigned a random execution time between 1 and 10 milliseconds.
- **System Status File**: After each system call, the simulator writes to `system_status.txt`:
  - Current simulated time
  - Contents of the PCB table

## Assumptions

- `exec` is invoked only by child processes, not by the parent.
- Child processes have higher priority than parent processes; they must complete execution before the parent resumes.
- `fork` system call is at vector 2, and `exec` is at vector 3.

## Test Scenario

#### **Test 1**

- **Initial State**: Only `init` process is in the system.
- **`init` Process Code**:
FORK, 17
EXEC program1, 16
- **Contents of `program1`**:
FORK, 15
EXEC program2, 33
- **Contents of `program2`**:
CPU, 53
SYSCALL 5, 128
END_IO 11, 115



## Input Files

1. **`external_files.txt`**: Contains the list of external programs and their sizes.
2. **`trace.txt`**: Initial execution trace for the simulator.
3. **`vector_table.txt`**: Defines the system call vectors.
4. **Program Traces**: Individual program files corresponding to the `exec` calls, accessible by the simulator.

## Usage

1. **Initialize the Simulator**:
 - Load `external_files.txt` to populate the external files list.
 - Initialize the PCB table with the `init` process occupying Partition 6.

2. **Run Test Scenarios**:
 - Use the provided test scenario or your custom test.
 - The simulator will execute the instructions, handle system calls, and manage memory and processes accordingly.

3. **Review Logs and Outputs**:
 - Check `system_status.txt` for system status after each system call.
 - Review activity logs for detailed execution steps and timings.

## Setup Instructions

1. **Clone the Repository**:
 ```bash
 git clone https://github.com/yourusername/api-simulator.git
2. Navigate to the project directory
3. Prepare input files
4. **Compile the simulator**:
gcc -o simulator simulator.c
5. Run the simulator
./simulator
