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
#include "rShell-defs.h"

#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
#define WAITING_INPUT 'W'
#define STDIN 1
#define STDOUT 2
#define MAXLINE 4096

void handle_command();
int check_built_in_commands();
void exec_command(char *command[], char *file, int newDescriptor,
                  int executionMode);
void launch_job(char *command[], char *file, int newDescriptor,
               int executionMode);
void put_foreground(t_job* job, int continueJob);
void put_background(t_job* job, int continueJob);
void wait_job(t_job* job);
void kill_job(int jobId);
void change_dir();
void init();
void sig_child(int p);
void pipeline(int);

char userInput = '\0';
int bufferChars = 0;
int commandArgc = 0;
char *commandArgv[5];
char* currentDirectory;
char buffer[BUFFER_MAX_LENGTH];

t_job *jobsList = NULL;

void handle_command()
{
        if (check_built_in_commands() == 0) {
                launch_job(commandArgv, "STANDARD", 0, FOREGROUND);
        }
}

/* 
 * Pipelining is acheived by simple use of popen and pclose
 * interprocess communication tools. The acccepted value is a flag
 * which indicates the number of levels of pipelinine to be done.
 * supports up to 5 levels of pipelining.
 */
 void pipeline(int flag)
 {
    char    line[MAXLINE];
    FILE    *fpin, *fpout;
    char arg1[50],arg2[50];
    int i;
   if(flag==2)
   {
	    strcat(arg1,commandArgv[0]);
	    strcpy(arg2,commandArgv[2]);
   }
    else
   {
	   for(i=0;i<flag-1;i++)
	   {
		   strcat(arg1,commandArgv[i]);
		   strcat(arg1," ");
	   }
	   strcpy(arg2,commandArgv[commandArgc-1]);
   }
   puts(commandArgv[0]);
   puts(arg1);
   puts(arg2);

    if ((fpin = popen(arg1, "r")) == NULL)
        printf("can't open %s", arg1);

    if ((fpout = popen(arg2, "w")) == NULL)
        printf("popen error");


    while (fgets(line, MAXLINE, fpin) != NULL) {
        if (fputs(line, fpout) == EOF)
            printf("fputs error to pipe");
    }
    if (ferror(fpin))
        printf("fgets error");
    if (pclose(fpout) == -1)
        printf("pclose error\n");
}

/*
 * tl;dr this :P
 * The PID of any of the signals childs are returns through the 
 * waitpid system call with a WAIT_ANY flag set. The pid of that 
 * child is returned. Using this PID, the shell is queried for the
 * jobs node. The returned job is checked for on of the following conditions:
 * 
 * 1) If Backgroud job and exitted: simply remoe it from the pool
 * 2) If signalled by kernel, remove from pool and clean up.
 * 3) If stopped, set status as waiting (mostly for user i/p) and take over session.
 * 4) Else, place it as suspended, take over session and process nect job.
 */  
void signalHandler_child(int p)
{
        pid_t pid;
        int terminationStatus;

        pid = waitpid(WAIT_ANY, &terminationStatus, WUNTRACED | WNOHANG);
        if (pid > 0) {
                t_job* job = get_job(pid, BY_PROCESS_ID);
                if (job == NULL)
                        return;
                if (WIFEXITED(terminationStatus)) {
                        if (job->status == BACKGROUND) {
                                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                                jobsList = del_job(job);
                        }
                } else if (WIFSIGNALED(terminationStatus)) {
                        printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
                        jobsList = del_job(job);
                } else if (WIFSTOPPED(terminationStatus)) {
                        if (job->status == BACKGROUND) {
                                tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID);
                                change_job_status(pid, WAITING_INPUT);
                                printf("\n[%d]+   suspended [wants input]\t   %s\n",
                                       numActiveJobs, job->name);
                                prompt();
                        } else {
                                tcsetpgrp(RSHELL_TERMINAL, job->pgid);
                                change_job_status(pid, SUSPENDED);
                                printf("\n[%d]+   stopped\t   %s\n", numActiveJobs, job->name);
                        }
                        return;
                } else {
                        if (job->status == BACKGROUND) {
                                jobsList = del_job(job);
                        }
                }
                tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID);
        }
}

