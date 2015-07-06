#include <stdio.h>
#include "rShell.h"
#define MAXLINE 200

void kill_job(int jobId)
{
        t_job *job = get_job(jobId, BY_JOB_ID);
        kill(job->pid, SIGKILL);
}

/*
 * Launching job sets apprpraite signal masks and forks a child to execv
 */
void exec_job(char *command[], char *file, int newDescriptor,
              int executionMode)
{
        pid_t pid;
        pid = fork();
        switch (pid) {
        case -1:
                perror("rShell Error: ");
                exit(EXIT_FAILURE);
                break;
        case 0:
                /* child sets up signal handeling, make group leader, execute*/
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, &sig_child);
                signal(SIGTTIN, SIG_DFL);
                usleep(20000);
                setpgrp();
                if (executionMode == FOREGROUND)
                        tcsetpgrp(RSHELL_TERMINAL, getpid());
                if (executionMode == BACKGROUND)
                        printf("[%d] %d\n", ++numActiveJobs, (int) getpid());

                exec_job(command, file, newDescriptor, executionMode);

                exit(EXIT_SUCCESS);
                break;
        
        default:
                /*Parent pushed job to queue and waits*/
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
                        exec_job(commandArgv + 3, *(commandArgv + 2), STDIN, BACKGROUND);
                else if (strcmp("out", commandArgv[1]) == 0)
                        exec_job(commandArgv + 3, *(commandArgv + 2), STDOUT, BACKGROUND);
                else
                        exec_job(commandArgv + 1, "STANDARD", 0, BACKGROUND);
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
                printf("Print job utility not implemented yet:\n");
                return 1;
        }
        if (strcmp("kill", commandArgv[0]) == 0)
        {
                if (commandArgv[1] == NULL)
                        return 0;
                kill_job(atoi(commandArgv[1]));
                return 1;
        }
        
        if( commandArgc == 3 )
        {
                if(strcmp("|", commandArgv[1]) == 0  )
            {
            pipeline(2);
            return 1;
            }
        }
        if( commandArgc > 3 )
{
                if(strcmp("|", commandArgv[commandArgc-2]) == 0 )
                {
                        pipeline(commandArgc-1);
                        return 1;
                }
        }
 
        return 0;
}

/*
 * BuiltInCommands are executed by the shell, such as backgrounding
 * foregrounding, change directory, etc.
 * If not built in, execute directly through execv
 */
void handle_command()
{
        if (check_built_in_commands() == 0) {
                exec_job(commandArgv, "STANDARD", 0, FOREGROUND);
        }
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

/* 
 * If | character found, use popen and pclose system calls to implement
 * Pipleining
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
 * Puts said job in fore ground by appending it with "bg"
 */
void put_foreground(t_job* job, int continueJob)
{
        job->status = FOREGROUND;
        tcsetpgrp(RSHELL_TERMINAL, job->pgid);
        if (continueJob) {
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");
        }

        wait_for_job(job);
        tcsetpgrp(RSHELL_TERMINAL, RSHELL_PGID);
}

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
 * called after exevr execution to make queue wait for job completion to put 
 * another in foreground
 */
void wait_for_job(t_job* job)
{
        int terminationStatus;
        while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {
                if (job->status == SUSPENDED)
                        return;
        }
        jobsList = del_job(job);
}



/*
 * cd command implementation
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
                signal(SIGCHLD, &sig_child);

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
                printf("Could not make MSH interactive. Exiting..\n");
                exit(EXIT_FAILURE);
        }
}

void sig_child(int signo)
{
	printf("Have no fucking clue about this.");
}

int main(int argc, char **argv, char **envp)
{
        init();
        hello_screen();
        shell_prompt();
        while (TRUE) {
                userInput = getchar();
                printf("Taken input");
                fflush(NULL);
                switch (userInput) {
                case '\n':
                        shell_prompt();
                        break;
                default:
                        get_line();
                        handle_command();
                        shell_prompt();
                        break;
                }
        }
        printf("\n");
        return 0;
}
