#include <deque>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

using namespace std;

template<class DataType>
class Message_Queue
{
public:
  /*msg_stop = 1,cancel*/
  int stop_msg(DataType &d)
  {
    msg_stop = (int)d;
    if(msg_stop == 1)
      {
	printf("Msg queue stopped");
      }
    pthread_cancel(pthread_self());
  }
  Message_Queue():msg_ready(0)
  {
    pthread_mutex_init(&mutex_queue,NULL);
    pthread_mutex_init(&mutex_message,NULL);
    pthread_cond_init(&msg_flag,NULL);
  }
  
  int push_msg(DataType &d)
  {
    /*lock the queue,push message to queue*/
    pthread_mutex_lock(&mutex_queue);
    queue.push_back(d);
    pthread_mutex_unlock(&mutex_queue);
    
    /*judge if it is necessary to wake up consumer*/
    pthread_mutex_lock(&mutex_message);
    if(msg_ready != 0)
      pthread_cond_signal(&msg_flag);

    /*number of messages in the queue*/
    msg_ready++;
    pthread_mutex_unlock(&mutex_message);
    
    return 0;
  }
  int pop_msg(DataType &d)
  {
    /*judge if there is no message,if no message,pend*/
    pthread_mutex_lock(&mutex_message);
    while(msg_ready == 0)
      pthread_cond_wait(&msg_flag,&mutex_message);
    
    /*when get here,there must be some message in the queue*/
    msg_ready--;
    pthread_mutex_unlock(&mutex_message);

    /*pop the message from the queue*/
    pthread_mutex_lock(&mutex_queue);
    d = queue.front();
    queue.pop_front();
    pthread_mutex_unlock(&mutex_queue);
    
    return 0;
  }

  
private:
  pthread_cond_t msg_flag;
  pthread_mutex_t mutex_queue;
  pthread_mutex_t mutex_message;
  deque<DataType> queue;
  int msg_ready;
  int msg_stop;
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
  while(1)
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
  iterator = 1;
  msg_queue.stop_msg(iterator);
  // for(iterator = 0;iterator < number_p + number_c;iterator++)
  // {
  //    pthread_join(k[iterator],NULL);
  //  }
  
}
