#include <queue>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#define THREAD_RUN  1 //Run the threads
#define THREAD_STOP 0 //Stop the threads

int STATUS = THREAD_RUN;

using namespace std;

template<class DataType>

class Message_Queue
{
public:
  Message_Queue():msg_ready(0)
  {
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_cond_init(&msg_flag,NULL);
  }
  int msg_queue_size(void)
  {
    printf("%d messags in the queue\n",msg_ready);
    return msg_ready;
  }
  int push_msg(DataType &d)
  {
    /*lock the queue,push message to queue*/
    pthread_mutex_lock(&mutex_queue);
    msg_queue.push(d);
    /*let all the threads pend know there are something new been push into the queue*/
    pthread_cond_broadcast(&msg_flag);
    /*number of messages in the queue now*/
    msg_ready++;
    pthread_mutex_unlock(&mutex_queue);
        
    return 0;
  }
  int pop_msg(DataType &d)
  {
    /*judge if there is any message,if no message,pend*/
    pthread_mutex_lock(&mutex_queue);
    while(msg_ready == 0)
      pthread_cond_wait(&msg_flag,&mutex_queue);
    /*when get here,there must be some message in the queue*/
    d = msg_queue.front();
    msg_queue.pop();
    msg_ready--;
    pthread_mutex_unlock(&mutex_queue);
    
    return 0;
  }

  
private:
  pthread_cond_t msg_flag;
  pthread_mutex_t mutex_queue;
  queue<DataType> msg_queue;
  int msg_ready;
};

/*thread consumer*/
void *consumer(void* arg)
{
  assert(arg != NULL);
  Message_Queue<int> *queue = (Message_Queue<int> *)arg;
  int i ;
  while(STATUS == THREAD_RUN)
    {
      sleep(1);
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
  while(STATUS == THREAD_RUN)
    {
      sleep(1);
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
  const int number_c = 8;
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
      pthread_create(&(k[iterator+8]),NULL,producer,&msg_queue);
    }

  sleep(50);
  /*stop consumer&producer threads*/
  STATUS = THREAD_STOP;
 
  for(iterator = 0;iterator < number_p + number_c;iterator++)
  {
      pthread_join(k[iterator],NULL);
  }  
}
