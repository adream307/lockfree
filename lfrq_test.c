#include"lfrq.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>


#define QUEUE_SIZE	100
#define THREAD_NUM	100
#define LOOP_NUM	1000000

lfrq_t *q;

long long total=1;

void* producer(void*p)
{
	long long *d;
	int i,r=0;
	for(i=0;i<LOOP_NUM;++i){
		do{lfrq_dequeue_free(q,d,r);}while(r);
		*d=__sync_fetch_and_add(&total,1);
		do{lfrq_enqueue_data(q,d,r);}while(r);
	}
}

void* consumer(void*p)
{
	long long*d;
	long long*cnt=(long long*)p;
	int i,r=0;
	for(i=0;i<LOOP_NUM;++i){
		do{lfrq_dequeue_data(q,d,r);}while(r);
		*cnt += *d;
		do{lfrq_enqueue_free(q,d,r);}while(r);
	}
}

int main()
{
	pthread_t threads[THREAD_NUM*2];
	long long cnt[THREAD_NUM];
	long long sum=0;
	long long expect=THREAD_NUM*LOOP_NUM;
	int i;
	lfrq_create(q,QUEUE_SIZE,sizeof(long long));
	if(q==NULL){
		printf("create lfrq error\n");
		return 0;
	}
	for(i=0;i<THREAD_NUM;++i) cnt[i]=0;
	for(i=0;i<THREAD_NUM;++i){
		if(pthread_create(threads+i,NULL,producer,NULL)){
			printf("Error at create thread %d\n",i);
		}
	}
	for(i=0;i<THREAD_NUM;++i){
		if(pthread_create(threads+i+THREAD_NUM,NULL,consumer,cnt+i)){
			printf("Error at create thread %d\n",i+THREAD_NUM);
		}
	}
	for(i=0;i<THREAD_NUM*2;++i){
		if(pthread_join(threads[i],NULL)){
			printf("Error at join thread %d\n",i);
		}
	}
	for(i=0;i<THREAD_NUM;++i){
		sum+=cnt[i];
		printf("%d=%lld\n",i,cnt[i]);
	}
	printf("Actual Value : %lld\n",sum);
	expect=(expect*(expect+1))/2;
	printf("Expect Value : %lld\n",expect);
	lfrq_free(q);
	return 0;
}

