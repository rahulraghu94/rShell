#include "rShell.h"

#ifndef _STRPARSER_
#define _STRPARSER_

void clear_command()
{
        while (commandArgc != 0) {
                commandArgv[commandArgc] = NULL;
                commandArgc--;
        }
        bufferChars = 0;
}

void make_command()
{
        char* bufferPointer;
        bufferPointer = strtok(buffer, " ");
        while (bufferPointer != NULL) {
                commandArgv[commandArgc] = bufferPointer;
                bufferPointer = strtok(NULL, " ");
                commandArgc++;
        }
}


void get_line()
{
        printf("Command reading...\n");
        fflush(NULL);
        clear_command();
        while ((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH)) {
                buffer[bufferChars++] = userInput;
                userInput = getchar();
        }
        buffer[bufferChars] = 0x00;
        make_command();
}

#endif