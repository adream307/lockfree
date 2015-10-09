/*
 * lock free ring queue, thread safe
 * lfrq_queue.c
 */

#include"lfrq_queue.h"
#include<stdlib.h>

lfrq_queue_t* lfrq_queue_create(const int_t type_number,const int_t type_size)
{
	int i;
	if(type_number<1) return NULL;
	lfrq_queue_t *p=(lfrq_queue_t*)malloc(sizeof(lfrq_queue_t));
	if(p==NULL) return NULL;
	p->m_capacity=type_number+1;
	p->m_type_size = type_size;
	p->m_data_queue =(void*)calloc(type_number,type_size);
	if(p->m_data_queue==NULL){
		free(p);
		return NULL;
	}
	p->m_free_array =(int_2t*)calloc(p->m_capacity,sizeof(int_2t));
	if(p->m_free_array==NULL){
		free(p->m_data_queue);
		free(p);
		return NULL;
	}
	p->m_data_array =(int_2t*)calloc(p->m_capacity,sizeof(int_2t));
	if(p->m_data_array==NULL){
		free(p->m_free_array);
		free(p->m_data_array);
		free(p);
		return NULL;
	}
	int_2t_low_part(p->m_free_tail)=p->m_capacity-1;
	int_2t_high_part(p->m_free_tail)=0;
	p->m_free_head=0;
	p->m_data_head=0;
	p->m_data_tail=0;
	p->m_type_num=0;
	for(i=0; i < p->m_capacity; ++i){
		int_2t_low_part(p->m_free_array[i])=i;
		int_2t_high_part(p->m_free_array[i])=0;
		int_2t_low_part(p->m_data_array[i])=-1;
		int_2t_high_part(p->m_data_array[i])=0;
	}
	int_2t_low_part(p->m_free_array[p->m_capacity-1])=-1;
	return p;
}

void lfrq_queue_free(lfrq_queue_t *p)
{
	if(p->m_capacity>1){
		free(p->m_data_queue);
		free(p->m_free_array);
		free(p->m_data_array);
		free(p);
	}
}

//enqueue one free node back to the tail
int lfrq_queue_enqueue_free(lfrq_queue_t *f, const void*p)
{
	int_t index = (p - f->m_data_queue)/(f->m_type_size);
	for(;;){
		int_2t tail = sync_get_2t(&(f->m_free_tail));
		int_2t head = sync_get_2t(&(f->m_free_head));
		int_t tail_ptr = int_2t_low_part(tail);
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_next_ptr = tail_ptr+1;
		if(tail_next_ptr==f->m_capacity) tail_next_ptr=0;
		if(tail!=sync_get_2t(&(f->m_free_tail))) continue;
		if(tail_next_ptr==head_ptr) return -1;
		int_2t tail_ptr_old_val = sync_get_2t(&(f->m_free_array[tail_ptr]));
		int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);
		if(tail_ptr_old_val_ptr==-1){
			int_2t tail_ptr_new_val = tail_ptr_old_val;
			int_2t_low_part(tail_ptr_new_val) = index;
			int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;
			if(sync_cas_2t(&(f->m_free_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&(f->m_free_tail),tail,tail_new);
				return 0;
			}
		}
		else{
			int_2t tail_new = tail;
			int_2t_low_part(tail_new)=tail_next_ptr;
			int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
			sync_cas_2t(&(f->m_free_tail),tail,tail_new);
		}
	}
}

