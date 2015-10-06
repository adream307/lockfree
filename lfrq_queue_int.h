/*
 * lock free queue, thread safe, int only
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * queue.hpp
 * enqueue into the tail
 * dequeue from head
 */

#ifndef __LOCK_FREE_RING_QUEUE_INT_H__
#define __LOCK_FREE_RING_QUEUE_INT_H__

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
	int_2t *m_data;
	int_2t m_head;
	int_2t m_tail;
	int_t  m_cap;
	int_t m_null;
	int_t  m_size;
}lfrq_queue_int_t;

lfrq_queue_int_t* lfrq_queue_int_create(int_t cap,int_t null_val);
void lfrq_queue_int_free(lfrq_queue_int_t *p);
int lfrq_queue_int_enqueue(lfrq_queue_int_t *p, int_t val); //if sucess return 0, else return -1;
int lfrq_queue_int_dequeue(lfrq_queue_int_t *p, int_t *val_ptr);
int_t lfrq_queue_int_size(lfrq_queue_int_t *p);
int_t lfrq_queue_int_null(lfrq_queue_int_t *p);

#endif

