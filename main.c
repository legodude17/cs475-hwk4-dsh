/*
 * main.c
 *
 *  Created on: Mar 17 2017
 *      Author: david
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include "dsh.h"

#define MAX_PROC 250

int main(int argc, char *argv[])
{

	// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
	struct rlimit limit;
	limit.rlim_cur = MAX_PROC;
	limit.rlim_max = MAX_PROC;
	setrlimit(RLIMIT_NPROC, &limit);
	// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //

	char *cmdline = (char *)malloc(MAXBUF);
	while (1)
	{
		printf("dsh> ");
		if (fgets(cmdline, MAXBUF, stdin) == NULL)
		{
			printf("\n");
			break;
		}
		else if (strlen(cmdline) == 1)
		{
			// If we just press enter, just move on
			continue;
		}
		else
		{
			cmdline[strlen(cmdline) - 1] = '\0';
			// Handle the command, it returns 1 if we want to exit
			if (handleCommand(cmdline))
			{
				break;
			}
		}
	}

	// Free the cmdline, which we allocated at the start
	free(cmdline);
	return 0;
}
