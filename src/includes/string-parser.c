void get_line()
{
        destroyCommand();
        while ((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH)) {
                buffer[bufferChars++] = userInput;
                userInput = getchar();
        }
        buffer[bufferChars] = 0x00;
        populateCommand();
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

void clear_command()
{
        while (commandArgc != 0) {
                commandArgv[commandArgc] = NULL;
                commandArgc--;
        }
        bufferChars = 0;
}