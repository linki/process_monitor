main: main.cpp libpm.so
	g++ -o main main.cpp -Iinclude -L. -lpm -lpthread -Wall

test: process_monitor_test
	./process_monitor_test

process_monitor_test: test/process_monitor_test.cpp libpm.so
	g++ -o process_monitor_test test/process_monitor_test.cpp -Iinclude -L. -lpm -lpthread -lgtest -lgtest_main -Wall

libpm.so: src/process_monitor.o
	g++ -o libpm.so -shared src/process_monitor.o

src/process_monitor.o: include/process_monitor.h src/process_monitor.cpp
	g++ -fPIC -Iinclude -c -o src/process_monitor.o src/process_monitor.cpp

clean:
	$(RM) src/*.o
	$(RM) main
	$(RM) process_monitor_test
	$(RM) libpm.so
