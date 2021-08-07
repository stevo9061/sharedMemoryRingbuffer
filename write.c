#include "head.h"

int main(int argc, char *argv[])
{


    /**
     * Takes the arguments in the command line and integer 'ringBufferSize'
     * gets assigned a size for the memory.
     *
     * put ':' at the starting of the string so compiler can distinguish between '?' and ':' .
     */
    while ((option = getopt(argc, argv, ":m:")) != -1) { //get option from the getopt() method
        switch (option){
            case 'm':
                ringBufferSize = strtol(optarg, &p, 10);
                if(ringBufferSize <= 0){
                    fprintf(stderr,"Ring buffer size must be > 0!\n");
                    break;
                }
                break;
            case ':':
                fprintf(stderr,"Option needs a value!\n");
                break;
            case '?':
                fprintf(stderr,"Unknown option: %c!\n", optopt);
                break;
        }
    }


    /**
     * Request a key.
     *  The key is linked to a filename,
     *  so that other programs can access it.
     */
    key = ftok(FILENAME,  0);
    if (key == IPC_REST_ERROR) {
        return IPC_REST_ERROR;
    }

    /**
     * Get shared memory block - Create it, if it doesn't exist
     * and returns a valid shared memory identifier. On error, -1
     */
    int shared_block_id = shmget(key, ringBufferSize, 0644 | IPC_CREAT);
    if (shared_block_id == IPC_REST_ERROR) {
        return IPC_REST_ERROR;
    }

    /**
      * Map the shared block into the address space of the calling process
      * and give me a pointer to it.
      */
    addressSpace = shmat(shared_block_id, NULL, 0);
    if (addressSpace == (char *)IPC_REST_ERROR) {
        return IPC_REST_ERROR;
    }


    /* A new semaphore is created or an existing one is opened, Init value set to 0. */
    sem_t *sem_read = sem_open(SEMAPHORE_ZERO, O_CREAT, S_IRWXU | S_IRWXG, 0);
    if (sem_read == SEM_FAILED) {
        fprintf(stderr, "ERROR with sem_open SEMAPHORE_ZERO: %s\n", strerror(errno));
        return IPC_REST_ERROR;
    }


    /* A new semaphore is created or an existing one is opened, Init value set to ringBufferSize. */
    sem_t *sem_write = sem_open(SEMAPHORE_OCCUPIED, O_CREAT, ringBufferSize);
    if (sem_write == SEM_FAILED) {
        fprintf(stderr, "ERROR with sem_open SEMAPHORE_OCCUPIED: %s\n", strerror(errno));
        return IPC_REST_ERROR;
    }


    /* We store here the first and last address of our created memory area to create the ring buffer. */
    addressSpaceHead = addressSpace;
    addressSpaceTail = (addressSpace + ringBufferSize - 1);


    /**
     * Here is a while(1) created because we cannot
     * know in advance when the input of the STDIN (EOF) is reached.
     */
    while (1){

        /**
         * Check that 'sem_read' is blocked since it is 0 and cannot be decremented further
         * until sem_post(sem_read) is released in read.c program .
         */
        while (sem_wait(sem_read) == -1){
            if(errno != EINTR){ //TODO: EINTR NECASSARY??
                fprintf(stderr,"Error with sem_read: %s\n",strerror(errno));
                return -1;
            }
        }


        /* If EOF is reached unlock/upcount sem_read and leave the endless loop */
        if(*addressSpace == EOF){
            if(sem_post(sem_write) == -1){
                fprintf(stderr,"Error with sem_write: %s",strerror(errno));
                return -1;
            }

            /* with break we get out of the endless while. */
            break;
        }

        /**
         * We check errors while printing the read characters to STDOUT
         * and 'clearerr' is needed to detect this. The function clearerr() clears
         * the end-of-file and error indicators for the stream pointed to by STREAM.
         */
        clearerr(stdout);
        if(fputc(*addressSpace, stdout) == EOF){
            if(ferror(stdout) != 0){ //return the error indicator for the STREAM
                fprintf(stderr,"Error with writing in STDOUT: %s",strerror(errno));
                return -1;
            }
        }


        /* Our created shared memory ring buffer */
        if (addressSpace != addressSpaceTail){
            addressSpace += 1;
        }
        else{
            addressSpace = addressSpaceHead;
        }

        //Unlock/upcount semaphor
        if(sem_post(sem_write) == -1){
            fprintf(stderr,"Error with sem_write: %s",strerror(errno));
            return -1;
        }
        
    }

    return 0;
}



