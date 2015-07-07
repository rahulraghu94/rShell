#ifndef _RSHELL_DEFINITIONS_
	#define _RSHELL_DEFINITIONS_

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
        printf("Comand done reading: %s", commandArgv[0]);
        make_command();
}

void make_command()
{
        int i;
        printf("Command making");
        fflush(NULL);
        char* bufferPointer;
        bufferPointer = strtok(buffer, " ");
        while (bufferPointer != NULL) {
                commandArgv[commandArgc] = bufferPointer;
                bufferPointer = strtok(NULL, " ");
                commandArgc++;
        }

        for(i=0; i< commandArgc; i++) {
                printf("%s why this not work\t", commandArgv[i]);
                fflush(NULL);
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

t_job* insert_job(pid_t pid, pid_t pgid, char* name, char* descriptor,
                 int status)
{
        usleep(10000);
        t_job *newJob = malloc(sizeof(t_job));

        newJob->name = (char*) malloc(sizeof(name));
        newJob->name = strcpy(newJob->name, name);
        newJob->pid = pid;
        newJob->pgid = pgid;
        newJob->status = status;
        newJob->descriptor = (char*) malloc(sizeof(descriptor));
        newJob->descriptor = strcpy(newJob->descriptor, descriptor);
        newJob->next = NULL;

        if (jobsList == NULL) {
                numActiveJobs++;
                newJob->id = numActiveJobs;
                return newJob;
        } else {
                t_job *auxNode = jobsList;
                while (auxNode->next != NULL) {
                        auxNode = auxNode->next;
                }
                newJob->id = auxNode->id + 1;
                auxNode->next = newJob;
                numActiveJobs++;
                return jobsList;
        }
}


int change_job_status(int pid, int status)
{
        usleep(10000);
        t_job *job = jobsList;
        if (job == NULL) {
                return 0;
        } else {
                int counter = 0;
                while (job != NULL) {
                        if (job->pid == pid) {
                                job->status = status;
                                return TRUE;
                        }
                        counter++;
                        job = job->next;
                }
                return FALSE;
        }
}

t_job* del_job(t_job* job)
{
        usleep(10000);
        if (jobsList == NULL)
                return NULL;
        t_job* currentJob;
        t_job* beforeCurrentJob;

        currentJob = jobsList->next;
        beforeCurrentJob = jobsList;

        if (beforeCurrentJob->pid == job->pid) {

                beforeCurrentJob = beforeCurrentJob->next;
                numActiveJobs--;
                return currentJob;
        }

        while (currentJob != NULL) {
                if (currentJob->pid == job->pid) {
                        numActiveJobs--;
                        beforeCurrentJob->next = currentJob->next;
                }
                beforeCurrentJob = currentJob;
                currentJob = currentJob->next;
        }
        return jobsList;
}

t_job* get_job(int searchValue, int searchParameter)
{
        usleep(10000);
        t_job* job = jobsList;
        switch (searchParameter) {
        case BY_PROCESS_ID:
                while (job != NULL) {
                        if (job->pid == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_ID:
                while (job != NULL) {
                        if (job->id == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_STATUS:
                while (job != NULL) {
                        if (job->status == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        default:
                return NULL;
                break;
        }
        return NULL;
}

/*
 * Prints welcome screen and general instructions
 */
void hello_screen()
{
	printf("Welcome to rShell!");
}


/*
 * still can't pick a symbol. $ was an excelent choice, linus. Douche.
 */
void shell_prompt()
{
        printf("[%s] : ",getcwd(currentDirectory, 1024));
}

#endif