#ifndef _RSHELL_
	#define _RSHELL_


static pid_t RSHELL_PID;
static pid_t RSHELL_PGID;
static int RSHELL_TERMINAL, RSHELL_IS_INTERACTIVE;
static struct termios RSHELL_TMODES;

void get_line();
void make_command();
void clear_command();
t_job * insert_job(pid_t pid, pid_t pgid, char* name, char* descriptor,
                  int status);
t_job* del_job(t_job* job);
t_job* get_job(int searchValue, int searchParameter);
void print_jobs();
void hello_screen();
int change_job_status(int pid, int status);
void prompt();

#endif