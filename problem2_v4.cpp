#include <queue>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>

#define PRODUCER_STOP  0 //Producer threads stop
#define CONSUMER_STOP  0 //Consumer threads stop
#define PRODUCER_RUN   3 //Producer threads run
#define CONSUMER_RUN   4 //Consumer threads run

int push_con = PRODUCER_RUN;
int pop_con = CONSUMER_RUN;


using namespace std;

template<class DataType>
class Message_Queue
{
public:
  Message_Queue(void):msg_size(100)
  {
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_cond_init(&msg_flag,NULL);
    sem_init(&msg_queue_count,0,msg_size);
    sem_init(&msg_queue_empty,0,0);
  } 
  int push_msg(DataType &d)
  {
    /*Wait on the queue semaphore,if its value is positive,indicating the queue is not full,decrement 1*/
    /*lock the queue,push message to queue*/
    sem_wait(&msg_queue_count);
    pthread_mutex_lock(&mutex_queue);
    msg_queue.push(d);
    /*let all the threads pend know there are something new been push into the queue*/
    pthread_cond_broadcast(&msg_flag);
    sem_post(&msg_queue_empty);
    pthread_mutex_unlock(&mutex_queue);
    return 0;
  }
  int pop_msg(DataType &d)
  {	
    /*judge if there is any message,if no message,pend*/
    sem_wait(&msg_queue_empty);
    pthread_mutex_lock(&mutex_queue);
    /*if message queue is full, don't block on condition variable,just pop from the queue*/
    pthread_cond_wait(&msg_flag,&mutex_queue);
    /*when get here,there must be some message in the queue*/
    d = msg_queue.front();
    msg_queue.pop();
    /*Post to the semaphore to indicate producer thread push available*/
    sem_post(&msg_queue_count);
    pthread_mutex_unlock(&mutex_queue); 
    return 0;
  }
  /*destructor run will destroy the object referenced by pthread_mutex_t or the pthread_cond_t*/
  /*After it completes,the synthesized destructor would also run to destroy the members of the class*/
  ~Message_Queue()
  {
    int error;
    error =  pthread_mutex_destroy(&mutex_queue);
    assert(error == 0);
    error = pthread_cond_destroy(&msg_flag);
    assert(error == 0);
  }
private:
  pthread_cond_t msg_flag;
  pthread_mutex_t mutex_queue;
  queue<DataType> msg_queue;
  sem_t msg_queue_count;
  sem_t msg_queue_empty;
  unsigned int msg_size;
};

/*thread consumer*/
void *consumer(void* arg)
{
  assert(arg != NULL);
  Message_Queue<int> *queue = (Message_Queue<int> *)arg;
  int i ;
  while(1)
    {
      sleep(1);
      if(pop_con == CONSUMER_STOP)
	{	  
	  pthread_exit(NULL);
	  break;
	}
      printf("[%lu]:ready to get message\n",pthread_self());
      queue->pop_msg(i);
      printf("[%lu]:already get message = %d\n",pthread_self(),i);
    }
}

/*thread producer*/
void *producer(void* arg)
{
  assert(arg != NULL);
  Message_Queue<int> *queue = (Message_Queue<int> *)arg;
  int i ;
  while(push_con == PRODUCER_RUN) 
    {
      sleep(1);
      if(push_con == PRODUCER_STOP)
	{
	  pthread_exit(NULL);
	  break;
	}
      printf("[%lu]:ready to push_msg\n",pthread_self());
      /*push_msg return 1 so the queue is full,block*/
      queue->push_msg(i);
      printf("[%lu]:push message done\n",pthread_self());
      i++;	
    }
}

int main()
{
  Message_Queue<int> msg_queue;
  int iterator;
  /*consumer threads number & producer threads number*/
  const int number_c = 7;
  const int number_p = 21;
  pthread_t k[number_c+number_p];
  pthread_t consumer_pool[number_c]; 
  printf("create %d consumer.............\n",number_c);
  for(iterator = 0;iterator < number_c;iterator++)
    {
      pthread_create(&(k[iterator]),NULL,consumer,&msg_queue);
    }
  printf("create %d producer.............\n",number_p);
  for(iterator = 0;iterator < number_p;iterator++)
    {
      pthread_create(&(k[iterator + number_c]),NULL,producer,&msg_queue);
    }
  sleep(10);
  /*Stop producer threads first*/
  push_con = PRODUCER_STOP;
  /*Stop consumer threads*/
  pop_con = CONSUMER_STOP;
  for(iterator = 0;iterator < number_p + number_c;iterator++)
    {
      pthread_join(k[iterator],NULL);
    }  
  return 0;
}
