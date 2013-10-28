#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

union semun{
  int val;
  struct semid_ds *buf;
  unsigned short int *array;
  struct seminfo *__buf;
};
union semun sem_write,sem_read;

/*Initialize semaphore with value 1*/
int semaphore_initialize(int semid,int sem_value,union semun sem)
{
   if(sem_value != 0 && sem_value != 1)
   {
      printf("You need correct initial semaphore value\n");
      exit(1);
    }
  unsigned short values[2];
  values[0] = sem_value;
  // values[1] = 0;
  sem.array = values;
  return semctl(semid,0,SETALL,sem);
}

/*Deallocate a semaphore*/
int semaphore_deallocate(int semid,union semun sem)
{
  return semctl(semid,1,IPC_RMID,sem);
}

/*Wait on a semaphore*/
int semaphore_wait(int semid)
{
  struct sembuf operations[1];
  operations[0].sem_num = 0;
  operations[0].sem_op = -1;
  operations[0].sem_flg = SEM_UNDO;
  return semop(semid,operations,1);
}

/*Post a binary semaphore*/
int semaphore_post(int semid)
{
  struct sembuf operations[1];
  operations[0].sem_num = 0;
  operations[0].sem_op = 1;
  operations[0].sem_flg = SEM_UNDO;

  return semop(semid,operations,1);
}



int main(int argc,char** argv)
{
  pid_t pid;
  int segment_id;
  const int shared_segment_size = 1024;
  char* write_addr;
  char* read_addr;
  char buffer[1024];
  int sem_write_finish,sem_read_finish;

  /*Share memory get*/
  segment_id = shmget(IPC_PRIVATE, shared_segment_size, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  /*Process semaphore*/
  sem_write_finish = semget(1000,1,0666|IPC_CREAT);
  sem_read_finish = semget(1100,1,0666|IPC_CREAT);
  semaphore_initialize(sem_write_finish,0,sem_write);
  semaphore_initialize(sem_read_finish,1,sem_read);
  
  if(segment_id == -1)
    {
      printf("Share memory create error\n");
      exit(1);
    }

  pid = fork();
  if(pid != 0)
    {
      write_addr = (char* )shmat(segment_id,0,0);
      while(1)
	{
	  semaphore_wait(sem_read_finish);
	  memset(write_addr,'\0',1024);
	  printf("Type one line.Maybe you wanner quit,press q\n");
	  fgets(buffer,sizeof(buffer),stdin);
	  if(strncmp(buffer,"q",1)==0)
	    {
	      semaphore_deallocate(sem_write_finish,sem_write);
	      semaphore_deallocate(sem_read_finish,sem_read);
	      shmdt(segment_id);
	      shmctl(segment_id,IPC_RMID,0);
	      //wait(NULL);
	      printf("Resource relised\n");
	      kill(0,SIGKILL);
	    }
	  strncpy(write_addr,buffer,1024);
	  semaphore_post(sem_write_finish);
	}
    }
  else
    {
      // sleep(1);
      read_addr = (char* )shmat(segment_id,0,0);
      while(1)
	{
	  semaphore_wait(sem_write_finish);
	  printf("%s\n",read_addr);
	  semaphore_post(sem_read_finish);
	}
    }
  
}
