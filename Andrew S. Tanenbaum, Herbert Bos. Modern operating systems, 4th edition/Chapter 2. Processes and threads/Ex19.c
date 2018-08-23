#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUMBER_OF_THREADS 10

enum states{PRINTING, EXITING};
enum states state[NUMBER_OF_THREADS] = {0};

void *print_hello_world(void *tid)
{
        int i = 0;
        while(i < (int)tid)
                if(state[i] == PRINTING)
                        pthread_yield();
                else
                        i++;

        printf("Привет, мир. Тебя приветствует поток № %d\n", tid);

        state[(int)tid] = EXITING;
        pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
        pthread_t threads[NUMBER_OF_THREADS];
        int status, i;
        for(i = 0; i < NUMBER_OF_THREADS; i++){
                printf("Это основная программа. Создание потока № %d\n", i);
                status = pthread_create(&threads[i], NULL, print_hello_world,
                        (void*)i);
                if(status != 0){
                        printf("Жаль, функция pthread_create вернула код ошибки %d\n",
                                status);
                        exit(-1);
                }
                while(state[i] != EXITING)
                        pthread_yield();
        }
        exit(NULL);
}
