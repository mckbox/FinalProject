#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>     /*  sem_t, pid_t      */
#include <sys/wait.h>       /* waitpid    */
#include <errno.h>          /* errno        */
#include <semaphore.h>      
#include <fcntl.h>  
#include <unistd.h>

#define NUM_OF_TRAINERS 5  /* For range still need to work on struct with lowerBount and upperBound */
#define NUM_OF_COUCHES 6
       

int main (int argc, char **argv){
    int i;                        /*      looping variables          */   
    sem_t *init_sem;                   /*      synch semaphore         */
    pid_t pid;                    /*      fork pid                */
    int *p;                       /*      shared variable         */

    p = (int*)malloc(sizeof(int));   /* this will get changed to choaches[i] from Payton's code*/
	*p = 0;

    /* initialize semaphores for shared processes */
    init_sem = sem_open ("FitnessCentre", O_CREAT | O_EXCL, 0644, NUM_OF_TRAINERS);  /* No. of semaphore */
   
    printf ("semaphores initialized.\n\n");


    /* fork child processes */
    for (i = 0; i < 7; i++){         /* I assumed 7 as no.of customer(processes). From Payton's code "Customer" to be integrate */
        pid = fork ();
        if (pid < 0) {
        /* check for error      */
            sem_unlink ("FitnessCentre");   
            sem_close(init_sem);              
            printf ("Fork error.\n");
        }
        else if (pid == 0)
            break;    
    }


    if (pid == 0){
		sem_wait (init_sem);           /* P operation */
        printf("Customer(%d) is in critical section.\n", i);
        sleep (1);
        *p += i % NUM_OF_TRAINERS;              
        printf ("  Customer(%d) completed training with Trainer %d.\n", i, *p);
        sem_post (init_sem);           /* V operation */
        exit (0);

    }

    else{
        /* wait for all children to exit */
        while (pid = waitpid (-1, NULL, 0))
		{
            if (errno == ECHILD)
            break;
        }

        printf ("\nParent: All children have exited.\n");
        /* cleanup semaphores */
        sem_unlink ("FitnessCentre");   
        sem_close(init_sem);  
        exit (0);   
    }
}


