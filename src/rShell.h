#define BUFFER_MAX_LENGTH 50
#define TRUE 1
#define FALSE !TRUE
#define BY_JOB_STATUS 3
#define BY_PROCESS_ID 1
#define BY_JOB_ID 2

extern char *commandArgv[5];
extern char* currentDirectory;
extern char userInput;
extern char buffer[BUFFER_MAX_LENGTH];
extern int bufferChars;
extern int commandArgc;


typedef struct job {
        int id;
        char *name;
        pid_t pid;
        pid_t pgid;
        int status;
        char *descriptor;
        struct job *next;
} t_job;

extern t_job *jobsList;

static int numActiveJobs = 0;
