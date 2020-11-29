#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>

//this variable contains the coaches 1 means a coach is avalible (on his phone) 0 means he is busy with customer
#define WAITING_ROOM "/wroom"
#define COACHES "/trainers"
#define FIRST "/pointer"
#define MAX_SIZE sizeof(int) * 8

int w;
int c;
int p;

int c_size;
int w_size;


/** The purpose of this function is to check the avalibilty of the trainers or the waiting room
 *  @param array the array to check
 *  @param size the size of the array
 *  @param start the first seat currently
 *  @return the index of the avalible spot or a -1 if no spot is avalible
 */
int avalible(int* array, int size, int *start){

    for(int i = *start; i < size; i++){
        if(array[i] == 0){
            array[i] = 1;
            return i;
        }
    }

    for(int i = 0; i < *start; i++){
        if(array[i] == 0){
            array[i] = 1;
            return i;
        }
    }

    return -1;
}

void print_info(int *coaches, int *waiting_room){
    
    printf("Coaches:\n");
    for(int i = 0; i < c_size; i++){
        
        printf("%d ",coaches[i]);
        
    }
    printf("\nWaiting Room:\n");
    for(int i = 0; i < w_size; i++){
        
        printf("%d ",waiting_room[i]);
        
    }
    printf("\n");
}

/** The purpose of this function is to find the first customer waiting in the waiting room
 *  @param array the array to search for an open spot
 *  @param first the seat in the waiting room
 *  @param size the size of array
 *  @return the index of the first avalible customer, -1 if the waiting room is empty
 */
int first_cus(int *array, int *first, int size){

    for(int i = *first; i < w_size; i++){
        if(array[i] == 1){
            //incrementing who is first in line
            (*first)++;
            if(*first >= w_size){
                
                *first = 0;
            }
            return i;
        }
    }

    for(int i = 0; i < *first; i++){
        if(array[i] == 1){
            //incrementing who is first in line
            (*first)++;
            if(*first >= w_size){
                *first = 0;
            }
            return i;
        }
    }

    return -1;
}


int customers(int *coaches, int *waiting_room, int *first){

    int tmp = 0;

    int trainer = avalible(coaches,c_size,&tmp);
    

    //checking to see if trainers are avalible
    if(trainer == -1){

        

        //if no trainer avilible walk back to the waiting room
        sleep(0.1);
        int cust_wait = avalible(waiting_room,w_size, first);


        //if the waiting room is full
        if(cust_wait == -1){
            //leave the gym
            exit(0);
        }

        printf("Moving customer to waiting room...\n");
        print_info(coaches,waiting_room);
        fflush(stdout);

        //waiting for a trainer to come get them
        while(waiting_room[cust_wait] == 1);



        //getting the trainer and releasing it to the customer as well as the seat in the waiting room
        trainer = waiting_room[cust_wait] * -1;
        waiting_room[cust_wait] = 0;
    }

    
    //ensure the trainer is marked as busy
    coaches[trainer] = 1;
    printf("Attaching trainer %d to cust\n", trainer);
    print_info(coaches,waiting_room);
    fflush(stdout);

    //once the trainer gets the customer

    //train the customer
    sleep(1);
    //next trainer walks back to the waiting room
    sleep(.05);

    //trainer searches for a new customer in the waiting room
    int new_cust = first_cus(waiting_room, first, w_size);

    printf("cust done training\n");
    fflush(stdout);

    //no customers waiting 
    if(new_cust == -1){
        //walking back to the gym
        sleep(.05);


        //mark trainer as ready
        coaches[trainer] = 0;
        printf("Releasing Trainer #%d\n",trainer);
        print_info(coaches,waiting_room);
        fflush(stdout);
        
    }

    else{

        //linking the trainer with a waiting customer in the waiting room
        printf("moving Trainer #%d to custromer #%d\n",trainer, new_cust);
        printf("First: %d\n", *first);
        fflush(stdout);
        waiting_room[new_cust] = trainer * -1;
    }

}



/** The purpose of this function is to create a shared memory space for the custromers 
 *  to all have acces to, one for coaches and one for trainers (called AFTER fork())
 *  @param wroom_size the size of the waiting room
 *  @param trainer_size the number of trainers
 *  @return 0 on success -1 on fail
 */
int ini_ipc(int wroom_size, int trainer_size){
    
    //first creat or open a shared memory space for the coaches
    c = shm_open(COACHES, O_CREAT | O_RDWR, 0666);
    if(c < 1){
        perror("err");
    }
    ftruncate(c, sizeof(int) * trainer_size);

    int *coaches = malloc(sizeof(int) * MAX_SIZE);
    coaches = (int*)mmap(0,sizeof(int) * MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, c, 0);

    for(int i = 0; i < c_size; i++){
        coaches[i] = 0;
    }

    //now create the shared space for the customer waiting room
    w = shm_open(WAITING_ROOM, O_CREAT | O_RDWR, 0666);
    if(w < 1){
        perror("err");
    }
    ftruncate(w, sizeof(int) * wroom_size);

    //finally put the size of each of the spaces into global variables so they are shared among procs
    p = shm_open(FIRST,O_CREAT | O_RDWR, 0666);



    ftruncate(p,sizeof(int));
    int *first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, p, 0);
    *first = 0;

    return 0;
}




