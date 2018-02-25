
CPPFLAGS=-O2 -std=c++14 -Iattoparsecpp/include

SRCS=main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: gdb_dummy

gdb_dummy: $(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm -rf $(OBJS) gdb_dummy
