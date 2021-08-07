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
      * All references are removed from the semaphores.
      * This gives us a clean "start".
      */
    sem_unlink(SEMAPHORE_ZERO);
    sem_unlink(SEMAPHORE_OCCUPIED);



    /**
     * Request a key.
     * The key is linked to a filename,
     * so that other programs can access it.
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



    /* A new semaphore is created or an existing one is opened, Init value set to the ringBufferSize. */
    sem_t *sem_write = sem_open(SEMAPHORE_OCCUPIED, O_CREAT, S_IRWXU | S_IRWXG, ringBufferSize);
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

        /* Decrement 'sem_write' -1 and read in the first character (byte) */
        while (sem_wait(sem_write) == -1){
            /**
             * Many system calls will report the 'EINTR' error code if a signal occurred while the system call was in progress.
             * No error actually occurred, it's just reported that way because the system isn't able to resume the system call automatically.
             */
            if(errno != EINTR){
                fprintf(stderr,"Error with sem_write: %s\n",strerror(errno));
                return -1;
            }
        }

        /**
         * We check errors while printing the read characters to STDOUT
         * and 'clearerr' is needed to detect this. The function clearerr() clears
         * the end-of-file and error indicators for the stream pointed to by STREAM.
         */
        clearerr(stdin);
        if((*addressSpace = getc(stdin)) == EOF){
            if(ferror(stdin) != 0){ //return the error indicator for the STREAM
                fprintf(stderr,"Error while reading from STDIN: %s",strerror(errno));
                return -1;
            }
        }

        /* When reading of the characters is finished (EOF), sem_read is released and incremented. We leave the while. */
        if(*addressSpace == EOF){
            if(sem_post(sem_read) == -1){
                fprintf(stderr,"Error with sem_read: %s",strerror(errno));
                return -1;
            }
            break;
        }

        /* Our created shared memory ring buffer */
        if (addressSpace != addressSpaceTail){
            addressSpace += 1;
        }
        else{
            addressSpace = addressSpaceHead;
        }

        /*
         * Now I release sem_read and increment from 0 to 1 and program 'write' can step into the critical section and read
         * the character from the ring buffer into e.g. a textfile .
         */
        if(sem_post(sem_read) == -1){
            fprintf(stderr,"Error with sem_read: %s",strerror(errno));
            return -1;
        }   
    }

    /**
     * shmctl: Shared memory control operation
     * IPC_RMID: Mark the segment to be destroyed. The segment will  actually
     * be  destroyed only after the last process detaches it.
     */
    if(shmctl(shared_block_id, IPC_RMID, NULL) == -1 ) {
        return IPC_REST_ERROR;
    }



    return 0;
}