int main(){

    //if a coach is avalible 0
    int *coaches;
    c_size = 3;

    //this variable contains customers in the waiting room. 0 means the seat is open 1 means the seat is occupied 
    int *waiting_room;
    w_size = 6;

    coaches = malloc(sizeof(int) * MAX_SIZE);
    waiting_room = malloc(sizeof(int) * MAX_SIZE);

    int* first = malloc(sizeof(int));

    //first initialization of the shared memory space
    ini_ipc(8,8);


    //loop that creates custromers, for testing purposes we are just going to create 3 for now
    for(int i = 0; i < 18; i++){
        
        //split of a child proc
        int pid = fork();

        //execute customer function then leave
        if(pid == 0){

            //initialize the coaches memory space
            int coach = shm_open(COACHES, O_RDWR, 0666);
            coaches = (int*) mmap(0,sizeof(int) * c_size, PROT_READ | PROT_WRITE, MAP_SHARED, coach,0);

            //initialize the waiting room memory space
            int waiting = shm_open(WAITING_ROOM, O_RDWR, 0666);
            waiting_room = (int*) mmap(0,sizeof(int) * w_size, PROT_READ | PROT_WRITE, MAP_SHARED, waiting, 0);

            //initialize a variable to keep track of whose first in line
            int fir = shm_open(FIRST, O_RDWR, 0666);
            first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fir, 0);
            
            customers(coaches,waiting_room, first);

            exit(0);
            
        }
        double rand_sleep = (double)rand()/RAND_MAX *1.5;
        sleep(rand_sleep);
    }

    //initialize the coaches memory space
    int coach = shm_open(COACHES, O_RDWR, 0666);
    if(coach < 1){
        perror("err");
    }
    coaches = (int*) mmap(0,sizeof(int) * c_size, PROT_READ | PROT_WRITE, MAP_SHARED, coach,0);

    int waiting = shm_open(WAITING_ROOM, O_RDWR, 0666);
    waiting_room = (int*) mmap(0,sizeof(int) * w_size, PROT_READ | PROT_WRITE, MAP_SHARED, waiting, 0);

    int fir = shm_open(FIRST, O_RDWR, 0666);
    first = (int*) mmap(0,sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fir, 0);

    int status = 0;

    //wait for all child proc to end
    while(wait(&status) > 0);

    printf("Coaches:\n");
    for(int i = 0; i < c_size; i++){
        
        printf("%d ",coaches[i]);
        
    }
    printf("\nWaiting Room:\n");
    for(int i = 0; i < w_size; i++){
        
        printf("%d ",waiting_room[i]);
        
    }

    printf("\n");

    printf("First: %d\n",*first);

    close(c);
    close(w);
    close(p);
    shm_unlink(FIRST);
    shm_unlink(COACHES);
    shm_unlink(WAITING_ROOM);

}






 /* (This is the Bankers algorithm)
int main(){
      int b = 0;
      int count = 0, m, n, process, temp, resource; 
      int allocation_table[5] = {0, 0, 0, 0, 0};
      int available[5], current[5][5], maximum_claim[5][5];
      int maximum_resources[5], running[5], safe_state = 0;
      printf("\nEnter The Total Number Of Processes:\t");
      scanf("%d", &process);
      for(m = 0; m < process; m++) {
            running[m] = 1;
            count++;
      }
      printf("\nEnter The Total Number Of Resources To Allocate:\t");
      scanf("%d", &resource);
      printf("\nEnter The Claim Vector:\t");
      for(m = 0; m < resource; m++) { 
            scanf("%d", &maximum_resources[m]);
      }
      printf("\nEnter Allocated Resource Table:\n");
      for(m = 0; m < process; m++) {
            for(n = 0; n < resource; n++) 
            {
                  scanf("%d", &current[m][n]);
            }
      }
      printf("\nEnter The Maximum Claim Table:\n");
      for(m = 0; m < process; m++) {
            for(n = 0; n < resource; n++) 
            {
                  scanf("%d", &maximum_claim[m][n]);
            }
      }
      printf("\nThe Claim Vector \n");
      for(m = 0; m < resource; m++) {
            printf("\t%d ", maximum_resources[m]);
      }
      printf("\n The Allocated Resource Table\n");
      for(m = 0; m < process; m++) {
            for(n = 0; n < resource; n++) {
                  printf("\t%d", current[m][n]);
            }
            printf("\n");
      }
      printf("\nThe Maximum Claim Table \n");
      for(m = 0; m < process; m++) {
            for(n = 0; n < resource; n++) {
                  printf("\t%d", maximum_claim[m][n]);
            }
            printf("\n");
      }
      for(m = 0; m < process; m++) {
            for(n = 0; n < resource; n++) {
                  allocation_table[n] = allocation_table[n] + current[m][n];
            }
      }
      printf("\nAllocated Resources \n");
      for(m = 0; m < resource; m++) {
            printf("\t%d", allocation_table[m]);
      }
      for(m = 0; m < resource; m++) {
            available[m] = maximum_resources[m] - allocation_table[m];
      }
      printf("\nAvailable Resources:");
      for(m = 0; m < resource; m++) {
            printf("\t%d", available[m]);
      }
      printf("\n");
      while(count != 0) {
            safe_state = 0;
            for(m = 0; m < process; m++) {
                  if(running[m]) {
                        temp = 1;
                        for(n = 0; n < resource; n++) {
                              if(maximum_claim[m][n] - current[m][n] > available[n]) {
                                    temp = 0;
                                    break;
                              }
                        }
                        if(temp) {
                               printf("\nProcess %d Is In Execution \n", m + 1);
                               running[m] = 0;
                               count--;
                               safe_state = 1;
                               for(n = 0; n < resource; n++) {
                                     available[n] = available[n] + current[m][n];
                               }
                               break;
                        }
                  }
            }
            if(!safe_state) {
                  printf("\nThe Processes Are In An Unsafe State \n");
                  break;
            } 
            else {
                  printf("\nThe Process Is In A Safe State \n");
                  printf("\nAvailable Vector\n");
                  for(m = 0; m < resource; m++) 
                  {
                        printf("\t%d", available[m]);
                  }
                  printf("\n");
            }
      }
      return 0;
}
/*