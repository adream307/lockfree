/*
 * lock free ring queue, thread safe
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * lfrq.h
 * enqueue into the tail
 * dequeue from head
 */
#include<stdlib.h>

#ifndef __LOCK_FREE_RING_QUEUE_H__
#define __LOCK_FREE_RING_QUEUE_H__

typedef int int_t;
typedef long long int_2t;

#define sync_get_2t(val_ptr)					__sync_fetch_and_add((val_ptr),0)
#define sync_inc_t(val_ptr)						__sync_fetch_and_add((val_ptr),1)
#define sync_dec_t(val_ptr)						__sync_fetch_and_sub((val_ptr),1)
#define sync_cas_2t(val_ptr,old_val,new_val)	__sync_bool_compare_and_swap((val_ptr),(old_val),(new_val))
#define sync_write_read_fence()					__sync_synchronize()

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
}lfrq_t;

//_n, the number of queue; _s, the size of each data
//void lfrq_queue_create(,lfrq_queue_t *_f, const int_t _n, const _s); 
#define lfrq_create(_f,_n,_s)													\
do{																					\
	int i;																			\
	if((_n)<1){																		\
		(_f) = NULL;																\
		break;																		\
	}																				\
	(_f)=(lfrq_t*)malloc(sizeof(lfrq_t));							\
	if((_f)==NULL) break;														\
	(_f)->m_capacity=(_n)+1;														\
	(_f)->m_type_size = (_s);													\
	(_f)->m_data_queue =(void*)calloc((_n),(_s));								\
	if((_f)->m_data_queue==NULL){												\
		free((_f));																\
		(_f)=NULL;																\
		break;																		\
	}																				\
	(_f)->m_free_array =(int_2t*)calloc((_f)->m_capacity,sizeof(int_2t));		\
	if((_f)->m_free_array==NULL){												\
		free((_f)->m_data_queue);												\
		free((_f));																\
		(_f)=NULL;																\
		break;																		\
	}																				\
	(_f)->m_data_array =(int_2t*)calloc((_f)->m_capacity,sizeof(int_2t));		\
	if((_f)->m_data_array==NULL){												\
		free((_f)->m_free_array);												\
		free((_f)->m_data_array);												\
		free((_f));																\
		(_f) = NULL;																\
		break;																		\
	}																				\
	int_2t_low_part((_f)->m_free_tail)=(_f)->m_capacity-1;					\
	int_2t_high_part((_f)->m_free_tail)=0;										\
	(_f)->m_free_head=0;															\
	(_f)->m_data_head=0;															\
	(_f)->m_data_tail=0;															\
	(_f)->m_type_num=0;															\
	for(i=0; i <((_f)->m_capacity); ++i){										\
		int_2t_low_part((_f)->m_free_array[i])=i;								\
		int_2t_high_part((_f)->m_free_array[i])=0;								\
		int_2t_low_part((_f)->m_data_array[i])=-1;								\
		int_2t_high_part((_f)->m_data_array[i])=0;								\
	}																				\
	int_2t_low_part((_f)->m_free_array[(_f)->m_capacity-1])=-1;				\
}while(0);

//void lfrq_queue_free(lfrq_queue_t *_f);
#define lfrq_free(_f)			\
do{									\
	if((_f)->m_capacity>1){			\
		free((_f)->m_data_queue);	\
		free((_f)->m_free_array);	\
		free((_f)->m_data_array);	\
		free((_f));					\
	}								\
}while(0);


