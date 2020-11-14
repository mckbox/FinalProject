#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>

//this variable contains the coaches 1 means a coach is avalible (on his phone) 0 means he is busy with customer
unsigned int *coaches;
int c_size;

//this variable contains customers in the waiting room. 0 means the seat is open -2 means a customer is in the seat
unsigned int *waiting_room;
int w_size;

/** The purpse of this function is to initilize the array of coache s (or recorces)
 *  @param num_coaches the number of coaches to initialize the array with
 *  @return 0 on success -1 otherwise
 */
int ini_coaches(int num_coaches){

    //initialize the size of the coaches array
    c_size = num_coaches;
    coaches = malloc(sizeof(unsigned int) * c_size);

    //set all the values of the coaches to 1 (aka free)
    for(int i = 0; i < c_size; i++){
        coaches[i] = 1;
    }
}

/** The purpose of this function is to 
 * 
 */
int customers(){

    int trainer = trainer_avalible();

    //checking to see if trainers are avalible
    if(trainer == -1){

        //if no trainer avilible walk back to the waiting room
        sleep(0.1);
        int cust_wait = add_cust();

        //if the waiting room is full
        if(cust_wait == -1){
            //leave the gym
            exit(0);
        }

        //waiting for a trainer to come get them
        while(waiting_room[cust_wait] == -2);
    }
    
    //once the trainer gets the customer

    //train the customer
    sleep(1);




}


int main(){
    ini_coaches(8);

    for(int i = 0; i < c_size; i++){
        printf("%d\n",coaches[i]);
    }
}
