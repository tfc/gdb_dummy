# GDB Dummy

This application is a little demo-showcase that tries to implement the GDB protocol using the [attoparsecpp](https://github.com/tfc/attoparsecpp) library.

It is not complete at all and will never be complete. This is rather a vehicle to evaluate and extend the `attoparsecpp` library.

Current state of GDB dummy server:

- Understands initialization protocol of connecting GDB instance
- Supports reading and writing memory (Server implements dummy memory sandbox vector)
- Parser is directly attached to socket and blocks on `read()` for more bytes when it expects more input

## How to run

In one shell run:

``` bash
$ ./gdb_dummy
Listening on port 1234. Open a GDB instance and enter "target remote tcp:localhost:1234"
```

In another shell:

``` bash
$ gdb
GNU gdb (GDB) 8.1
# ...
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000ab08 in ?? ()
(gdb) x 0
0x0:    0x00000000
(gdb) s {int}0 = 0xaabbccdd   # write memory
0x0000ab08 in ?? ()
(gdb) x 0                     # read back same memory
0x0:    0xaabbccdd
(gdb) x 2
0x2:    0x0000aabb
```