/* 
 * Built in commands and checked by a small grounp of if statements 
 * match the given input. The shell itself executes these command.
 * There is no context switch to kernel space, and control lies
 * within shell space. Usage of built-in commands on super user
 * space requires acess permissions. Run hell in sudo. 
 * WITH CAUTION!
 */
int check_built_in_commands()
{
        if (strcmp("exit", commandArgv[0]) == 0) {
                exit(EXIT_SUCCESS);
        }
        if (strcmp("cd", commandArgv[0]) == 0) {

                change_dir();
                return 1;
        }

        if (strcmp("bg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                if (strcmp("in", commandArgv[1]) == 0)
                        launch_job(commandArgv + 3, *(commandArgv + 2), STDIN, BACKGROUND);
                else if (strcmp("out", commandArgv[1]) == 0)
                        launch_job(commandArgv + 3, *(commandArgv + 2), STDOUT, BACKGROUND);
                else
                        launch_job(commandArgv + 1, "STANDARD", 0, BACKGROUND);
                return 1;
        }
        if (strcmp("fg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                int jobId = (int) atoi(commandArgv[1]);
                t_job* job = get_job(jobId, BY_JOB_ID);
                if (job == NULL)
                        return 0;
                if (job->status == SUSPENDED || job->status == WAITING_INPUT)
                        put_foreground(job, TRUE);
                else                                                                                                // status = BACKGROUND
                        put_foreground(job, FALSE);
                return 1;
        }
        if (strcmp("jobs", commandArgv[0]) == 0) {
                //printJobs();
                return 1;
        }
        if (strcmp("kill", commandArgv[0]) == 0)
        {
                if (commandArgv[1] == NULL)
                        return 0;
                kill_job(atoi(commandArgv[1]));
                return 1;
        }
        //int i;
        ////for( i=0;i<=commandArgc;i++)
        //{
        //char *str;
        //strcpy(str,commandArgv);
        if( commandArgc == 3 )
        {
        if(strcmp("|", commandArgv[1]) == 0  )
            {
            pipeline(2);
            return 1;
            }
            //return 1;
        }
    if( commandArgc > 3 )
{
        if(strcmp("|", commandArgv[commandArgc-2]) == 0 )
        {
            //puts("works");
            //printf("%d",commandArgc);
            pipeline(commandArgc-1);
            return 1;
        }
    }
        //break;
        //}



        return 0;
}

/*
 * Here, we simply provide file descriptors for input and out put so that
 * are taken and arrive on the same terminal / console. 
 */
void exec_command(char *command[], char *file, int newDescriptor,
                    int executionMode)
{
        int commandDescriptor;
        if (newDescriptor == STDIN) {
                commandDescriptor = open(file, O_RDONLY, 0600);
                dup2(commandDescriptor, STDIN_FILENO);
                close(commandDescriptor);
        }
        if (newDescriptor == STDOUT) {
                commandDescriptor = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                dup2(commandDescriptor, STDOUT_FILENO);
                close(commandDescriptor);
        }
        if (execvp(*command, command) == -1)
                perror("RSHELL");
}

/*
 * Job is to be lay=unched after verification that we are nt=ot 
 * trying to execute a built-in command, as they are done by the shell 
 * itself. Once checked, we issue the command to launch_job that forks,
 * sets up signal handlers and allows the child to complete the task.
 * The parent waits for the child to return with status.
 */
void launch_job(char *command[], char *file, int newDescriptor,
               int executionMode)
{
        pid_t pid;
        pid = fork();
        switch (pid) {
        case -1:
                perror("rShell fail: ");
                exit(EXIT_FAILURE);
                break;
        case 0:
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, &signalHandler_child);
                signal(SIGTTIN, SIG_DFL);
                usleep(20000);
                setpgrp();
                if (executionMode == FOREGROUND)
                        tcsetpgrp(RSHELL_TERMINAL, getpid());
                if (executionMode == BACKGROUND)
                        printf("[%d] %d\n", ++numActiveJobs, (int) getpid());

                exec_command(command, file, newDescriptor, executionMode);

                exit(EXIT_SUCCESS);
                break;
        default:
                setpgid(pid, pid);

                jobsList = insert_job(pid, pid, *(command), file, (int) executionMode);

                t_job* job = get_job(pid, BY_PROCESS_ID);

                if (executionMode == FOREGROUND) {
                        put_foreground(job, FALSE);
                }
                if (executionMode == BACKGROUND)
                        put_background(job, FALSE);
                break;
        }
}

/*
 * Used to allow the current process to become the fore ground 
 * process by becoming a sessions leader. Sessions leader access is 
 * gained by tscetpgrp function. Functionality to bring a current 
 * job in background to foreground not tested yet.
 */
void put_foreground(t_job* job, int continueJob)
{
        job->status = FOREGROUND;
        tcsetpgrp(RSHELL_TERMINAL, job->pgid);
        if (continueJob) {
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");
        }

        wait_job(job);
        tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID);
}

