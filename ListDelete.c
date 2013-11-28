#include <stdio.h>

#define OK 0
#define ERROR 1

typedef struct
{
  char* element;
  int length;
}SqList;

int ListDelete(SqList *p,int position)
{
  int j = 0;
  char *elem = p->element;
  if(position < 1 || position >( p->length - 1))
    return ERROR;
  else
    {
      for(j = position;j <= (p->length - 1);j++)
	*(elem + j) = *(elem+j+1);
      --p->length;
      return OK;
    }
}

int main()
{
  int position = 3;
  SqList a = {"ajniydu",7};
  printf("The original List\n");
  printf("%s\n",a.element);
  ListDelete(&a,position);
  printf("The List has been deleted\n");
  printf("%s\n",a.element);
}
