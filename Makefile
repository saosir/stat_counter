CXX=g++
CFLAGS=-g -Wall -O2

all:stat_counter_test

clean:
	rm -fr $(PROGS) a.out *.o

stat_counter_test: stat_counter_test.cpp stat_counter.h
	$(CXX) $(CFLAGS) -o $@ $<