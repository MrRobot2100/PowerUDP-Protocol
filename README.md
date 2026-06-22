# PowerUDP - Reliable UDP Protocol 📡🔒

## Overview

This project is a full implementation of **PowerUDP**, a custom application-layer reliability protocol built on top of raw UDP sockets, developed in **C** for the *Computer Networks* course. It enhances standard UDP with retransmission guarantees, ordered delivery, and dynamic runtime reconfiguration — all without TCP's overhead.

The system is composed of a reusable protocol library (`powerudp.c/h`), an interactive CLI client, and a fork-based TCP server that coordinates configuration updates across all active hosts via **IP Multicast**.

## ⚙️ Key Features

### Reliable Transmission (PowerUDP Core)

- **Stop-and-Wait Model:** The protocol only sends the next message after the previous one has been acknowledged (ACK), ensuring ordered, lossless delivery over unreliable UDP.
- **Retransmission with Exponential Backoff:** On timeout, messages are retransmitted up to a configurable maximum. Wait time between attempts follows `Tn = Tmin × 2ⁿ`, doubling on each failure to reduce network congestion.
- **Sequence Numbers:** Every outgoing message is tagged with an incrementing sequence number. The receiver rejects duplicates and out-of-order packets, replying with a NAK to signal the anomaly.
- **Custom Encapsulation:** A `PowerUDPHeader` (type, seq\_num, data\_len) is prepended to every payload before it is placed in the UDP datagram, cleanly separating protocol state from application data.

### Server & Multicast Configuration

- **TCP Registration Server:** A fork()-based server accepts client connections, validates a pre-shared key (PSK) read from `psk.txt`, and registers authenticated hosts.
- **Dynamic Reconfiguration via Multicast:** When a client sends a new `ConfigMessage` via TCP, the server immediately broadcasts it to all active clients over the multicast group `239.0.0.1`, using `IP_MULTICAST_TTL` to traverse routers (PIM sparse-dense-mode on Cisco IOS).
- **Background Multicast Listener:** Each client runs a dedicated `pthread` that continuously listens for incoming `ConfigMessage` multicast packets and updates the active protocol configuration in real time.

### Diagnostics & Testing

- **Packet Loss Injection:** `inject_packet_loss(probability)` simulates network drops at the sender (0–100%), enabling repeatable retransmission tests without physical network degradation.
- **Per-Message Statistics:** After every send, `get_last_message_stats()` reports the number of retransmissions used and the total delivery time in milliseconds.
- **Interactive CLI:** The client exposes `send`, `recv`, `config`, `loss`, and `stats` commands for live protocol exercising and demonstration.

## 🛠️ Tech Stack

- **Language:** C (C99)
- **OS:** Linux
- **Tools:** GCC, Make, GDB, Cisco IOS (Multicast routing)
- **Concepts:** UDP/TCP Sockets, IP Multicast (PIM), POSIX Threads, `fork()`, Stop-and-Wait, Exponential Backoff, Sequence Numbers, PSK Authentication.

## 🚀 How to Run

### 1. Compile

```bash
make
```

### 2. Start the Server

```bash
./servidor
```

### 3. Start a Client

```bash
./cliente <server_ip> <psk>
# Example:
./cliente 10.5.2.1 my_secret_key
```

### 4. Available Client Commands

```
send <dest_ip> <message>              # Send a PowerUDP message
recv                                  # Block and wait for a message
config <retx> <backoff> <seq> <timeout_ms> <max_retries>  # Request config change
loss <0-100>                          # Inject packet loss probability (%)
stats                                 # Show stats for the last sent message
sair                                  # Disconnect and exit
```

### 5. Example: Simulate Retransmissions

```bash
# On the sender side — force 50% packet loss to trigger retransmissions
> loss 50
> send 10.5.2.2 hello
[cliente] Entregue: retransmissoes=2  tempo=1500ms
```
