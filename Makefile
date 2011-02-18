app: main.cpp process_monitor.h process_monitor.cpp
	g++ -o main main.cpp process_monitor.cpp -lpthread -O3 -pipe -Wall

test: process_monitor_test.cpp process_monitor.h process_monitor.cpp
	g++ -o process_monitor_test process_monitor_test.cpp process_monitor.h process_monitor.cpp -lpthread -lgtest -lgtest_main -pipe -Wall
	./process_monitor_test

uncrustify: process_monitor_test.cpp process_monitor.h process_monitor.cpp main.cpp
	uncrustify -c ../../.uncrustify process_monitor_test.cpp process_monitor.h process_monitor.cpp main.cpp

clean:
	rm -f main process_monitor process_monitor_test
	rm -f *.uncrustify
