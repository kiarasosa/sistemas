#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

#define MAX_SEMAPHORES 256

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

typedef struct semaphore{
  int value;              // representa el estado del semáforo
  struct spinlock lock;   // asegura que solo un proceso modifique el valor del semáforo a la vez
} semaphore;

semaphore sem_t[MAX_SEMAPHORES];

// inicializa los semáforos
void sem_init(void){

  for(unsigned int i = 0; i < MAX_SEMAPHORES; i++){
    sem_t[i].value = -1;
    initlock(&sem_t[i].lock, "semaphore"); // inicializa el spinlock de cada semáforo
  }
}

//abre un semáforo
uint64
sys_sem_open(void){
  int sem, sem_value;

  argint(0, &sem);
  argint(1, &sem_value);

   if (sem >= MAX_SEMAPHORES || sem < 0) {
        printf("ERROR: invalid sem ID.\n");
        return 0;
    }

    char ret_value = 0;
    
    acquire(&(sem_t[sem].lock));

    if (sem_t[sem].value == -1) {
        sem_t[sem].value = sem_value;
        ret_value = 1;
    }

    release(&(sem_t[sem].lock));

    return ret_value;
}


// cierra un semáforo 
uint64
sys_sem_close(void){
  int sem;
  
  argint(0, &sem); // obtiene el valor del semáforo

  if(sem < 0 || sem >= MAX_SEMAPHORES){
    return 0; // error si el valor no es válido
  }

  acquire(&sem_t[sem].lock); // adquiere el semáforo

  sem_t[sem].value = 0; // cierra el semáforo

  wakeup(&sem_t[sem]); // despierta los procesos que estén esperando

  release(&sem_t[sem].lock); // libera el lock

  return 1;
}

// incrementa su valor. Libera un recurso
uint64
sys_sem_up(void){
  int sem;

  argint(0, &sem);

  if(sem < 0 || sem >= MAX_SEMAPHORES){
    return 0; 
  }

  acquire(&sem_t[sem].lock); // acceso exclusivo al semáforo

  sem_t[sem].value++; // incrementa el valor del semáforo
  
  wakeup(&sem_t[sem]); // despierta los procesos que estén esperando luego de haber liberado un recurso
  
  release(&sem_t[sem].lock);

  return 1;

}

// decrementa su valor. Solicita un recurso
uint64 
sys_sem_down(void){
  int sem;

  argint(0, &sem);

  if(sem < 0 || sem >= MAX_SEMAPHORES){
    return 0; 
  }

  acquire(&sem_t[sem].lock); // acceso exclusivo al semáforo

  while(sem_t[sem].value == 0){
    sleep(&sem_t[sem], &sem_t[sem].lock); // el proceso entra en estado de suspensión si no hay recursos disponibles
  } 

  sem_t[sem].value--; // decrementa el valor del semáforo

  release(&sem_t[sem].lock);

  return 1;
  
}