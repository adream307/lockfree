#include<stdio.h>
#include"lfrq.h"



lfrq_queue_t *q;

long long total=1;

void* producer(void*p)
{
 long long *d;
 int i,r=0;
 for(i=0;i<1000000;++i){
  do{do{ for(;;){ int_2t head = __sync_fetch_and_add((&((q)->m_free_head)),0); int_2t tail = __sync_fetch_and_add((&((q)->m_free_tail)),0); int_t head_ptr = (*(((int_t*)(&(head))))); int_t tail_ptr = (*(((int_t*)(&(tail))))); if(head!=__sync_fetch_and_add((&((q)->m_free_head)),0)) continue; if(head_ptr==tail_ptr){ (r)=-1; break; } int_2t head_ptr_old_val = __sync_fetch_and_add((&((q)->m_free_array[head_ptr])),0); int_t head_ptr_old_val_ptr = (*(((int_t*)(&(head_ptr_old_val))))); if(head_ptr_old_val_ptr!=-1){ int_2t head_ptr_new_val = head_ptr_old_val; (*(((int_t*)(&(head_ptr_new_val)))))=-1; (*((((int_t*)(&(head_ptr_new_val)))+1)))=(*((((int_t*)(&(head_ptr_old_val)))+1)))+1; if(__sync_bool_compare_and_swap((&((q)->m_free_array[head_ptr])),(head_ptr_old_val),(head_ptr_new_val))){ int_2t head_new = head; int_t head_new_ptr = (*(((int_t*)(&(head_new)))))+1; if(head_new_ptr==(q)->m_capacity) head_new_ptr=0; (*(((int_t*)(&(head_new)))))=head_new_ptr; (*((((int_t*)(&(head_new)))+1)))=(*((((int_t*)(&(head)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_free_head)),(head),(head_new)); (d)=head_ptr_old_val_ptr*((q)->m_type_size) + ((q)->m_data_queue); (r)=0; break; } } else{ int_2t head_new = head; int_t head_new_ptr = (*(((int_t*)(&(head_new)))))+1; if(head_new_ptr==(q)->m_capacity) head_new_ptr=0; (*(((int_t*)(&(head_new)))))=head_new_ptr; (*((((int_t*)(&(head_new)))+1)))=(*((((int_t*)(&(head)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_free_head)),(head),(head_new)); } } }while(0);;}while(r);
  *d=__sync_fetch_and_add(&total,1);
  do{do{ int_t index = ((d) - (q)->m_data_queue)/((q)->m_type_size); for(;;){ int_2t tail = __sync_fetch_and_add((&((q)->m_data_tail)),0); int_2t head = __sync_fetch_and_add((&((q)->m_data_head)),0); int_t tail_ptr = (*(((int_t*)(&(tail))))); int_t head_ptr = (*(((int_t*)(&(head))))); int_t tail_next_ptr = tail_ptr+1; if(tail_next_ptr==(q)->m_capacity) tail_next_ptr=0; if(tail!=__sync_fetch_and_add((&((q)->m_data_tail)),0)) continue; if(tail_next_ptr==head_ptr){ (r)=-1; break; } int_2t tail_ptr_old_val = __sync_fetch_and_add((&((q)->m_data_array[tail_ptr])),0); int_t tail_ptr_old_val_ptr = (*(((int_t*)(&(tail_ptr_old_val))))); if(tail_ptr_old_val_ptr==-1){ int_2t tail_ptr_new_val = tail_ptr_old_val; (*(((int_t*)(&(tail_ptr_new_val))))) = index; (*((((int_t*)(&(tail_ptr_new_val)))+1))) = (*((((int_t*)(&(tail_ptr_old_val)))+1)))+1; if(__sync_bool_compare_and_swap((&((q)->m_data_array[tail_ptr])),(tail_ptr_old_val),(tail_ptr_new_val))){ int_2t tail_new = tail; (*(((int_t*)(&(tail_new)))))=tail_next_ptr; (*((((int_t*)(&(tail_new)))+1)))=(*((((int_t*)(&(tail)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_data_tail)),(tail),(tail_new)); __sync_fetch_and_add((&((q)->m_type_num)),1); (r)=0; break; } } else{ int_2t tail_new = tail; (*(((int_t*)(&(tail_new)))))=tail_next_ptr; (*((((int_t*)(&(tail_new)))+1)))=(*((((int_t*)(&(tail)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_data_tail)),(tail),(tail_new)); } } }while(0);;}while(r);
 }
}

void* consumer(void*p)
{
 long long*d;
 long long*cnt=(long long*)p;
 int i,r=0;
 for(i=0;i<1000000;++i){
  do{do{ for(;;){ int_2t head = __sync_fetch_and_add((&((q)->m_data_head)),0); int_2t tail = __sync_fetch_and_add((&((q)->m_data_tail)),0); int_t head_ptr = (*(((int_t*)(&(head))))); int_t tail_ptr = (*(((int_t*)(&(tail))))); if(head!=__sync_fetch_and_add((&((q)->m_data_head)),0)) continue; if(head_ptr==tail_ptr){ (r)=-1; break; } int_2t head_ptr_old_val = __sync_fetch_and_add((&((q)->m_data_array[head_ptr])),0); int_t head_ptr_old_val_ptr = (*(((int_t*)(&(head_ptr_old_val))))); if(head_ptr_old_val_ptr!=-1){ int_2t head_ptr_new_val = head_ptr_old_val; (*(((int_t*)(&(head_ptr_new_val)))))=-1; (*((((int_t*)(&(head_ptr_new_val)))+1)))=(*((((int_t*)(&(head_ptr_old_val)))+1)))+1; if(__sync_bool_compare_and_swap((&((q)->m_data_array[head_ptr])),(head_ptr_old_val),(head_ptr_new_val))){ int_2t head_new = head; int_t head_new_ptr = (*(((int_t*)(&(head_new)))))+1; if(head_new_ptr==(q)->m_capacity) head_new_ptr=0; (*(((int_t*)(&(head_new)))))=head_new_ptr; (*((((int_t*)(&(head_new)))+1)))=(*((((int_t*)(&(head)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_data_head)),(head),(head_new)); (d)=head_ptr_old_val_ptr*((q)->m_type_size) + ((q)->m_data_queue); __sync_fetch_and_sub((&((q)->m_type_num)),1); (r))=0; break; } } else{ int_2t head_new = head; int_t head_new_ptr = (*(((int_t*)(&(head_new)))))+1; if(head_new_ptr==(q)->m_capacity) head_new_ptr=0; (*(((int_t*)(&(head_new)))))=head_new_ptr; (*((((int_t*)(&(head_new)))+1)))=(*((((int_t*)(&(head)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_data_head)),(head),(head_new)); } } }while(0);;}while(r);
  *cnt += *d;
  do{do{ int_t index = ((d) - (q)->m_data_queue)/((q)->m_type_size); for(;;){ int_2t tail = __sync_fetch_and_add((&((q)->m_free_tail)),0); int_2t head = __sync_fetch_and_add((&((q)->m_free_head)),0); int_t tail_ptr = (*(((int_t*)(&(tail))))); int_t head_ptr = (*(((int_t*)(&(head))))); int_t tail_next_ptr = tail_ptr+1; if(tail_next_ptr==(q)->m_capacity) tail_next_ptr=0; if(tail!=__sync_fetch_and_add((&((q)->m_free_tail)),0)) continue; if(tail_next_ptr==head_ptr){ (r)=-1; break; } int_2t tail_ptr_old_val = __sync_fetch_and_add((&((q)->m_free_array[tail_ptr])),0); int_t tail_ptr_old_val_ptr = (*(((int_t*)(&(tail_ptr_old_val))))); if(tail_ptr_old_val_ptr==-1){ int_2t tail_ptr_new_val = tail_ptr_old_val; (*(((int_t*)(&(tail_ptr_new_val))))) = index; (*((((int_t*)(&(tail_ptr_new_val)))+1))) = (*((((int_t*)(&(tail_ptr_old_val)))+1)))+1; if(__sync_bool_compare_and_swap((&((q)->m_free_array[tail_ptr])),(tail_ptr_old_val),(tail_ptr_new_val))){ int_2t tail_new = tail; (*(((int_t*)(&(tail_new)))))=tail_next_ptr; (*((((int_t*)(&(tail_new)))+1)))=(*((((int_t*)(&(tail)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_free_tail)),(tail),(tail_new)); (r)=0; break; } } else{ int_2t tail_new = tail; (*(((int_t*)(&(tail_new)))))=tail_next_ptr; (*((((int_t*)(&(tail_new)))+1)))=(*((((int_t*)(&(tail)))+1)))+1; __sync_bool_compare_and_swap((&((q)->m_free_tail)),(tail),(tail_new)); } } }while(0);;}while(r);
 }
}

int main()
{
 pthread_t threads[100*2];
 long long cnt[100];
 long long sum=0;
 long long expect=100*1000000;
 int i;
 do{ int i; if((100)<1){ (q) = ((void *)0); break; } (q)=(lfrq_queue_t*)malloc(sizeof(lfrq_queue_t)); if((q)==((void *)0)) break; (q)->m_capacity=(100)+1; (q)->m_type_size = (sizeof(long long)); (q)->m_data_queue =(void*)calloc((100),(sizeof(long long))); if((q)->m_data_queue==((void *)0)){ free((q)); (q)=((void *)0); break; } (q)->m_free_array =(int_2t*)calloc((q)->m_capacity,sizeof(int_2t)); if((q)->m_free_array==((void *)0)){ free((q)->m_data_queue); free((q)); (q)=((void *)0); break; } (q)->m_data_array =(int_2t*)calloc((q)->m_capacity,sizeof(int_2t)); if((q)->m_data_array==((void *)0)){ free((q)->m_free_array); free((q)->m_data_array); free((q)); (q) = ((void *)0); break; } (*(((int_t*)(&((q)->m_free_tail)))))=(q)->m_capacity-1; (*((((int_t*)(&((q)->m_free_tail)))+1)))=0; (q)->m_free_head=0; (q)->m_data_head=0; (q)->m_data_tail=0; (q)->m_type_num=0; for(i=0; i <((q)->m_capacity); ++i){ (*(((int_t*)(&((q)->m_free_array[i])))))=i; (*((((int_t*)(&((q)->m_free_array[i])))+1)))=0; (*(((int_t*)(&((q)->m_data_array[i])))))=-1; (*((((int_t*)(&((q)->m_data_array[i])))+1)))=0; } (*(((int_t*)(&((q)->m_free_array[(q)->m_capacity-1])))))=-1; }while(0);;
 if(q==((void *)0)){
  printf("create lfrq error\n");
  return 0;
 }
 for(i=0;i<100;++i) cnt[i]=0;
 for(i=0;i<100;++i){
  if(pthread_create(threads+i,((void *)0),producer,((void *)0))){
   printf("Error at create thread %d\n",i);
  }
 }
 for(i=0;i<100;++i){
  if(pthread_create(threads+i+100,((void *)0),consumer,cnt+i)){
   printf("Error at create thread %d\n",i+100);
  }
 }
 for(i=0;i<100*2;++i){
  if(pthread_join(threads[i],((void *)0))){
   printf("Error at join thread %d\n",i);
  }
 }
 for(i=0;i<100;++i){
  sum+=cnt[i];
  printf("%d=%lld\n",i,cnt[i]);
 }
 printf("Actual Value : %lld\n",sum);
 expect=(expect*(expect+1))/2;
 printf("Expect Value : %lld\n",expect);
 return 0;
}
