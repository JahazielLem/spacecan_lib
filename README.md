# SpaceCAN Library

[![Build Status](https://github.com/JahazielLem/spacecan_lib/actions/workflows/build.yml/badge.svg)](https://github.com/JahazielLem/spacecan_lib/actions)
[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL--3.0--or--later-blue.svg)](https://opensource.org/licenses/GPL-3.0)

A high-performance, lightweight C implementation of the **SpaceCAN** protocol. Designed for modular spacecraft communication and research, this library provides a simulated environment to test satellite bus architectures, telemetry flows, and cybersecurity vulnerabilities (ICS-style) without requiring physical hardware.

## Core Features

* **Standard Compliance**: Implementation of SpaceCAN ID management (Request/Reply frames).
* **Packet Fragmentation**: Native support for multi-frame transfer and reassembly for payloads exceeding the 8-byte CAN limit.
* **Network Management (NMT)**: Built-in Heartbeat and Synchronization (SYNC) mechanisms for node health monitoring.
* **Virtual Bus Architecture**: A robust server-client model (`busd`) using Unix sockets to emulate a physical CAN bus.
* **Research Oriented**: Designed for "Space-ICSim" scenarios, allowing for fault injection and protocol analysis.

## Toolset

The library includes a suite of CLI and TUI tools for network interaction:

| Tool | Description |
| :--- | :--- |
| **`controller`** | The central bus daemon (busd) acting as the communication hub. |
| **`scsniffer`** | Diagnostic tool with filtering. Supports PCAP (Wireshark) and custom Replay formats. |
| **`scviewer`** | Real-time Ncurses TUI for monitoring packet frequency, timing, and data. |
| **`scmonitor`** | Dedicated TUI for real-time telemetry visualization from simulated sensors. |
| **`scinjection`** | Command-line utility for manual frame crafting and bus injection. |
| **`screplay`** | Automated traffic replay engine for testing state-machine persistence. |

## Getting Started

### Prerequisites
* GCC / Clang
* Ncurses Development Libraries
* Make

### Installation
```bash
git clone https://github.com/JahazielLem/spacecan_lib.git
cd spacecan_lib
make
```

### Usage Workflow

1. **Initialize the Bus**:
    
    Start the central controller to allow nodes to connect.
   ```shell
   ./controller
   ```
2. **Network Monitoring**:

    Use the sniffer to capture traffic or the viewer for a live overview.
    ```shell
    ./scsniffer -o capture.pcap  # Capture to Wireshark format
    ./scviewer                   # Launch real-time TUI
    ```
3. **Sensor Visualization**:

    Monitor simulated node data (telemetry).
    ```shell
    ./scmonitor
    ```
4. **Traffic Injection**:

    Inject a manual frame (Example: CANID 0x284, 2 bytes of data).
    ```shell
    ./scinjection 0x284 01 12
    ```

## Node Emulation
The library provides pre-defined worker patterns for Sensors and Motors. These templates include state management and telemetry loops, allowing researchers to quickly instantiate operational nodes within the simulation.

## License
This project is licensed under the GPL-3.0-or-later license.
