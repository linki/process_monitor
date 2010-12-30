CC=g++
CFLAGS=-Wall -g
CLIBS=-lgtop-2.0 -lgtop_sysdeps-2.0 -lgtop_common-2.0
PKG_CONFIG=`pkg-config --libs --cflags libgtop-2.0`

app:
	g++ -o main main.cpp process_monitor.cpp -lpthread

test: compile
	./process_monitor_test

compile: process_monitor_test.cpp process_monitor.h process_monitor.cpp
	$(CC) -o process_monitor_test process_monitor_test.cpp process_monitor.h process_monitor.cpp -lgtest -lgtest_main -lpthread -Wall -g

uncrusty:
	uncrustify -c ../../.uncrustify process_monitor.h process_monitor.cpp main.cpp

clean:
	rm -f main process_monitor process_monitor_test
