#include <stdio.h>
#include "pthread.h"
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
typedef struct Philosopher{
    bool left_available,right_available;
    struct Philosopher* left_p;
    struct Philosopher* right_p;
    int total_eaten;
    int number;
}Philosopher;

void take_chopstick(Philosopher* p);
void eat(Philosopher* p);
void init_philosophers(Philosopher* p);
void print_stats(Philosopher* p);
void serve_rice();

int current_rice=0,total_served=0,total_round=0,philosopher_count;

pthread_cond_t rice_served=PTHREAD_COND_INITIALIZER;
pthread_cond_t rice_empty=PTHREAD_COND_INITIALIZER;
pthread_mutex_t rice_lock=PTHREAD_MUTEX_INITIALIZER;



int main(int argc, char* argv[]) {
    philosopher_count= atoi(argv[1]);
    //printf("%d\n",philosopher_count);
    Philosopher philosophers[philosopher_count];

    pthread_t philosopher_t[philosopher_count];
    pthread_t waiter_t;

    init_philosophers(philosophers);
    //print_stats(philosophers);

    pthread_create(&waiter_t, NULL, serve_rice, philosopher_t);

    //int x= pthread_attr_getschedpolicy();
    for(int i=0;i<philosopher_count;++i){
        pthread_create(&philosopher_t[i], NULL, eat, &philosophers[i]);
    }
    pthread_join(waiter_t,NULL);
    print_stats(philosophers);
    pthread_cond_destroy(&rice_empty);
    pthread_mutex_destroy(&rice_lock);
    pthread_cond_destroy(&rice_served);

    pthread_cond_init(&rice_served,NULL);
    pthread_cond_init(&rice_empty,NULL);
    pthread_mutex_init(&rice_lock,NULL);

    init_philosophers(philosophers);
    struct sched_param param;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_getschedparam (&attr, &param);
    param.sched_priority = sched_get_priority_max(0);
    pthread_attr_setschedparam (&attr, &param);


    pthread_create(&waiter_t, NULL, serve_rice, philosopher_t);
    pthread_create(&philosopher_t[0], &attr, eat, &philosophers[0]);
    //int x= pthread_attr_getschedpolicy();
    for(int i=1;i<philosopher_count;++i){
        pthread_create(&philosopher_t[i], NULL, eat, &philosophers[i]);
    }
    pthread_join(waiter_t,NULL);
    print_stats(philosophers);
}




void take_chopstick(Philosopher* p){
    p->left_available=false;
    p->right_available=false;
    p->left_p->right_available=false;
    p->right_p->left_available=false;
}

void leave_chopstick(Philosopher* p){
    p->left_available=true;
    p->right_available=true;
    p->left_p->right_available=true;
    p->right_p->left_available=true;
}

void print_stats(Philosopher* philosophers){
    for(int i=0;i<philosopher_count;++i){
        printf("Philosopher %d\n",philosophers[i].number);
        printf("Total Rice Eaten: %d\n",philosophers[i].total_eaten);
        printf("---------------------------\n");
    }
}
void kill_threads(pthread_t* threads){
    for(int i=0;i<philosopher_count;++i){
        pthread_cancel(threads[i]);
    }
}

void serve_rice(pthread_t* p){
    //printf("fonksiyondayim");
    srand(time(NULL));
    while(1){
        if(total_round>=1000){
            /*
            if(last_round){
                printf("\n\nTotal served: %d\n\n",total_served);
                print_stats(p);
                exit(0);
            }
             */
            printf("Total served: %d\n\n",total_served);
            total_round=0;
            current_rice=0;
            total_served=0;

            kill_threads(p);
            break;
        }
        while(current_rice > 0)
            pthread_cond_wait(&rice_empty, &rice_lock);
        pthread_mutex_lock(&rice_lock);
        current_rice+= rand() % philosopher_count + 1;
        //printf("total rice:%d\n", current_rice);
        total_served+=current_rice;
        pthread_mutex_unlock(&rice_lock);
        pthread_cond_signal(&rice_served);
        nanosleep((const struct timespec[]){{0, 1000000L}},NULL);//10msec=10000000L
        total_round++;
    }
}

void eat(Philosopher* p){
    int hungry_days=0;
    while(1){
        while(!p->left_available && !p->right_available);
        take_chopstick(p);
        while(pthread_mutex_trylock(&rice_lock));
        while(current_rice == 0)
            pthread_cond_wait(&rice_served,&rice_lock);
        current_rice--;
        if(current_rice == 0)
            pthread_cond_signal(&rice_empty);
        pthread_mutex_unlock(&rice_lock);
        leave_chopstick(p);
        p->total_eaten+=1;
        hungry_days=0;
    }
}

void init_philosophers(Philosopher* philosophers){
    for(int i=0;i<philosopher_count;++i){
        philosophers[i].left_available=true;
        philosophers[i].right_available=true;
        philosophers[i].number=i;
        philosophers[i].total_eaten=0;
        if(i+1>philosopher_count-1)
            philosophers[i].left_p=&philosophers[0];
        else
            philosophers[i].left_p=&philosophers[i+1];

        if(i-1<0)
            philosophers[i].right_p=&philosophers[philosopher_count-1];
        else
            philosophers[i].right_p=&philosophers[i-1];
    }
}