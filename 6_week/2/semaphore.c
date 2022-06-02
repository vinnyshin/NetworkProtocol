#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

void * read(void *arg);
void * accu(void *arg);

sem_t* sem_one;
sem_t* sem_two;
int num;

int main(int argc, char const *argv[])
{
    pthread_t t1, t2;
    
    sem_one = sem_open("sem_one", O_CREAT | O_EXCL, 0644, 0);
    sem_two = sem_open("sem_two", O_CREAT | O_EXCL, 0644, 1);

    // int value;
    // sem_getvalue(sem_one, &value);
    // printf("sem1: %d\n", value);
    // sem_getvalue(sem_two, &value);
    // printf("sem2: %d\n", value);


    pthread_create(&t1, NULL, read, NULL);
    pthread_create(&t2, NULL, accu, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_close(sem_one);
    sem_close(sem_two);

    sem_unlink("sem_one");
    sem_unlink("sem_two");

    return 0;
}

void * read(void * arg) {
    for(int i = 0; i < 5; i++) {
        sem_wait(sem_two); // sem_two semaphore 감소 0 이면 wait
        fputs("input num:", stdout);
        scanf("%d", &num);
        sem_post(sem_one); // sem_one semaphore 1 증가
    }
    return NULL;
}

void * accu(void * arg) {
    int sum = 0;

    for (int i = 0; i < 5; i++)
    {
        sem_wait(sem_one); // sem_one 1 감소
        sum += num;
        sem_post(sem_two); // sem_two 1 증가
    }

    printf("result: %d \n", sum);
    return NULL;
}