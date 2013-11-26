#include "stdio.h"
#include "assert.h"
#include "string.h"

#define OK 0
#define ERROR 1
#define OVER_FLOW 2
#define MAX 100

int ListInsert(char *List,int position,char data)
{
  int i,last;
  if(position <1 || position >= MAX)
    return ERROR;
  if(strlen(List) >= MAX)
    return OVER_FLOW;
  else
    {
      for(i = 0 ;i < MAX;i++)
	{
	  if(*(List + i) == 0)
	    {
	      last = i;
	      break;
	    }
	}
      for(i = last; i >= position -1;--i) //move all the element behind position to the next position
	{
	  *(List + i + 1) = *(List + i); 
	}
      *(List + position - 1) = data;
      return OK;	
    }
}

int main()
{
  int i = 0;
  char Sq_list[MAX] = "abcdefg";
  char data;
  printf("The content of the list\n");
  for(i = 0;(i < MAX && Sq_list[i] != 0); i++)
    {
      printf("%c\t",Sq_list[i]);
      if((i+1)%8 == 0)
	printf("\n");
    }
  printf("\n");
  unsigned int position = 0;
  printf("Put in the position you wanner to insert!\n");
  scanf("%d,",&position);
  printf("Put in the data you wanner insert\n");
  scanf("%c",&data);
  ListInsert(Sq_list,position,data);
  printf("The insert result:\n");
  for(i = 0;(i < MAX && Sq_list[i] != 0); i++)
    {
      printf("%c\t",Sq_list[i]);
      if((i+1)%8 == 0)
	printf("\n");
    }
}