//dequeue one free node from the head
int lfrq_queue_dequeue_free(lfrq_queue_t *f, void**p)
{
	for(;;){
		int_2t head = sync_get_2t(&(f->m_free_head));
		int_2t tail = sync_get_2t(&(f->m_free_tail));
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_ptr = int_2t_low_part(tail);
		if(head!=sync_get_2t(&(f->m_free_head))) continue;
		if(head_ptr==tail_ptr) return -1;
		int_2t head_ptr_old_val = sync_get_2t(&(f->m_free_array[head_ptr]));
		int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);
		if(head_ptr_old_val_ptr!=-1){
			int_2t head_ptr_new_val = head_ptr_old_val;
			int_2t_low_part(head_ptr_new_val)=-1;
			int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;
			if(sync_cas_2t(&(f->m_free_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head_new)+1;
				if(head_new_ptr==f->m_capacity) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&(f->m_free_head),head,head_new);
				*p=head_ptr_old_val_ptr*f->m_type_size + f->m_data_queue;
				return 0;
			}
		}
		else{
			int_2t head_new = head;
			int_t head_new_ptr = int_2t_low_part(head_new)+1;
			if(head_new_ptr==f->m_capacity) head_new_ptr=0;
			int_2t_low_part(head_new)=head_new_ptr;
			int_2t_high_part(head_new)=int_2t_high_part(head)+1;
			sync_cas_2t(&(f->m_free_head),head,head_new);
		}
	}
}
//enqueue one data node back to the tail
int lfrq_queue_enqueue_data(lfrq_queue_t *f, const void*p)
{
	sync_write_read_fence();
	int_t index = (p - f->m_data_queue)/(f->m_type_size);
	for(;;){
		int_2t tail = sync_get_2t(&(f->m_data_tail));
		int_2t head = sync_get_2t(&(f->m_data_head));
		int_t tail_ptr = int_2t_low_part(tail);
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_next_ptr = tail_ptr+1;
		if(tail_next_ptr==f->m_capacity) tail_next_ptr=0;
		if(tail!=sync_get_2t(&(f->m_data_tail))) continue;
		if(tail_next_ptr==head_ptr) return -1;
		int_2t tail_ptr_old_val = sync_get_2t(&(f->m_data_array[tail_ptr]));
		int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);
		if(tail_ptr_old_val_ptr==-1){
			int_2t tail_ptr_new_val = tail_ptr_old_val;
			int_2t_low_part(tail_ptr_new_val) = index;
			int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;
			if(sync_cas_2t(&(f->m_data_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&(f->m_data_tail),tail,tail_new);
				sync_inc_t(&(f->m_type_num));
				return 0;
			}
		}
		else{
			int_2t tail_new = tail;
			int_2t_low_part(tail_new)=tail_next_ptr;
			int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
			sync_cas_2t(&(f->m_data_tail),tail,tail_new);
		}
	}
}
//dequeue one data node from the head
int lfrq_queue_dequeue_data(lfrq_queue_t *f, void**p)
{
	for(;;){
		int_2t head = sync_get_2t(&(f->m_data_head));
		int_2t tail = sync_get_2t(&(f->m_data_tail));
		int_t head_ptr = int_2t_low_part(head);
		int_t tail_ptr = int_2t_low_part(tail);
		if(head!=sync_get_2t(&(f->m_data_head))) continue;
		if(head_ptr==tail_ptr) return -1;
		int_2t head_ptr_old_val = sync_get_2t(&(f->m_data_array[head_ptr]));
		int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);
		if(head_ptr_old_val_ptr!=-1){
			int_2t head_ptr_new_val = head_ptr_old_val;
			int_2t_low_part(head_ptr_new_val)=-1;
			int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;
			if(sync_cas_2t(&(f->m_data_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head_new)+1;
				if(head_new_ptr==f->m_capacity) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&(f->m_data_head),head,head_new);
				*p=head_ptr_old_val_ptr*f->m_type_size + f->m_data_queue;
				sync_dec_t(&(f->m_type_num));
				return 0;
			}
		}
		else{
			int_2t head_new = head;
			int_t head_new_ptr = int_2t_low_part(head_new)+1;
			if(head_new_ptr==f->m_capacity) head_new_ptr=0;
			int_2t_low_part(head_new)=head_new_ptr;
			int_2t_high_part(head_new)=int_2t_high_part(head)+1;
			sync_cas_2t(&(f->m_data_head),head,head_new);
		}
	}
}
const int_t lfrq_size(lfrq_queue_t *f){ return f->m_type_num; }

