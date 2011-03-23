ifeq ($(shell uname), Linux)
LINUX_LDFLAGS = -Xlinker -rpath `pwd`
endif

main: bin/main

bin/main: src/main.o lib/libpm.so
	g++ -o bin/main src/main.o -Llib -lpm -lpthread $(LINUX_LDFLAGS) -Wall

src/main.o: src/main.cpp
	g++ -o src/main.o src/main.cpp -Iinclude -c -Wall

test: bin/test
	bin/test

bin/test: src/test.o lib/libpm.so
	g++ -o bin/test src/test.o -Llib -lpm -lpthread -lgtest -lgtest_main $(LINUX_LDFLAGS) -Wall

src/test.o: src/test.cpp
	g++ -o src/test.o src/test.cpp -Iinclude -c -Wall

lib/libpm.so: src/process_monitor.o
	g++ -o lib/libpm.so -shared src/process_monitor.o -Wall

src/process_monitor.o: include/process_monitor.h src/process_monitor.cpp
	g++ -fPIC -Iinclude -c -o src/process_monitor.o src/process_monitor.cpp -Wall

clean:
	$(RM) src/*.o
	$(RM) lib/libpm.so
	$(RM) bin/*
