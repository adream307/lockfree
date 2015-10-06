/*
 * lock free queue, thread safe, int only
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * queue.hpp
 * enqueue into the tail
 * dequeue from head
 */

#include"lfrq_queue_int.h"
#include<stdlib.h>

lfrq_queue_int_t* lfrq_queue_int_create(int_t cap,int_t null_val)
{
	int i;
	if(cap<1) return NULL;
	lfrq_queue_int_t *p = (lfrq_queue_int_t*)malloc(sizeof(lfrq_queue_int_t));
	if(p==NULL) return NULL;
	p->m_cap=cap;
	p->m_null=null_val;
	p->m_data =(int_2t*)calloc(cap,sizeof(int_2t));
	if(p->m_data==NULL){
		free(p);
		return NULL;
	}
	for(i=0;i<cap;++i){
		int_2t_low_part(p->m_data[i])=null_val;
		int_2t_high_part(p->m_data[i])=0;
	}
	p->m_head=0;
	p->m_tail=0;
	p->m_size=0;
	return p;
}

void lfrq_queue_int_free(lfrq_queue_int_t *p)
{
	if(p->m_cap>0){
		free(p->m_data);
		free(p);
	}
}

int lfrq_queue_int_enqueue(lfrq_queue_int_t *p, int_t val)
{
	for(;;){
		int_2t tail = sync_get_2t(&(p->m_tail));
		int_2t head = sync_get_2t(&(p->m_head));
		int_t tail_ptr = int_2t_low_part(tail);
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_next_ptr = tail_ptr+1;
		if(tail_next_ptr==p->m_cap) tail_next_ptr=0;
		if(tail!=sync_get_2t(&(p->m_tail))) continue;
		if(tail_next_ptr==head_ptr) return -1;
		int_2t tail_ptr_old = sync_get_2t(&(p->m_data[tail_ptr]));
		int_t tail_ptr_old_val = int_2t_low_part(tail_ptr_old);
		if(tail_ptr_old_val==p->m_null){
			int_2t tail_ptr_new = tail_ptr_old;
			int_2t_low_part(tail_ptr_new) = val;
			int_2t_high_part(tail_ptr_new) = int_2t_high_part(tail_ptr_old)+1;
			if(sync_cas_2t(&(p->m_data[tail_ptr]),tail_ptr_old,tail_ptr_new)){
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&(p->m_tail),tail,tail_new);
				sync_inc_t(&(p->m_size));
				return 0;
			}
		}
		else{
			int_2t tail_new = tail;
			int_2t_low_part(tail_new)=tail_next_ptr;
			int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
			sync_cas_2t(&(p->m_tail),tail,tail_new);
		}
	}
}

int lfrq_queue_int_dequeue(lfrq_queue_int_t *p, int_t *val_ptr)
{
	for(;;){
		int_2t head = sync_get_2t(&(p->m_head));
		int_2t tail = sync_get_2t(&(p->m_tail));
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_ptr = int_2t_low_part(tail);
		if(head!=sync_get_2t(&(p->m_head))) continue;
		if(head_ptr==tail_ptr) return -1;
		int_2t head_ptr_old = sync_get_2t(&(p->m_data[head_ptr]));
		int_t head_ptr_old_val = int_2t_low_part(head_ptr_old);
		if(head_ptr_old_val!=p->m_null){
			int_2t head_ptr_new = head_ptr_old;
			int_2t_low_part(head_ptr_new)=p->m_null;
			int_2t_high_part(head_ptr_new)=int_2t_high_part(head_ptr_old)+1;
			if(sync_cas_2t(&(p->m_data[head_ptr]),head_ptr_old,head_ptr_new)){
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head)+1;
				if(head_new_ptr==p->m_cap) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&(p->m_head),head,head_new);
				*val_ptr=head_ptr_old_val;
				sync_dec_t(&(p->m_size));
				return 0;
			}
		}
		else{
			int_2t head_new = head;
			int_t head_new_ptr = int_2t_low_part(head)+1;
			if(head_new_ptr==p->m_cap) head_new_ptr=0;
			int_2t_low_part(head_new)=head_new_ptr;
			int_2t_high_part(head_new)=int_2t_high_part(head)+1;
			sync_cas_2t(&(p->m_head),head,head_new);
		}
	}
}
int_t lfrq_queue_int_size(lfrq_queue_int_t *p) {return p->m_size;}
int_t lfrq_queue_int_null(lfrq_queue_int_t *p) {return p->m_null;}

