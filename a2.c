#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "a2_helper.h"


pthread_mutex_t mutexP4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condStart = PTHREAD_COND_INITIALIZER;
pthread_cond_t condEnd = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutexP6 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condP6 = PTHREAD_COND_INITIALIZER;

int p6Count = 0;

void *thread_function(void *arg) {
    int *infoPair = (int *)arg;

    if(infoPair[0] == 4 && infoPair[1] == 1){
        sem_t* P8T6 = sem_open("P8T6", O_CREAT, 0644, 0);
        sem_wait(P8T6);
        sem_close(P8T6);
    }

    if(infoPair[0] == 8 && infoPair[1] == 4){
        sem_t* P8T4 = sem_open("P8T4", O_CREAT, 0644, 0);
        sem_wait(P8T4);
        sem_close(P8T4);
    }

    if (infoPair[0] == 4 && infoPair[1] == 3)
        pthread_cond_wait(&condStart, &mutexP4);

    if (infoPair[0] == 6) {
        pthread_mutex_lock(&mutexP6);
        while (p6Count == 5)
            pthread_cond_wait(&condP6, &mutexP6);
        p6Count++;
        pthread_mutex_unlock(&mutexP6);
    }

    info(BEGIN, infoPair[0], infoPair[1]);

    if (infoPair[0] == 4 && infoPair[1] == 4) {
        pthread_mutex_lock(&mutexP4);
        pthread_cond_signal(&condStart);
        pthread_cond_wait(&condEnd, &mutexP4);
    }

    info(END, infoPair[0], infoPair[1]);

    if (infoPair[0] == 6) {
        pthread_mutex_lock(&mutexP6);
        p6Count--;
        pthread_cond_signal(&condP6);
        pthread_mutex_unlock(&mutexP6);
    }

    if (infoPair[0] == 4 && infoPair[1] == 3) {
        pthread_cond_signal(&condEnd);
        pthread_mutex_unlock(&mutexP4);
    }

    if(infoPair[0] == 8 && infoPair[1] == 6){
        sem_t* P8T6 = sem_open("P8T6", O_CREAT, 0644, 0);
        sem_post(P8T6);
        sem_close(P8T6);
    }

    if(infoPair[0] == 4 && infoPair[1] == 1){
        sem_t* P8T4 = sem_open("P8T4", O_CREAT, 0644, 0);
        sem_post(P8T4);
        sem_close(P8T4);
    }

    return NULL;
}
int main() {

    init();
    info(BEGIN, 1, 0);

    // Process 2
    pid_t pid2 = fork();
    if (pid2 == 0) {
        info(BEGIN, 2, 0);
        // Process 7
        pid_t pid7 = fork();
        if (pid7 == 0) {
            info(BEGIN, 7, 0);
            info(END, 7, 0);
            return 0;
        }
        // Process 8
        pid_t pid8 = fork();
        if (pid8 == 0) {
            info(BEGIN, 8, 0);
            pthread_t p8Threads[6];
            for (int i = 0; i < 6; i ++) {
                int *infoPair8 = malloc(2 * sizeof(int));
        	infoPair8[0] = 8;
        	infoPair8[1] = i + 1;
        	pthread_create(&p8Threads[i], NULL, thread_function, (void *)infoPair8);
            }
            for (int i = 0; i < 6; i ++)
                pthread_join(p8Threads[i], NULL);
            info(END, 8, 0);
            return 0;
        }
        waitpid(pid7, NULL, 0);
        waitpid(pid8, NULL, 0);
        info(END, 2, 0);
        return 0;
    }

    // Process 3
    pid_t pid3 = fork();
    if (pid3 == 0) {
        info(BEGIN, 3, 0);
        // Process 4
        pid_t pid4 = fork();
        if (pid4 == 0) {
            info(BEGIN, 4, 0);
            pthread_t p4Threads[5];
            for (int i = 0; i < 5; i ++) {
                int *infoPair4 = malloc(2 * sizeof(int));
                infoPair4[0] = 4;
        	infoPair4[1] = i + 1;
        	pthread_create(&p4Threads[i], NULL, thread_function, (void *)infoPair4);
            }
            for (int i = 0; i < 5; i ++)
                pthread_join(p4Threads[i], NULL);
            pthread_mutex_destroy(&mutexP4);
            pthread_cond_destroy(&condStart);
            pthread_cond_destroy(&condEnd);
            info(END, 4, 0);
            return 0;
        }
        // Process 5
        pid_t pid5 = fork();
        if (pid5 == 0) {
            info(BEGIN, 5, 0);
            // Process 6
            pid_t pid6 = fork();
            if (pid6 == 0) {
                info(BEGIN, 6, 0);
                pthread_t p6Threads[48];
                for (int i = 0; i < 48; i ++) {
                    int *infoPair6 = malloc(2 * sizeof(int));
        	    infoPair6[0] = 6;
        	    infoPair6[1] = i + 1;
        	    pthread_create(&p6Threads[i], NULL, thread_function, (void *)infoPair6);
                }
                for (int i = 0; i < 48; i ++)
                    pthread_join(p6Threads[i], NULL);
                    
                pthread_mutex_destroy(&mutexP6);
                pthread_cond_destroy(&condP6);
                
                info(END, 6, 0);
                return 0;
            }
            // Process 9
            pid_t pid9 = fork();
            if (pid9 == 0) {
                info(BEGIN, 9, 0);
                info(END, 9, 0);
                return 0;
            }
            waitpid(pid6, NULL, 0);
            waitpid(pid9, NULL, 0);
            info(END, 5, 0);
            return 0;
        }
        waitpid(pid4, NULL, 0);
        waitpid(pid5, NULL, 0);
        info(END, 3, 0);
        return 0;
    }

    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);

    info(END, 1, 0);


    sem_unlink("P8T6");
    sem_unlink("P8T4");
    // done
    return 0;
}

