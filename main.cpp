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

    while (1)
    {
        pm->fetch();
        printf("%lu\n", pm->utime());
        sleep(1);
    }

    return 0;
}
