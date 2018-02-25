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
GNU gdb (GDB) 8.0.1
Copyright (C) 2017 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-apple-darwin16.7.0".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb) target remote tcp:localhost:1234
Remote debugging using tcp:localhost:1234
Ignoring packet error, continuing...
warning: unrecognized item "timeout" in "qSupported" response
Remote connection closed
```

Full output of server process:

``` bash
$ ./gdb_dummy
Listening on port 1234. Open a GDB instance and enter "target remote tcp:localhost:1234"
[Connection from 127.0.0.1:62022]
[buffer] empty. reading...
[buffer] received 1 bytes: {{{+}}}
<- OK
[main] client sent OK
[buffer] empty. reading...
[buffer] received 159 bytes: {{{$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386#6a}}}
[buffer] empty. reading...
[buffer] received 159 bytes: {{{$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386#6a}}}
<- client supports: multiprocess
<- break msg: swbreak
<- break msg: hwbreak
<- whatever that means, it parses OK: qRelocInsn
<- whatever that means, it parses OK: fork-events
<- v-prefix msg: fork-events
<- whatever that means, it parses OK: exec-events
<- v-prefix msg: ContSupported
<- whatever that means, it parses OK: QThreadEvents
<- whatever that means, it parses OK: no-resumed
<- REGISTER support: i386
[buffer] empty. reading...
[buffer] received 159 bytes: {{{$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;no-resumed+;xmlRegisters=i386#6a}}}
[main] Read successful. Sending ACK.
[buffer] empty. reading...
[buffer] received 1 bytes: {{{-}}}
<- client wants retransmit Plz!
[main] Read successful. Sending ACK.
[buffer] empty. reading...
[buffer] received 1 bytes: {{{-}}}
<- client wants retransmit Plz!
[main] Read successful. Sending ACK.
[buffer] empty. reading...
[buffer] received 1 bytes: {{{-}}}
<- client wants retransmit Plz!
[main] Read successful. Sending ACK.
[buffer] empty. reading...
[buffer] received 20 bytes: {{{+$vMustReplyEmpty#3a}}}
[buffer] empty. reading...
[buffer] received 1 bytes: {{{-}}}
[main] Read error
```

