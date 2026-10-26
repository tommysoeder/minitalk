# Minitalk

**Inter-process communication in C using Unix signals.**

Minitalk sends a command-line message from a client process to a server process. The client encodes each byte as eight signals, and the server reconstructs the bytes and writes them to standard output. This project explores process identifiers, asynchronous signal handlers, and bitwise operations through a small communication protocol.

**Core technologies:** C · Unix/Linux · `SIGUSR1` / `SIGUSR2` · `sigaction` · Make

## How it works

### Eight-bit encoding

The client reads each message byte as an `unsigned char` and sends its bits from position `7` down to `0`, most significant bit first:

| Bit value | Signal |
| --- | --- |
| `0` | `SIGUSR1` |
| `1` | `SIGUSR2` |

For example, the ASCII character `A` has byte value `65` (`01000001`):

```text
Bits:     0       1       0       0       0       0       0       1
Signals:  USR1    USR2    USR1    USR1    USR1    USR1    USR1    USR2
```

`USR1` and `USR2` above abbreviate `SIGUSR1` and `SIGUSR2`. [`send_char()`](minitalk/client.c) extracts each bit with `(c >> i) & 1`, sends the corresponding signal with `kill()`, and requests a 300 microsecond pause using `usleep(300)` after each signal.

The client also sends the terminating `\0` byte as eight zero bits. The server uses that byte as the message terminator and adds a newline.

### Client

[`client.c`](minitalk/client.c) expects exactly two arguments: the server PID and one message string. It converts the PID with the local `ft_atoi()`, rejects converted values less than or equal to zero, and sends every message byte followed by the terminator.

Each `kill()` call is checked. On failure, the client writes an error to standard error and exits with status `1`. If all sends complete, it exits with status `0` without waiting for a response from the server.

### Server

[`server.c`](minitalk/server.c) prints its PID using `getpid()`, installs the same handler for `SIGUSR1` and `SIGUSR2` through `sigaction()`, and waits for signals in a loop using `pause()`.

The handler maintains a static bit counter and byte accumulator. For `SIGUSR2`, it sets the current bit with `c |= (1 << (7 - bit))`; `SIGUSR1` leaves that bit at zero. After eight signals, it writes the reconstructed byte and resets both values for the next byte.

The server writes **every reconstructed byte, including the terminating NUL**, and then writes a newline after NUL. NUL is normally invisible in a terminal, but is present if stdout is captured to a file. The server stays running after each message.

## Build

Requirements:

- A Linux or Unix environment providing the signal APIs used by the program.
- A C compiler available as `cc`, Make, and Git for cloning.

```sh
git clone https://github.com/tommysoeder/minitalk.git
cd minitalk/minitalk
make re
```

The repository includes prebuilt executables and object files. Use `make re` for the initial build to rebuild from source on your machine. The [Makefile](minitalk/Makefile) uses `cc` with `-Wall -Wextra -Werror` and produces two executables, `server` and `client`, in the inner `minitalk/` directory.

Run these commands from that directory:

| Command | Effect |
| --- | --- |
| `make` | Build `server` and `client` when their dependencies require it |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and both executables |
| `make re` | Clean and rebuild both executables |

## Run in two terminals

Open the cloned repository's inner `minitalk/` directory in both terminals. Run both programs on the same machine, with permission to send signals to the server process.

### Terminal 1: start the server

```sh
./server
```

It prints `Server PID: <number>`. Keep this terminal open and copy the actual PID for the client.

### Terminal 2: send a message

Capture the PID in a shell variable:

```sh
read -r SERVER_PID
```

Paste the numeric PID printed by the server and press Enter. Then send a quoted message:

```sh
./client "$SERVER_PID" "Hello from Minitalk"
```

If the bytes arrive intact, Terminal 1 displays the message followed by a newline. The client does not print a delivery confirmation. Wait for the message to appear before sending another one.

Additional examples, sent one at a time:

```sh
./client "$SERVER_PID" "A message with spaces"
./client "$SERVER_PID" ""
```

An empty string sends only the NUL terminator, producing a visible blank line on the server. Stop the server with `Ctrl+C` in Terminal 1 when finished. Always use the PID of the server instance you started.

## Technical concepts explored

| Concept | Application in this project |
| --- | --- |
| Process identifiers (PIDs) | The server prints its PID; the client uses it to address that process |
| `kill()` | Sends a selected signal to the server PID |
| `sigaction()` | Registers the receive handler for the two signal types |
| Asynchronous signals | The handler processes incoming bits while the server waits with `pause()` |
| Bitwise operations | Right shift and masking extract bits; left shift and OR reconstruct bytes |
| Inter-process communication | Two separately launched programs exchange a message through a shared encoding convention |
| Protocol design | A NUL byte marks the end of a message; fixed sender pacing exposes the need for receiver feedback |

## Limitations

- **No acknowledgements or delivery verification.** The server sends no confirmation for a bit, byte, or complete message. The client has no retry mechanism. A successful client exit means its `kill()` calls succeeded, not that the server reconstructed the message correctly.
- **Timing-dependent delivery.** `usleep(300)` provides fixed pacing, without feedback from the receiver. On Linux, repeated instances of an already-pending standard signal can collapse into one, and the delivery order of different pending standard signals is unspecified. This protocol therefore cannot guarantee intact messages under load. See [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html).
- **One client at a time.** The server has a single accumulator and bit counter, with no sender identification or per-client state. Signals from concurrent clients can mix into the same byte stream.
- **No recovery from missing bits.** There is no sequence numbering, checksum, or resynchronization mechanism. A lost signal can misalign subsequent bytes; restart the server before trying again after a corrupted or interrupted transmission.
- **Limited handler masking.** `sa_mask` is empty and `sa_flags` is zero. The setup does not explicitly block both protocol signals while the handler runs, so the other signal type can interrupt it and access the shared decoding state. This follows from the setup in [`server.c`](minitalk/server.c) and the handler-mask semantics in [sigaction(2)](https://man7.org/linux/man-pages/man2/sigaction.2.html).
- **Permissive PID parsing.** The local `ft_atoi()` accepts an initial numeric portion and stops at the first non-digit; it does not validate the entire argument. Enter only the actual numeric server PID.
- **Byte-oriented messages.** The program sends bytes without decoding or validating a character encoding. An embedded NUL cannot be part of the command-line message; it is the terminator.

These descriptions are based on source inspection. The examples are usage instructions, not recorded reliability or stress-test results.

## Source guide

| File | Responsibility |
| --- | --- |
| [`minitalk/client.c`](minitalk/client.c) | PID handling, byte encoding, and signal transmission |
| [`minitalk/server.c`](minitalk/server.c) | Handler registration, byte reconstruction, and output |
| [`minitalk/utils.c`](minitalk/utils.c) | String length, file-descriptor output, number printing, and PID conversion helpers |
| [`minitalk/minitalk.h`](minitalk/minitalk.h) | System includes and utility declarations |
| [`minitalk/Makefile`](minitalk/Makefile) | Build and cleanup targets for the two programs |

## Credits

Repository owner and recorded Git commit author: [tommysoeder](https://github.com/tommysoeder).

System-call references: [kill(2)](https://man7.org/linux/man-pages/man2/kill.2.html), [sigaction(2)](https://man7.org/linux/man-pages/man2/sigaction.2.html), and [signal(7)](https://man7.org/linux/man-pages/man7/signal.7.html).

## AI usage

This README was prepared with AI assistance and checked against the repository source. The repository does not contain a statement about AI use during the original implementation.
