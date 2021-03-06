#include <queue>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

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
  Message_Queue()
  {
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_cond_init(&msg_flag,NULL);
  }
  int number_msg(void)
  {
    return msg_queue.size();
  }
  int clear_msg(void)
  {
    while(!msg_queue.empty())
      {
	msg_queue.pop();
      }
  }
  int push_msg(DataType &d)
  {
    /*lock the queue,push message to queue*/
    pthread_mutex_lock(&mutex_queue);
    msg_queue.push(d);
    /*let all the threads pend know there are something new been push into the queue*/
    pthread_cond_broadcast(&msg_flag);
    pthread_mutex_unlock(&mutex_queue);
    return 0;
  }
  int pop_msg(DataType &d)
  {
    /*judge if there is any message,if no message,pend*/
    pthread_mutex_lock(&mutex_queue);
    pthread_cond_wait(&msg_flag,&mutex_queue);
    /*when get here,there must be some message in the queue*/
    d = msg_queue.front();
    msg_queue.pop();
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
  const int number_p = 11;
  pthread_t k[number_c+number_p];
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
  printf("%d message in the queue*******************************\n",msg_queue.number_msg());
  /*Clear Msg Queue*/
  msg_queue.clear_msg();
  /*Stop consumer threads*/
  pop_con = CONSUMER_STOP;
  printf("%d message in the queue*******************************\n",msg_queue.number_msg());

  for(iterator = 0;iterator < number_p + number_c;iterator++)
  {
      pthread_join(k[iterator],NULL);
  }  
  return 0;
}
