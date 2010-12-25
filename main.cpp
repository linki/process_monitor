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
    pm->proc_path("test/proc");

    pm->start();

    for (int i = 0; i < 20; ++i)
    {
	    printf("%lu %lu %lu %lu\n", pm->__last_stat.utime, pm->__stat.utime, pm->__last_system_stat.utime, pm->__last_system_stat.utime);        
        sleep(1);
    }

    pm->stop();

    return 0;
}








// src/lib/net handler
