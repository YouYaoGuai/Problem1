#include <queue>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#define CONSUMER_STOP  0 //consumer threads stop
#define PRODUCER_STOP  0 //producer threads stop
#define PRODUCER_RUN   3 //Producer threads run
#define CONSUMER_RUN   4 //Consumer threads run

int push_con = PRODUCER_RUN;
int pop_con = CONSUMER_RUN;

using namespace std;

template<class DataType>
class Message_Queue
{
public:
  Message_Queue(int d)
  {
    queue_size = d;
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_cond_init(&msg_flag,NULL);
    pthread_cond_init(&msg_avail,NULL);
  } 
  int push_msg(DataType &d)
  {
    pthread_mutex_lock(&mutex_queue);
    while((msg_queue.size()) >= queue_size)
      {
	pthread_cond_wait(&msg_avail,&mutex_queue);
      }
    msg_queue.push(d);
    pthread_cond_broadcast(&msg_flag);
    printf("%d messages in the queue\n",msg_queue.size());
    pthread_mutex_unlock(&mutex_queue);
    return 0;
  }
  int pop_msg(DataType &d)
  {
    pthread_mutex_lock(&mutex_queue);
    while((msg_queue.size()) <= 0)
      {
	pthread_cond_wait(&msg_flag,&mutex_queue);
      }
    d = msg_queue.front();
    msg_queue.pop();
    pthread_cond_broadcast(&msg_avail);
    printf("%d messages in the queue\n",msg_queue.size());
    pthread_mutex_unlock(&mutex_queue);
    return 0;
  }
  /*destructor run will destroy the object referenced by pthread_mutex_t or the pthread_cond_t*/
  /*After it completes,the synthesized destructor would also run to destroy the members of the class*/
  ~Message_Queue()
  {
    int error;
    error = pthread_cond_destroy(&msg_flag);
    assert(error == 0);
    error = pthread_cond_destroy(&msg_avail);
    assert(error == 0);
    error =  pthread_mutex_destroy(&mutex_queue);
    assert(error == 0);
  }
private:
  pthread_mutex_t mutex_queue;
  queue<DataType> msg_queue;
  pthread_cond_t msg_flag;
  pthread_cond_t msg_avail;
  unsigned int queue_size;
};

/*thread consumer*/
void *consumer(void* arg)
{
  assert(arg != NULL);
  Message_Queue<int> *queue = (Message_Queue<int> *)arg;
  int i ;
  while(pop_con == CONSUMER_RUN)
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
      queue->push_msg(i);
      printf("[%lu]:push message done\n",pthread_self());	
      i++;	
    }
}

int main()
{
  Message_Queue<int> msg_queue(100);
  int iterator;
  /*consumer threads number & producer threads number*/
  const int number_c = 7;
  const int number_p = 28;
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
#if 1
  if(number_p < number_c)
    {
      pop_con = CONSUMER_STOP;
      sleep(3);
      push_con = PRODUCER_STOP;
    }
  else
    {
      push_con = PRODUCER_STOP;
      sleep(3);
      pop_con = CONSUMER_STOP;
    }
#endif
  for(iterator = 0;iterator < number_p + number_c;iterator++)
    {
      pthread_join(k[iterator],NULL);
    }  
  return 0;
}
