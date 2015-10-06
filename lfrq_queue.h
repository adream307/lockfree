/*
 * lock free ring queue, thread safe
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * lfrq_queue.h
 * enqueue into the tail
 * dequeue from head
 */
#ifndef __LOCK_FREE_RING_QUEUE_H__
#define __LOCK_FREE_RING_QUEUE_H__

typedef int int_t;
typedef long long int_2t;

#define sync_get_2t(val_ptr)					__sync_fetch_and_add((val_ptr),0)
#define sync_inc_t(val_ptr)						__sync_fetch_and_add((val_ptr),1)
#define sync_dec_t(val_ptr)						__sync_fetch_and_sub((val_ptr),1)
#define sync_cas_2t(val_ptr,old_val,new_val)	__sync_bool_compare_and_swap((val_ptr),(old_val),(new_val))

#define int_2t_low_part_ptr(x)		((int_t*)(&(x)))
#define int_2t_high_part_ptr(x)		(((int_t*)(&(x)))+1)
#define int_2t_low_part(x)			(*(int_2t_low_part_ptr(x)))
#define int_2t_high_part(x)			(*(int_2t_high_part_ptr(x)))

typedef struct{
	void *m_data_queue;
	int_2t *m_free_array;
	int_2t *m_data_array;
	int_2t m_free_head;
	int_2t m_free_tail;
	int_2t m_data_head;
	int_2t m_data_tail;
	int_t m_capacity;
	int_t m_type_size;
	int_t m_type_num;
}lfrq_queue_t;

lfrq_queue_t* lfrq_queue_create(const int_t type_number,const int_t type_size);
void lfrq_queue_free(lfrq_queue_t *p);
int lfrq_queue_enqueue_free(lfrq_queue_t *f, const void*p); //if success, reurn 0, else return -1;
int lfrq_queue_dequeue_free(lfrq_queue_t *f, void**p); //if success, reurn 0, else return -1;
int lfrq_queue_enqueue_data(lfrq_queue_t *f, const void*p); //if success, reurn 0, else return -1;
int lfrq_queue_dequeue_data(lfrq_queue_t *f, void**p); //if success, reurn 0, else return -1;
const int_t lfrq_size(lfrq_queue_t *f);

#endif
