#include<iostream>
#include<unistd.h>
#include<pthread.h>
#include"queue.hpp"

using namespace std;

#define QUEUE_SIZE	100
#define THREAD_NUM	100
#define LOOP_NUM	1000000

queue<long long> q(QUEUE_SIZE);

long long total=1;

void* producer(void*p)
{
	long long *d;
	for(int i=0;i<LOOP_NUM;++i){
		while(q.dequeue_free(d)==false);
		*d=__sync_fetch_and_add(&total,1);
		while(q.enqueue_data(d)==false);
	}
}

void* consumer(void*p)
{
	long long*d;
	long long*cnt=(long long*)p;
	for(int i=0;i<LOOP_NUM;++i){
		while(q.dequeue_data(d)==false);
		*cnt += *d;
		while(q.enqueue_free(d)==false);
	}
}

int main()
{
	pthread_t threads[THREAD_NUM*2];
	long long cnt[THREAD_NUM];
	long long sum=0;
	long long expect=THREAD_NUM*LOOP_NUM;
	for(int i=0;i<THREAD_NUM;++i) cnt[i]=0;
	for(int i=0;i<THREAD_NUM;++i){
		if(pthread_create(threads+i,NULL,producer,NULL)) cout << "Error at create thread " << i << endl;
	}
	for(int i=0;i<THREAD_NUM;++i){
		if(pthread_create(threads+i+THREAD_NUM,NULL,consumer,cnt+i)) cout << "Error at create thread " << i+THREAD_NUM << endl;
	}
	for(int i=0;i<THREAD_NUM*2;++i){
		if(pthread_join(threads[i],NULL)) cout << "Error at join thread " << i << endl;
	}
	for(int i=0;i<THREAD_NUM;++i){
		sum+=cnt[i];
		cout << i << "=" << cnt[i] << endl;
	}
	cout << "Actual Value : " << sum << endl;
	expect=(expect*(expect+1))/2;
	cout << "Expect Value : " << expect << endl;
	return 0;
}
