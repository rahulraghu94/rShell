
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#ifndef _RSHELL_
    #define _RSHELL_

static char *commandArgv[5];
static int commandArgc = 0;


#define FALSE 0
#define TRUE !FALSE

#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
#define WAITING_INPUT 'W'


#define STDIN 1
#define STDOUT 2


#define BY_PROCESS_ID 1
#define BY_JOB_ID 2
#define BY_JOB_STATUS 3
#define BUFFER_MAX_LENGTH 500


static int numActiveJobs = 0;

static char* currentDirectory;
static char userInput = '\0';
static char buffer[BUFFER_MAX_LENGTH];
static int bufferChars = 0;



static pid_t RSHELL_PID;
static pid_t RSHELL_PGID;
static int RSHELL_TERMINAL, RSHELL_IS_INTERACTIVE;
static struct termios RSHELL_TMODES;


typedef struct job {
        int id;
        char *name;
        pid_t pid;
        pid_t pgid;
        int status;
        char *descriptor;
        struct job *next;
} t_job;

static t_job* jobsList = NULL;

void pipeline(int);
void get_line();

void make_command();

void clear_command();

t_job * insert_job(pid_t pid, pid_t pgid, char* name, char* descriptor,
                  int status);

t_job* del_job(t_job* job);

t_job* get_job(int searchValue, int searchParameter);

//void printJobs();

void hello_screen();

void shell_prompt();
void handle_command();

int check_built_in_commands();

void exec_command(char *command[], char *file, int newDescriptor,
                    int executionMode);

void exec_Job(char *command[], char *file, int newDescriptor,
               int executionMode);

void put_foreground(t_job* job, int continueJob);

void put_background(t_job* job, int continueJob);

void wait_for_job(t_job* job);

void kill_job(int jobId);

void change_dir();

void init();

void sig_child(int p);

#endif