#include <stdio.h>
#include "/include/rShell.h"

void helloScreen()
{
	printf("Welcome to rShell!");
}

void shellPrompt()
{
        printf("%s :> ",getcwd(currentDirectory, 1024));
}

