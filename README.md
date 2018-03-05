# GDB Dummy

This application is a little demo-showcase that tries to implement the GDB protocol using the [attoparsecpp](https://github.com/tfc/attoparsecpp) library.

It is not complete at all.

## How to run

In one shell run:

``` bash
$ ./gdb_dummy
Listening on port 1234. Open a GDB instance and enter "target remote tcp:localhost:1234"
```

In another shell:

``` bash
$ gdb
(gdb) target remote tcp:localhost:1234
```

### Example output

> Currently erroring-out because i did not implement the `g` (show register content) message, yet.

Client:

``` bash
$ gdb
GNU gdb (GDB) 8.0.1
Copyright (C) 2017 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-unknown-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb) target remote tcp:localhost:1234
Remote debugging using tcp:localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
Remote connection closed
(gdb) set debug remote 1
(gdb) target remote tcp:localhost:1234
Remote debugging using tcp:localhost:1234
Sending packet: $qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386#6a...Ack
Packet received: PacketSize=2000
Packet qSupported (supported-packets) is supported
Sending packet: $vMustReplyEmpty#3a...Ack
Packet received:
Sending packet: $Hg0#df...Ack
Packet received:
Sending packet: $qTStatus#49...Ack
Packet received: T0
Packet qTStatus (trace-status) is supported
Sending packet: $qTfV#81...Ack
Packet received:
Sending packet: $?#3f...Ack
Packet received: S05
Sending packet: $qfThreadInfo#bb...Ack
Packet received:
Sending packet: $qL1200000000000000000#50...Ack
Packet received:
Sending packet: $Hc-1#09...Ack
Packet received:
Sending packet: $qC#b4...Ack
Packet received:
Sending packet: $qAttached#8f...Ack
Packet received: 1
Packet qAttached (query-attached) is supported
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
Sending packet: $g#67...Ack
Packet received:
Bad register packet; fetching a new packet
Timed out.
Remote connection closed
(gdb) quit
```

Server:

``` bash
$ ./gdb_dummy
Listening on port 1234. Open a GDB instance and enter "target remote tcp:localhost:1234"
[Connection from 127.0.0.1:46428]
<- + (ACK)
-> +
<- [qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386]
-> +
<- Supported
-> $PacketSize=2000#f2
<- + (ACK)
<- [vMustReplyEmpty]
-> +
<- vMustReplyEmpty (UNKNOWN)
-> $#00
<- + (ACK)
<- [Hg0]
-> +
<- Hg0 (UNKNOWN)
-> $#00
<- + (ACK)
<- [qTStatus]
-> +
<- Status
-> $T0#84
<- + (ACK)
<- [qTfV]
-> +
<- fV (whatever that means)
-> $#00
<- + (ACK)
<- [?]
-> +
<- ?
-> $S05#b8
<- + (ACK)
<- [qfThreadInfo]
-> +
<- qfThreadInfo (UNKNOWN)
-> $#00
<- + (ACK)
<- [qL1200000000000000000]
-> +
<- qL1200000000000000000 (UNKNOWN)
-> $#00
<- + (ACK)
<- [Hc-1]
-> +
<- Hc-1 (UNKNOWN)
-> $#00
<- + (ACK)
<- [qC]
-> +
<- qC (UNKNOWN)
-> $#00
<- + (ACK)
<- [qAttached]
-> +
<- Attached
-> $1#31
<- + (ACK)
<- [g]
-> +
<- g (UNKNOWN)
-> $#00
<- + (ACK)
[main] Read error
```

