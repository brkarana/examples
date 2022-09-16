#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<assert.h>
#include<unistd.h>
#include<signal.h>

typedef struct Lock {
pthread_t owner;
} Lock;

volatile Lock *lk;

// Global Variables
int var = 0;
int flag = 0;

pthread_t unlocked = 0;

__thread pthread_t tid = 0;

int debug = 0;

#define DEBUG_STMNT(x, y)  if(debug) printf(x,y);

void spinLockAcquire() {
   int locked;
   int nCount;
   while (1) {
      locked = __atomic_compare_exchange_n(&lk->owner, &unlocked, tid, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
      if (locked)  {
         DEBUG_STMNT("%ld acquired lock\n", tid); 
         break;
      } else {
         __atomic_store_n(&unlocked, 0, __ATOMIC_SEQ_CST);
         nCount++;
        if(nCount % 60 == 0) sleep(1);
      }
   }
   assert(lk->owner == tid);
}

void spinLockRelease() {
   assert(lk->owner == tid);
   __atomic_store(&lk->owner, &unlocked, __ATOMIC_SEQ_CST);
   DEBUG_STMNT("%ld released the lock\n", tid);
}

void* thread1_func(void *arg) {
     tid = pthread_self();
     spinLockAcquire();
     var =  1;
     if (flag == 0) {
        printf("Hello\n");
        fflush(stdout);
     } 
     spinLockRelease();
}

void* thread2_func(void *arg) {
    tid = pthread_self();
    spinLockAcquire();
    flag = 1;
    if (var == 0) {
       printf("World\n");
       fflush(stdout);
    }
    spinLockRelease();
}
    
int main(){
   pthread_t tid1, tid2;
   lk = (volatile Lock*) calloc(1, sizeof(Lock));
   pthread_create(&tid1, NULL, &thread1_func, NULL);
   pthread_create(&tid2, NULL, &thread2_func, NULL);
   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);
}
   