//void lfrq_queue_enqueue_free(lfrq_queue_t *_f, const void*_p, int _r); //if success, _r=0, else _r=-1;
#define lfrq_enqueue_free(_f,_p,_r)														\
do{																								\
	int_t index = (((void*)(_p)) - (_f)->m_data_queue)/((_f)->m_type_size);						\
	for(;;){																					\
		int_2t tail = sync_get_2t(&((_f)->m_free_tail));										\
		int_2t head = sync_get_2t(&((_f)->m_free_head));										\
		int_t tail_ptr = int_2t_low_part(tail);													\
		int_t head_ptr = int_2t_low_part(head);													\
		int_t tail_next_ptr = tail_ptr+1;														\
		if(tail_next_ptr==(_f)->m_capacity) tail_next_ptr=0;									\
		if(tail!=sync_get_2t(&((_f)->m_free_tail))) continue;									\
		if(tail_next_ptr==head_ptr){															\
			(_r)=-1;																			\
			break;																				\
		}																						\
		int_2t tail_ptr_old_val = sync_get_2t(&((_f)->m_free_array[tail_ptr]));					\
		int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);							\
		if(tail_ptr_old_val_ptr==-1){															\
			int_2t tail_ptr_new_val = tail_ptr_old_val;											\
			int_2t_low_part(tail_ptr_new_val) = index;											\
			int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;			\
			if(sync_cas_2t(&((_f)->m_free_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){	\
				int_2t tail_new = tail;															\
				int_2t_low_part(tail_new)=tail_next_ptr;										\
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;							\
				sync_cas_2t(&((_f)->m_free_tail),tail,tail_new);								\
				(_r)=0;																		\
				break;																			\
			}																					\
		}																						\
		else{																					\
			int_2t tail_new = tail;																\
			int_2t_low_part(tail_new)=tail_next_ptr;											\
			int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;								\
			sync_cas_2t(&((_f)->m_free_tail),tail,tail_new);									\
		}																						\
	}																							\
}while(0);

//void lfrq_queue_dequeue_free(lfrq_queue_t *_f, const void*_p, int_r); //if success, _r=0, else _r=-1;
#define lfrq_dequeue_free(_f,_p,_r)														\
do{																								\
	for(;;){																					\
		int_2t head = sync_get_2t(&((_f)->m_free_head));										\
		int_2t tail = sync_get_2t(&((_f)->m_free_tail));										\
		int_t head_ptr = int_2t_low_part(head);													\
		int_t tail_ptr = int_2t_low_part(tail);													\
		if(head!=sync_get_2t(&((_f)->m_free_head))) continue;									\
		if(head_ptr==tail_ptr){																	\
			(_r)=-1;																			\
			break;																				\
		}																						\
		int_2t head_ptr_old_val = sync_get_2t(&((_f)->m_free_array[head_ptr]));					\
		int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);							\
		if(head_ptr_old_val_ptr!=-1){															\
			int_2t head_ptr_new_val = head_ptr_old_val;											\
			int_2t_low_part(head_ptr_new_val)=-1;												\
			int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;			\
			if(sync_cas_2t(&((_f)->m_free_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){	\
				int_2t head_new = head;															\
				int_t head_new_ptr = int_2t_low_part(head_new)+1;								\
				if(head_new_ptr==(_f)->m_capacity) head_new_ptr=0;								\
				int_2t_low_part(head_new)=head_new_ptr;											\
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;							\
				sync_cas_2t(&((_f)->m_free_head),head,head_new);								\
				(_p)=head_ptr_old_val_ptr*((_f)->m_type_size) + ((_f)->m_data_queue);		\
				(_r)=0;																		\
				break;																			\
			}																					\
		}																						\
		else{																					\
			int_2t head_new = head;																\
			int_t head_new_ptr = int_2t_low_part(head_new)+1;									\
			if(head_new_ptr==(_f)->m_capacity) head_new_ptr=0;									\
			int_2t_low_part(head_new)=head_new_ptr;												\
			int_2t_high_part(head_new)=int_2t_high_part(head)+1;								\
			sync_cas_2t(&((_f)->m_free_head),head,head_new);									\
		}																						\
	}																							\
}while(0);

//void lfrq_queue_enqueue_data(lfrq_queue_t *_f, const void*_p, int_r); //if success, _r=0, else _r=-1;
#define lfrq_enqueue_data(_f,_p,_r)														\
do{																								\
	sync_write_read_fence();																	\
	int_t index = (((void*)(_p)) - (_f)->m_data_queue)/((_f)->m_type_size);						\
	for(;;){																					\
		int_2t tail = sync_get_2t(&((_f)->m_data_tail));										\
		int_2t head = sync_get_2t(&((_f)->m_data_head));										\
		int_t tail_ptr = int_2t_low_part(tail);													\
		int_t head_ptr = int_2t_low_part(head);													\
		int_t tail_next_ptr = tail_ptr+1;														\
		if(tail_next_ptr==(_f)->m_capacity) tail_next_ptr=0;									\
		if(tail!=sync_get_2t(&((_f)->m_data_tail))) continue;									\
		if(tail_next_ptr==head_ptr){															\
			(_r)=-1;																			\
			break;																				\
		}																						\
		int_2t tail_ptr_old_val = sync_get_2t(&((_f)->m_data_array[tail_ptr]));					\
		int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);							\
		if(tail_ptr_old_val_ptr==-1){															\
			int_2t tail_ptr_new_val = tail_ptr_old_val;											\
			int_2t_low_part(tail_ptr_new_val) = index;											\
			int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;			\
			if(sync_cas_2t(&((_f)->m_data_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){	\
				int_2t tail_new = tail;															\
				int_2t_low_part(tail_new)=tail_next_ptr;										\
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;							\
				sync_cas_2t(&((_f)->m_data_tail),tail,tail_new);								\
				sync_inc_t(&((_f)->m_type_num));												\
				(_r)=0;																		\
				break;																			\
			}																					\
		}																						\
		else{																					\
			int_2t tail_new = tail;																\
			int_2t_low_part(tail_new)=tail_next_ptr;											\
			int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;								\
			sync_cas_2t(&((_f)->m_data_tail),tail,tail_new);									\
		}																						\
	}																							\
}while(0);

//void lfrq_queue_dequeue_data(lfrq_queue_t *_f, void*_p,int _r); //if success, (_r)=0, else (_r)-1;
#define lfrq_dequeue_data(_f,_p,_r)														\
do{																								\
	for(;;){																					\
		int_2t head = sync_get_2t(&((_f)->m_data_head));										\
		int_2t tail = sync_get_2t(&((_f)->m_data_tail));										\
		int_t head_ptr = int_2t_low_part(head);													\
		int_t tail_ptr = int_2t_low_part(tail);													\
		if(head!=sync_get_2t(&((_f)->m_data_head))) continue;									\
		if(head_ptr==tail_ptr){																	\
			(_r)=-1;																			\
			break;																				\
		}																						\
		int_2t head_ptr_old_val = sync_get_2t(&((_f)->m_data_array[head_ptr]));					\
		int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);							\
		if(head_ptr_old_val_ptr!=-1){															\
			int_2t head_ptr_new_val = head_ptr_old_val;											\
			int_2t_low_part(head_ptr_new_val)=-1;												\
			int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;			\
			if(sync_cas_2t(&((_f)->m_data_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){	\
				int_2t head_new = head;															\
				int_t head_new_ptr = int_2t_low_part(head_new)+1;								\
				if(head_new_ptr==(_f)->m_capacity) head_new_ptr=0;								\
				int_2t_low_part(head_new)=head_new_ptr;											\
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;							\
				sync_cas_2t(&((_f)->m_data_head),head,head_new);								\
				(_p)=head_ptr_old_val_ptr*((_f)->m_type_size) + ((_f)->m_data_queue);		\
				sync_dec_t(&((_f)->m_type_num));												\
				(_r)=0;																		\
				break;																			\
			}																					\
		}																						\
		else{																					\
			int_2t head_new = head;																\
			int_t head_new_ptr = int_2t_low_part(head_new)+1;									\
			if(head_new_ptr==(_f)->m_capacity) head_new_ptr=0;									\
			int_2t_low_part(head_new)=head_new_ptr;												\
			int_2t_high_part(head_new)=int_2t_high_part(head)+1;								\
			sync_cas_2t(&((_f)->m_data_head),head,head_new);									\
		}																						\
	}																							\
}while(0);

#define lfrq_size(_f)	((_f)->m_type_num)

#endif
