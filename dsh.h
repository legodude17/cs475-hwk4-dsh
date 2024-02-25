#define MAXBUF 256 // max number of characteres allowed on command line

/**
 * Split a string by another string
 * @param str The string to split
 * @param delim The delimiter to split by
 * @returns An array of strings, each on being a token from str
 */
char **split(char *str, char *delim);

/**
 * Run an executable with some arguments, and optionally in the background
 * @param exe The path to the executable to run
 * @param args The arguments to use
 * @param bg 1 to run in the background, 0 to not
 */
void run(char *exe, char **args, int bg);

/**
 * Handle a command
 * @param cmdline The line of text entered
 * @returns 1 to exit, 0 to not
 */
int handleCommand(char *cmdline);