/*
 * Places the ongoing job in the background.
 * Usus the defined macro BACKGROUND.
 */
void put_background(t_job* job, int continueJob)
{
        if (job == NULL)
                return;

        if (continueJob && job->status != WAITING_INPUT)
                job->status = WAITING_INPUT;
        if (continueJob)
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");

        tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID);
}

/* 
 * Causes parent, i.e, the current process, that spawned a child to complete
 * a job, to wait for the child to complete execution and return an exit status.
 * The signal handler sid_child handles signals of a child process.
 */
void wait_job(t_job* job)
{
        int terminationStatus;
        while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {
                if (job->status == SUSPENDED)
                        return;
        }
        jobsList = del_job(job);
}

/*
 * kill a job and remove from queue
 */
void kill_job(int jobId)
{
        //t_job *job = get_job(jobId, BY_JOB_ID);
        kill(jobId, SIGKILL);
}

/*
 * Simple implementation of cd Command as an internal command.
 */
void change_dir()
{
        if (commandArgv[1] == NULL) {
                chdir(getenv("HOME"));
        } else {
                if (chdir(commandArgv[1]) == -1) {
                        printf(" %s: no such directory\n", commandArgv[1]);
                }
        }
}

/*
 * initialize the shell with terminals, file despcriptors etc.
 * Set up signl masks and callback functions to handle signals.
 * Setup directories
 */
void init()
{
        RSHELL_PID = getpid();
        RSHELL_TERMINAL = STDIN_FILENO;
        RSHELL_IS_INTERACTIVE = isatty(RSHELL_TERMINAL);

        if (RSHELL_IS_INTERACTIVE) {
                while (tcgetpgrp(RSHELL_TERMINAL) != (RSHELL_PGID = getpgrp()))
                        kill(RSHELL_PID, SIGTTIN);

                signal(SIGQUIT, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
                signal(SIGTTIN, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                signal(SIGINT, SIG_IGN);
                signal(SIGCHLD, &signalHandler_child);

                setpgid(RSHELL_PID, RSHELL_PID);
                RSHELL_PGID = getpgrp();
                if (RSHELL_PID != RSHELL_PGID) {
                        printf("Error, the shell is not process group leader");
                        exit(EXIT_FAILURE);
                }
                if (tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID) == -1)
                        tcgetattr(RSHELL_TERMINAL, &RSHELL_TMODES);

                currentDirectory = (char*) calloc(1024, sizeof(char));
        } else {
                printf("Could not make RSHELL interactive. Exiting..\n");
                exit(EXIT_FAILURE);
        }
}

int main(int argc, char **argv, char **envp)
{
        init();
        hello_screen();
        prompt();

        while (TRUE) {
                userInput = getchar();
                switch (userInput) {
                case '\n':
                        prompt();
                        break;
                default:
                        get_line();
                        handle_command();
                        prompt();
                        break;
                }
        }
        printf("\n");
        return 0;
}

