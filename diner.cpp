#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <unistd.h>

using namespace std;


sem_t printing;     // whether or not its safe to print
sem_t tables;       // The amount of free tables
sem_t get_customer; // make sure that you can get a customer cleanly
sem_t waiter1;
sem_t waiter2;

sem_t waiter1_busy;
sem_t waiter2_busy;

// the Queue.
// Guests are added to this when created
queue<int> order;

// when this value turns to 1, a waiter is interacting
// with the customer with that id
// 5 isnt 1 or 0 so I am using it.
// could be writen better
int giveing_menu[9] = {5,5,5,5,5,5,5,5,5};

void safe_print(const std::string& input, int i, int j)
{
    sem_wait(&printing);
    cout  << "#" << i << "," << j << " "  << input  << endl;
	sem_post(&printing);
}

// Customer
void *customer(void *x)
{
    int n = *((int *) x);

    safe_print("I am customer # ", n,999);


    while (giveing_menu[n] == 0) 
    {
        //sleep(1);
    }
    
    sem_wait(&tables);
    safe_print("this person sat at a table ", n,999);
    safe_print("I am ready to order", n,999);


    int w_id = giveing_menu[n];
    while(w_id == 5)
    {
        w_id = giveing_menu[n];
    }

    // sink up the guest and the waiter
    if (n == 1){ sem_post(&waiter1); }
    else if(n == 0){ sem_post(&waiter2); }


    sem_wait(&printing);
    cout  << "####" << n << " "  << w_id << " WE ARE PARED UP" << endl;
	sem_post(&printing);


    safe_print("I am paired with this waiter ",n, w_id);
    safe_print("I order food: ", n,w_id);
    

    safe_print(" I eat ",n,w_id);

    safe_print(" I AM LEAVING", n,w_id);

    //post
    if (w_id == 1){ sem_post(&waiter1_busy); }
    else if(w_id == 0){ sem_post(&waiter2_busy); }


    safe_print("I pay the bill",n,w_id);

    if (w_id == 1){ sem_wait(&waiter1_busy); }
    else if(w_id == 0){ sem_wait(&waiter2_busy); }
    sem_post(&tables);


    //post
    safe_print("Well, Its time to leave I suppose", n,w_id);
    if (w_id == 1){ sem_post(&waiter1_busy); }
    else if(w_id == 0){ sem_post(&waiter2_busy); }

    pthread_exit(NULL);

}
// Waiter
void *waiter(void *x)
{

    int n = *((int *) x);
    int customer_id;
    safe_print("I am waiter # ", 999,n);

    while (order.size() > 0)
    {

        // give a menu to customer 
        sem_wait(&get_customer);
        safe_print("this person is going to be next ", customer_id,n);

        order.pop();
        safe_print("This many people are still waiting", order.size(), order.size());
        customer_id = order.front();
        safe_print("I am going to give a menu to this customer ", customer_id,n);
        sem_post(&get_customer);

        // set the giving_id to your id
        giveing_menu[customer_id] = n;

        // sink up the guest and the waiter
        if (n == 1){ sem_wait(&waiter1); }
        else{ sem_wait(&waiter2); }

        //wait
        if (n == 1){ sem_wait(&waiter1_busy); }
        else if(n == 0){ sem_wait(&waiter2_busy); }

      
        safe_print("Here is the bill sir/madam",customer_id, n);
        //post
        if (n == 1){ sem_post(&waiter1_busy); }
        else if(n == 0){ sem_post(&waiter2_busy); }

        //wait
        if (n == 1){ sem_wait(&waiter1_busy); }
        else if(n == 0){ sem_wait(&waiter2_busy); }

        safe_print("Here is your reciept", customer_id,n);

        if (n == 1){ sem_post(&waiter1_busy); }
        else if(n == 0){ sem_post(&waiter2_busy); }

    }
    safe_print("There are no more customers",customer_id,n);
    pthread_exit(NULL);
    safe_print("THIS SHOULDNT HAPPEN",n,n);
}

int main(int argc, char *argv[])
{
    
    sem_init(&printing, 0, 1);
    sem_init(&tables, 0, 4);
    sem_init(&get_customer, 0, 1);
    sem_init(&waiter1, 0, 0);
    sem_init(&waiter2, 0, 0);
    sem_init(&waiter1_busy, 0, 0);
    sem_init(&waiter2_busy, 0, 0);

    


    int rc,w,i;

    // Create guest threads
    pthread_t threads[9];
    for (i=0; i<10; ++i)
    {
        order.push(i);
        rc = pthread_create(&threads[i], NULL, customer, new int(i));

    }

    // create waiters
    pthread_t wthreads[2];
    for (i=0; i<2; ++i)
    {
        w = pthread_create(&wthreads[i], NULL, waiter, new int(i));

    }

    sleep(10);
    //clean up
    for (i=0; i<9; ++i) {
        rc = pthread_join(threads[i], NULL);
    }
    for (i=0; i<2; ++i) {
        w = pthread_join(wthreads[i], NULL);
    }   
    cout << "finishing" << endl;
    return 0;
}

