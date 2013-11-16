#include <queue>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define CONSUMER_STOP 0 //consumer threads stop
#define PRODUCER_STOP 0 //producer threads stop
#define PRODUCER_RUN 3 //producer threads run
#define CONSUMER_RUN 4 //consumer threads run

int push_con = PRODUCER_RUN;
int pop_con = CONSUMER_RUN;

using namespace std;

template<class DataType>
class Message_Queue
{
public:
  Message_Queue(const unsigned int &d):queue_size(d)
  {
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_cond_init(&msg_flag,NULL);
    pthread_cond_init(&msg_avail,NULL);
  }
  void push_msg(DataType &d)
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
  }
  void pop_msg(DataType &d)
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
  const unsigned int queue_size;
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
      printf("[%lu]:ready to get message\n",pthread_self());
      queue->pop_msg(i);
      /*-1 is stop message*/
      if(i == -1)
	{
	  pthread_exit(NULL);
	}
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
  int ret;
  pthread_mutex_t thread_flag;
  pthread_mutex_init(&thread_flag,NULL);

  /*consumer threads number & producer threads number*/
  const int number_c = 28;
  const int number_p = 7;
  int stop_msg = -1;
  pthread_t k[number_c + number_p];
  printf("create %d consumer.............\n",number_c);
  for(iterator = 0;iterator < number_c;iterator++)
    {
      ret =  pthread_create(&(k[iterator]),NULL,consumer,&msg_queue);
      assert(ret == 0);
    }
  printf("create %d producer.............\n",number_p);
  for(iterator = 0;iterator < number_p;iterator++)
    {
      ret = pthread_create(&(k[iterator + number_c]),NULL,producer,&msg_queue);
      assert(ret == 0);
    }
  sleep(10);
  pthread_mutex_lock(&thread_flag);
  push_con = PRODUCER_STOP;
  pthread_mutex_unlock(&thread_flag);
  /*producer join*/
  for(iterator = 0;iterator < number_p ;iterator++)
    {
       pthread_join(k[iterator + number_c],NULL);
    }
  printf("Send Stop Mesage ***********************************************\n");
  printf("When the program blocked,press any key to quit\n");
  scanf("%d",&ret);
  /*send stop message*/
  for(iterator = number_p; iterator < number_p + number_c;iterator++)
    {
      msg_queue.push_msg(stop_msg);
      printf("push stop message\n");
    }
  for(iterator = number_p;iterator < number_p + number_c;iterator++)
    {
       pthread_join(k[iterator],NULL);
    }  
  pthread_mutex_destroy(&thread_flag);
  return 0;
}
