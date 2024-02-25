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

// The delimiter for commands
char *cmddelim = " ";

// The delimiter for the PATH env var
char *pathdelim = ":";

char **split(char *str, char *delim)
{
    int numTokens = 0;
    int len = strlen(str);
    int delimLen = strlen(delim);
    // Here we count the number of tokens
    int i = 0;
    int span = 1;
    // We countinue until we either find an empty token
    // or finish going through the string
    while (span != 0 && i < len)
    {
        // strcspn counts the number of characters from
        // the start of str that do not contain delim
        // We have it start at the ith character of str
        // pointer arithmetic
        span = strcspn(str + i, delim);
        i += span;
        i += delimLen;
        numTokens++;
    }

    // We now know the number of tokens, so we can allocate the array
    // with one extra slot for the terminating NULL
    char **result = (char **)malloc((numTokens + 1) * sizeof(char *));
    // Start the strtok chain
    char *tok = strtok(str, delim);
    // Allocate the first token into the result array, then copy it
    result[0] = (char *)malloc((strlen(tok) + 1) * sizeof(char));
    strcpy(result[0], tok);
    for (i = 1; i < numTokens; i++)
    {
        // Continue the strtok chain for each token
        tok = strtok(NULL, delim);
        // We copy the tok to new memory since strtok will
        // overwrite the pointer it returns on subsequent calls
        result[i] = (char *)malloc((strlen(tok) + 1) * sizeof(char));
        strcpy(result[i], tok);
    }

    // Add the final NULL and return
    result[numTokens] = NULL;
    return result;
}

void run(char *exe, char **args, int bg)
{
    // Make a child
    int child = fork();
    if (child == 0)
    {
        // If we are the child, execute the executable
        execv(exe, args);
        // If execv succeeds, it doesn't return
        // So if we get to this line it's failed
        // We simply exit so that we don't have multiple shells running
        exit(1);
    }
    else
    {
        // If we are the parent, then we need to check bg
        if (!bg)
        {
            // If bg is 0, then we wait for the child, otherwise we don't
            wait(NULL);
        }
    }
}

/**
 * Find the last term, check if it has an & at the end
 * @param terms The terms
 * @param bg Output parameter, whether or not to do background running
 */
void checkBg(char **terms, int *bg)
{
    char *lastTerm;
    int i = 0;
    while (terms[i] != NULL)
        i++;
    lastTerm = terms[i - 1];
    *bg = lastTerm[strlen(lastTerm) - 1] == '&';
    if (*bg)
    {
        // Chop off the & so we don't pass it to the child process
        lastTerm[strlen(lastTerm) - 1] = '\0';
        if (strlen(lastTerm) == 0)
        {
            // If we chopped off the entirety of the last term, then remove it entirely
            terms[i - 1] = NULL;
            free(lastTerm);
        }
    }
}

/**
 * Find a command in the PATH variable
 * @param terms The parsed terms from the command line
 * @param fullpath The variable to hold the full path to the exectuable
 * @param bg Whether or not to run the command in the background
 */
void searchPath(char **terms, char *fullpath, int bg)
{
    char *path = getenv("PATH");
    // split modifies it's input, so copy the variable to new memory
    char *mypath = (char *)malloc(strlen(path) + 1);
    strcpy(mypath, path);
    // Split the path by :
    char **paths = split(mypath, pathdelim);
    free(mypath);

    // Go through each path in PATH, check if a command with the right name exists
    int i = 0;
    while (paths[i] != NULL)
    {
        // Reallocate the fullpath to the right size
        fullpath = (char *)realloc(fullpath, strlen(paths[i]) + strlen(terms[0]) + 2);
        // Copy the path, then add the slash and the command
        strcpy(fullpath, paths[i]);
        strcat(fullpath, "/");
        strcat(fullpath, terms[0]);

        // If the fullpath exists, execute it
        if (!access(fullpath, X_OK | R_OK))
        {
            free(terms[0]);
            terms[0] = fullpath;
            run(fullpath, terms, bg);
            break;
        }

        i++;
    }
    // If we got to the end, then we failed to find the path
    if (paths[i] == NULL)
    {
        // We only free the fullpath if we didn't find it, since otherwise it's now terms[0]
        free(fullpath);
        printf("ERROR: Command not found: %s\n", terms[0]);
    }

    // Free the paths array, including freeing all of it's strings
    i = 0;
    while (paths[i] != NULL)
    {
        free(paths[i]);
        i++;
    }
    free(paths);
}

int handleCommand(char *cmdline)
{
    char **terms = split(cmdline, cmddelim);
    // Handle builtin commands
    int doExit = !strcmp(terms[0], "exit");
    if (!doExit)
    {
        if (!strcmp(terms[0], "pwd"))
        {
            // Get the cwd, then print it
            // The NULL pointer tells getcwd to allocate memory itself
            char *cwd = getcwd(NULL, 0);
            printf("%s\n", cwd);
            // We need to free the cwd, since we told getcwd to allocate it
            free(cwd);
        }
        else
        {
            if (!strcmp(terms[0], "cd"))
            {
                char *dir;
                // If there are no arguments, go to HOME
                if (terms[1] == NULL)
                {
                    dir = getenv("HOME");
                }
                else
                {
                    // Otherwise, we just take the argument
                    dir = terms[1];
                }

                // Change to the directory
                chdir(dir);
                // We don't need to free dir,
                // since it's either a global env var or part of terms,
                // which is freed later
            }
            else
            {
                int bg;
                // Check if we want to run in the background
                checkBg(terms, &bg);
                // If we start with a slash, it's an absolute path
                if (terms[0][0] == '/')
                {
                    // Path to a file, check for it, then run it
                    if (!access(terms[0], X_OK | R_OK))
                    {

                        run(terms[0], terms, bg);
                    }
                    else
                    {
                        printf("Error: %s not found!\n", terms[0]);
                    }
                }
                else
                {
                    // Check cwd first
                    char *cwd = getcwd(NULL, 0);
                    char *fullpath = (char *)malloc(strlen(cwd) + strlen(terms[0]) + 2);
                    // Combine the cwd with the given path
                    strcpy(fullpath, cwd);
                    strcat(fullpath, "/");
                    strcat(fullpath, terms[0]);
                    // Check if it exists, if so, run it
                    if (!access(fullpath, X_OK | R_OK))
                    {
                        free(cwd);
                        free(terms[0]);
                        terms[0] = fullpath;
                        run(fullpath, terms, bg);
                    }
                    else
                    {
                        // If we can't find it in the cwd, then we search the PATH
                        // We need to free cwd since we told getcwd to allocate it
                        free(cwd);
                        // We don't need to free fullpath, since searchPath will do it
                        searchPath(terms, fullpath, bg);
                    }
                }
            }
        }
    }

    // Free the terms, including all the strings inside the array
    int i = 0;
    while (terms[i] != NULL)
    {
        free(terms[i]);
        i++;
    }
    free(terms);

    return doExit;
}
