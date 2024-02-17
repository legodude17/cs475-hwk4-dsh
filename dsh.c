/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>

char **split(char *str, char *delim)
{
    int numTokens = 0;
    int len = strlen(str);
    int i = 0;
    int span = 1;
    while (span != 0 && i < len)
    {
        span = strcspn(str + i, delim);
        i += span;
        numTokens++;
    }

    char **result = (char **)malloc((numTokens + 1) * sizeof(char *));
    char *tok = strtok(str, delim);
    result[0] = (char *)malloc((strlen(tok) + 1) * sizeof(char));
    strcpy(result[0], tok);
    for (i = 1; i < numTokens; i++)
    {
        tok = strtok(NULL, delim);
        result[i] = (char *)malloc((strlen(tok) + 1) * sizeof(char));
        strcpy(result[i], tok);
    }
    result[numTokens] = NULL;

    return result;
}
