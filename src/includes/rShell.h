#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

static int numActiveJobs = 0;

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

static pid_t MSH_PID;
static pid_t MSH_PGID;
static int MSH_TERMINAL, MSH_IS_INTERACTIVE;
static struct termios MSH_TMODES;

void pipeing(int);
void getNextLine();

void makeCommand();

void clearCommand();

t_job * insertJob(pid_t pid, pid_t pgid, char* name, char* descriptor,
                  int status);

t_job* delJob(t_job* job);

t_job* getJob(int searchValue, int searchParameter);

//void printJobs();

void HelloScreen();

void shellPrompt();
void handleUserCommand();

int checkBuiltInCommands();

void executeCommand(char *command[], char *file, int newDescriptor,
                    int executionMode);

void launchJob(char *command[], char *file, int newDescriptor,
               int executionMode);

void putJobForeground(t_job* job, int continueJob);

void putJobBackground(t_job* job, int continueJob);

void waitJob(t_job* job);

void killJob(int jobId);

void changeDirectory();

void init();

void signalHandler_child(int p);
