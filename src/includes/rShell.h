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

static pid_t RSHELL_PID;
static pid_t RSHELL_PGID;
static int RSHELL_TERMINAL, RSHELL_IS_INTERACTIVE;
static struct termios RSHELL_TMODES;

void pipeline(int);
void getNextLine();

void make_command();

void clear_command();

t_job * insert_job(pid_t pid, pid_t pgid, char* name, char* descriptor,
                  int status);

t_job* del_job(t_job* job);

t_job* get_job(int searchValue, int searchParameter);

//void printJobs();

void hello_creen();

void shell_prompt();
void handle_command();

int check_built_in_commands();

void exec_ommand(char *command[], char *file, int newDescriptor,
                    int executionMode);

void exec_Job(char *command[], char *file, int newDescriptor,
               int executionMode);

void put_foreground(t_job* job, int continueJob);

void put_background(t_job* job, int continueJob);

void wait_for_job(t_job* job);

void kill_ob(int jobId);

void change_dir();

void init();

void sig_child(int p);
