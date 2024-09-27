#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/sysproc.c"

void ping(int sem, int max){
    for(int i = 0; i < max; i++){
        sem_down(sem);  // espera a que el semáforo esté desbloqueado
        printf("Ping\n");
        sem_up(sem);    // libera el semáforo para que "pong" pueda ser ejecutado
        } 
    sem_close(sem);
}

void pong(int sem, int max){
    for(int i = 0; i < max; i++){
        sem_down(sem); 
        printf("Pong\n");
        sem_up(sem); 
        } 
    sem_close(sem);
}


int main(int argc, char *argv[]){

    if(argc <= 1){
        printf("Failed, not enough arguments\n");
        exit(1);
    }
    
    int ping_sem = sem_open(0, 1);   // inicializa el semáforo del padre desbloqueado
    int pong_sem = sem_open(1, 0);   // inicializa el semáforo del hijo bloqueado

    int pid = fork(); 

        if(pid == 0){
            ping(ping_sem, atoi(argv[1]));
        } else {
            pong(pong_sem, atoi(argv[1]));
        }

        printf("&d&d\n", ping_sem, pong_sem);
        
    exit(0);
}


