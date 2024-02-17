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

	char *cmdline = (char *)malloc(MAXBUF);					// stores user input from commmand line
	char *delim = (char *)malloc(2 * sizeof(char)); // Tried to make this global, but gcc didn't like it
	delim[0] = ' ';
	delim[1] = '\0';
	while (1)
	{
		printf("dsh> ");
		if (fgets(cmdline, MAXBUF, stdin) == NULL)
		{
			printf("\n");
			break;
		}
		else
		{
			cmdline[strlen(cmdline) - 1] = '\0';
			char **terms = split(cmdline, delim);
			// Handle builtin commands
			int doExit = !strcmp(terms[0], "exit");
			if (!doExit)
			{
				if (!strcmp(terms[0], "pwd"))
				{
					char *cwd = getcwd(NULL, 0);
					printf("%s\n", cwd);
					free(cwd);
				}
				else
				{
					if (!strcmp(terms[0], "cd"))
					{
						char *dir;
						if (terms[1] == NULL)
						{
							dir = getenv("HOME");
						}
						else
						{
							dir = terms[1];
						}

						chdir(dir);
					}
					else
					{
						// Handle command running
					}
				}
			}

			// Free the terms
			int i = 0;
			while (terms[i] != NULL)
			{
				free(terms[i]);
				i++;
			}
			free(terms);

			if (doExit)
			{
				break;
			}
		}
	}

	free(cmdline);
	free(delim);
	return 0;
}
