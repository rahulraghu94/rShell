#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include "rShell.h"

/*
 * External declarations of variables from rShell.h
 */
extern char *commandArgv[5];
extern char* currentDirectory;
extern char buffer[BUFFER_MAX_LENGTH];
extern char userInput;
extern int bufferChars;
extern int commandArgc;
extern t_job *jobsList;

/*
 * make_command purpose is to populate commandArgv[], which stores different tokens 
 * of an input command, by using c built-in tokenizer strtok().
 * The entered user string is broken down across the " " delimiter and stored
 * as serperate strings for processing.
 * Current limitation is 5 tokens per command.
 * Command string neads to be cleared before use
 */
void make_command()
{
        int i;
        char* bufferPointer;
        bufferPointer = strtok(buffer, " ");
        while (bufferPointer != NULL) {
                commandArgv[commandArgc] = bufferPointer;
                bufferPointer = strtok(NULL, " ");
                commandArgc++;
        }

}

/*
 * clear_command resets the argc count and clears the string for command
 */
void clear_command()
{
        while (commandArgc != 0) {
                commandArgv[commandArgc] = NULL;
                commandArgc--;
        }
        bufferChars = 0;
}

/*
 * Queries the user for a new command. Initially clear the command, reads the entire 
 * buffer string, and calls make_command.
 */
void get_line()
{
        clear_command();
        while ((userInput != '\n') && (bufferChars < BUFFER_MAX_LENGTH)) {
                buffer[bufferChars++] = userInput;
                userInput = getchar();
        }
        buffer[bufferChars] = 0x00;
        make_command();
}

/*
 * A new job is inserted and scheduled through the jobsList queue. 
 * A new job is allocated a memory block to holds it essentian informaation
 * such as name, ID's, status etc. 
 */
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

/*
 * Job statuc gets changes via pid to the passed status parameter
 */
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

/* Remove jobs from the pool after completion or upon unwanted/wanted termination.
 * Waiting parent process is woken up here.
 */
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

/* Fetches a job from the job pool using a parameter and value.
 * car must be taken to ensure values passed are unique
 */
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

void hello_screen()
{
        printf("Rahuls shell. :P\n\n");
}

void prompt()
{
        char *save_pointer;
        char *p, *val;
        char *buff = getenv("PWD");
        char *who = getenv("USER");
        char name[120];
        char wd[120];
        getlogin_r(who, 10);

        p = strtok_r(buff, "/", &save_pointer);

        gethostname(name, sizeof(name));
        getcwd(wd, sizeof(wd));
    
    while(p!=NULL)
    {
        p = strtok_r(NULL, "/", &save_pointer);
        if(p != NULL)
            val = p;
    }


    printf("[%s@%s %s]> ",who, name, wd);
        fflush(NULL);
}
