#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "process_monitor.h"

int main (int argc, const char* argv[])
{
    int pid;

	if (argc == 2)
	{
	    pid = atoi(argv[1]);
	}
	else
	{
		printf("USAGE: %s PID\n", argv[0]);
		return -1;
	}

	ProcessMonitor* pm = new ProcessMonitor(pid);

	pm->fetch();
	printf("%lu\n", pm->__stat.cmajflt);
	sleep(1);
    pm->fetch();
	printf("%lu\n", pm->utime());

    return 0;
}
