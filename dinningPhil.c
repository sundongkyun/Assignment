#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdint.h>

#define NUMBER_OF_PHIL 5
#define EXEC_TIME 3

enum PHIL_STATE
{
    HUNGRY = 0,
    EATING = 1,
    THINKING = 2
};

typedef struct philosopher
{
    unsigned short num_eat;
    enum PHIL_STATE state;
}philosopher;

philosopher phil[NUMBER_OF_PHIL];
sem_t chopstick[NUMBER_OF_PHIL];

sem_t lock;

//10~100 msec random wait
int idlewait()
{
    int sleepTimeMS = (rand() % 91 + 10);
    usleep(sleepTimeMS * 1000);
    return sleepTimeMS;
}

unsigned int tick() //get current time(msec)
{
    struct timeval tv;
    gettimeofday(&tv, (void*)0);
    return tv.tv_sec * (unsigned int)1000 + tv.tv_usec / 1000;
}
/* Philosopher & Chopstick initialize */
void initPhil(void)
{
    unsigned short i;
    for(i=0; i<NUMBER_OF_PHIL; i++)
    {
        phil[i].num_eat = 0;
        phil[i].state = THINKING;
        sem_init(&chopstick[i], 0,1);
    }

}

/*Vonvurrent pick left & right */
void* dining(void* arg)
{
    int t = EXEC_TIME;
    unsigned short left, right;
    unsigned int start_time;
    unsigned short phil_i = (int)(intptr_t)arg;
    philosopher* currentphil = &phil[phil_i];
    left = phil_i;
    right = (phil_i +1)% NUMBER_OF_PHIL;
    //Loop during EXEC_TIME

    while(t>0)
    {
        start_time = tick();
        idlewait();

        currentphil->state = HUNGRY;
        printf("phil[%d}: hungry\n",phil_i);

        sem_wait(&lock);

        sem_wait(&chopstick[left]);
        sem_wait(&chopstick[right]);

        currentphil->state = EATING;
        printf("phil[%d]: eating\n",phil_i);
        currentphil->num_eat++;

        idlewait();

        sem_post(&chopstick[left]);
        sem_post(&chopstick[right]);
        sem_post(&lock);

        currentphil->state = THINKING;
        printf("phil[%d]: thinking\n",phil_i);

        t = t - (tick()/1000 - start_time/1000);
    }

}

void main(void)
{
    pthread_t t[NUMBER_OF_PHIL];
    unsigned short i, args[NUMBER_OF_PHIL], minCount = USHRT_MAX, maxCount = 0;
    long long start = 0, end = 0; //start time, end time
    void* thread_result;
    srand(time(NULL));

    initPhil();
    start = tick();

    sem_init(&lock, 0, NUMBER_OF_PHIL -1);

    for(i=0; i<NUMBER_OF_PHIL; i++)
    {
        args[i] = i;
        pthread_create(&t[i], NULL, dining, (void*)(intptr_t)args[i]);
    }
    for(i=0; i<NUMBER_OF_PHIL; i++)
    {
        pthread_join(t[i],&thread_result);
    }

    end = tick();

    for(i=0; i<NUMBER_OF_PHIL; i++)
    {    sem_destroy(&chopstick[i]);
    }
    sem_destroy(&lock);


    //Display Answer
    printf("\n\n");
    for(i=0;i<NUMBER_OF_PHIL;i++)
    {
        printf("Philosopher %d eating count : %d\n\n", i, phil[i].num_eat);
    }

    printf("Total Exec Time : %lld.%lld sec\n\n", (end-start) / 1000, (end-start) % 1000);
}
