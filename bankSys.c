#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_ACCS 2
#define NUMS_TRANSACTIONS 10

//"bank accounts" info
typedef struct {
    int userID;
    double balance;
    pthread_mutex_t lock;
}

Account;

Account accs[NUM_ACCS]; //stores bank accounts

//deposit function of the "bank"

void* transaction(void *arg) {
    double amount = (rand() % 100) + 1;
    int acc_id = *(int *)arg; //takes account ID from arguement
    for(int i = 0; i < NUMS_TRANSACTIONS; i++) {
        pthread_mutex_lock(&accs[acc_id].lock); //locks acc mutex
        accs[acc_id].balance += amount; //adds money into acc
        printf("Account %d: Deposited %f. The balance is %.2f\n", acc_id, amount , accs[acc_id].balance);
        pthread_mutex_unlock(&accs[acc_id].lock); //unlocks acc mutex
        usleep(500000); //simulates real time
    }
    return NULL;
}

// function for money transfer between accs

void *transfer(void *arg) {
    int *ids = (int *)arg;
    int from = ids[0], to = ids[1]; //get start and end accs for transfers

    for (int i = 0; i < NUMS_TRANSACTIONS; i++) {
        double amount = (rand() % 100) + 1; //generates random amount for each acc to send
        if(from < to) {
            pthread_mutex_lock(&accs[from].lock);
            pthread_mutex_lock(&accs[to].lock);
        }
        else {
            pthread_mutex_lock(&accs[to].lock);
            pthread_mutex_lock(&accs[from].lock);
        }

        if (accs[from].balance >= amount) { //makes sure balance is valid
            accs[from].balance -= amount;
            accs[to].balance += amount;
            printf("An amount of %f from account %i has been sent to account %i.\n", amount, from, to);
        }

        pthread_mutex_unlock(&accs[from].lock); //unlocks reciever acc
        pthread_mutex_unlock(&accs[to].lock); //unlocks sender acc
        usleep(500000);
    }
    return NULL;
}

int main() {
    pthread_t threads[4];
    int ids[NUM_ACCS] = {0 , 1};
    int transfer_ids[2] = {0 , 1};
    int transfer_ids_reverse[2] = {1, 0};

    //init accs
    for(int i = 0; i < NUM_ACCS; i++) {
        accs[i].userID = i;
        accs[i].balance = 1000.0; //load account with $1000
        pthread_mutex_init(&accs[i].lock, NULL); //init mutex lock
    }

    //creates threads for transfers / transactions
    pthread_create(&threads[0], NULL, transaction, &ids[0]);
    pthread_create(&threads[1], NULL, transaction, &ids[1]);
    pthread_create(&threads[2], NULL, transfer, transfer_ids);
    pthread_create(&threads[3], NULL, transfer, transfer_ids_reverse);
    

    //waiting for threads to complete
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    //ends mutex locks
    for(int i = 0; i < NUM_ACCS; i++) {
        pthread_mutex_destroy(&accs[i].lock);
    }

    //display final balances
    printf("Final Balances: Acc 0: %.2f, Acc 1: %.2f\n", accs[0].balance, accs[1].balance);
    return 0;
}
