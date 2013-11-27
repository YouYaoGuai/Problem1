#include <stdio.h>
#include <string.h>

#define MAX 100

typedef struct
{
  char list[MAX];
  int last;
}sq_list;

void MergeList(sq_list a, sq_list b,sq_list *c)
{
  int i = 0;
  int j = 0;
  int k = 0;
  while(i <= (a.last - 1) && j <= (b.last-1))
    {
      if(a.list[i] <= b.list[j])
	c->list[k++] = a.list[i++];
      else
	c->list[k++] = b.list[j++];
    }
  while(i <= (a.last - 1))
    {
      c->list[k++] = a.list[i++];
    }
  while(j <= (b.last - 1))
    {
      c->list[k++] = b.list[j++];
    }
  c->list[k] = 0;
  c->last = k;
}

int main()
{
  sq_list Sq_list1 = {"aghkosu",7};
  sq_list Sq_list2 = {"cfklns",6};
  sq_list Sq_list3;

  printf("Struct has element %s\n",Sq_list1.list);
  printf("Struct has element %s\n",Sq_list2.list);

  MergeList(Sq_list1,Sq_list2,&Sq_list3);
  printf("Merge list has element %s\n",Sq_list3.list);
  printf("%d elements in the queue\n",Sq_list3.last);
  return 0;
}
