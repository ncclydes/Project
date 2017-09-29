//
//  mysh.c
//  Project2
//
//  Created by Patrick McCabe on 9/28/17.
//
//

#include "mysh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define READ  0
#define WRITE 1

// Args holds our command information
char* args[512];
char* args2[512];
int j = 0;
int background = 0;

int main(int argc, char* argv[]) {
    int numProgs = 1;
    char line[1024];
    int i = 0;
    char* inFile;
    char* outFile;
	if (argv[1] != NULL)
	{
		myerror("Y");
		exit(1);
	}
    // Loop the prompt
    while (1) {
        // Print the command prompt
        printf("mysh (%d)> ", numProgs);
        fflush(NULL);
		inFile = NULL;
		outFile = NULL;
        // Read the command line
        if (fgets(line, 1024, stdin) == NULL)
		{
			exit(0);
		}
        // Split the line
        //char* cmd = line;
        char *token;
		char *token2;
		token = strtok(line, "|"); // Piece 1
		token2 = strtok(NULL, "|"); // Piece 2
        splitstuff(token);
		if (token2 != NULL) {
			splitstuff2(token2);
			continue;
		}
		if (args[0] == NULL) {
			continue;
		}
        while (args[i] != NULL) {
            if (strstr(args[i],"<") != NULL) {
                args[i] = NULL;
                inFile = args[++i];
            }
            if (strstr(args[i],">") != NULL) {
                args[i] = NULL;
                outFile = args[++i];
            }
			if (strstr(args[i], "&") != NULL) {
				background = 1;
				args[i] = NULL;
				break;
			}
            i++;
        }
		if (background) {
			background = 0;
			numProgs++;
			continue;
		}
		j = i;
        // Handle Exit
        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }
		if (strcmp(args[0], "cd") == 0) {
			cd(args[1]);
			numProgs++;
			continue;
		}
        if (inFile != NULL || outFile != NULL) {
            forkForRedirection(inFile,outFile);
        }
        // Handle pwd
        else if (strcmp(args[0], "pwd") == 0) {
		    if (args[1] != NULL) {
			    myerror("g");
				numProgs++;
				continue;
		    }
            mypwd();
        }
		else {
			forkWithoutRedirection();
		}
        numProgs++;
        i = 0;
        args[0] = NULL;
    }
    return 0;
}
// SPLIT ME LINE ENTRY
void splitstuff(char* line) {
    // skip dat white space
    line = skipwhite(line);
    char* next = strchr(line, ' ');
    int i = 0;
    
    while (next != NULL) {
        next[0] = '\0';
        args[i] = line;
        ++i;
        line = skipwhite(next + 1);
        next = strchr(line, ' ');
    }
    
    if (line[0] != '\0') {
        args[i] = line;
        next = strchr(line, '\n');
        next[0] = '\0';
        ++i;
    }
    
    args[i] = NULL;
}

void splitstuff(char* line) {
    // skip dat white space
    line = skipwhite(line);
    char* next = strchr(line, ' ');
    int i = 0;
    
    while (next != NULL) {
        next[0] = '\0';
        args2[i] = line;
        ++i;
        line = skipwhite(next + 1);
        next = strchr(line, ' ');
    }
    
    if (line[0] != '\0') {
        args2[i] = line;
        next = strchr(line, '\n');
        next[0] = '\0';
        ++i;
    }
    
    args2[i] = NULL;
}
// My implementation of skipping whitespace
char* skipwhite(char* s)
{
    while (isspace(*s)) ++s;
    return s;
}
// My implementation of pwd
int mypwd(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n",cwd);
    } else {
        myerror("f");
    }
    return 0;
}

int cd(char *arg) {
	if (arg == NULL) {
		myerror("i");
	} else {
		if (chdir(arg) != 0) {
			myerror("e");
		}
	}
	return 0;
}

int forkForRedirection(char* inFile, char* outFile) {
    if (fork() == 0) {
        int in = 0;
		int out = 0;
        // open input and output files
        if (inFile != NULL) {
			in = open(inFile, O_RDONLY);
			if (in < 0) {
				myerror("b");
				exit(0);
			}
		}
		if (outFile != NULL) {
			out = open(outFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			if (out < 0) {
				myerror("c");
				exit(0);
			}
		}
        // replace standard input with input file
        if (in) {
            dup2(in, 0);
            close(in);
        }
        // replace standard output with output file
        if (out) {
            dup2(out, 1);
            close(out);
        }
        // Handle pwd
        if (strcmp(args[0], "pwd") == 0) {
            mypwd();
            exit(0);
        }
        if (execvp(args[0], args) == -1) {
            myerror("d");
        }
		exit(1);
    }
	else {
		if (background == 1) {
			background = 0;
			return 0;
		}
		wait(NULL);
	}
    return 0;
}

void forkWithoutRedirection(void) {
	if (fork() == 0) {
		if (execvp(args[0], args) == -1) {
			myerror("a");
		}
		exit(1);
	} else {
		if (background == 1) {
			background = 0;
			return;
		}
		wait(NULL);
	}
	return;
}

void myerror(char *error_message) {
	error_message = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

