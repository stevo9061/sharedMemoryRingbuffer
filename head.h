
/*--- DEFINE ---*/
#define SEMAPHORE_OCCUPIED "/SEMAPHORE_OCCUPIED"
#define SEMAPHORE_ZERO "/SEMAPHORE_ZERO"
#define FILENAME "write.c"
#define IPC_REST_ERROR (-1)

/*--- COMMON LIBRARIES ---*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     /* getopt */
#include <sys/shm.h>    /* shmget, shmat */
#include <sys/types.h>  /* ftok, shmat */
#include <sys/ipc.h>    /* ftok */
#include <fcntl.h>      /* sem_open */
#include <sys/stat.h>   /* sem_open */
#include <errno.h>      /* errno */
#include <semaphore.h> /* sem_open, sem_wait, sem_post */


int option;
char *p;
int ringBufferSize;
char *addressSpace;
char *addressSpaceHead;
char *addressSpaceTail;
key_t key;




