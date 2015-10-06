/*
 * lock free queue, thread safe
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * queue.hpp
 * enqueue into the tail
 * dequeue from head
 */
#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

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

template<typename T>
class queue
{
private:
	T *m_data_queue;
	int_2t *m_free_array;
	int_2t *m_data_array;
	int_2t m_free_head;
	int_2t m_free_tail;
	int_2t m_data_head;
	int_2t m_data_tail;
	const int_t m_capacity;
	int_t m_size;
public:
	queue(const int_t size=1024):m_capacity(size+1)
	{
		if(m_capacity>1){
			m_data_queue = new T[m_capacity-1];
			m_free_array = new int_2t[m_capacity];
			m_data_array = new int_2t[m_capacity];
			int_2t_low_part(m_free_tail)=m_capacity-1;
			int_2t_high_part(m_free_tail)=0;
			m_free_head=0;
			m_data_head=0;
			m_data_tail=0;
			m_size=0;
			for(int i=0;i<m_capacity;++i){
				int_2t_low_part(m_free_array[i])=i;
				int_2t_high_part(m_free_array[i])=0;
				int_2t_low_part(m_data_array[i])=-1;
				int_2t_high_part(m_data_array[i])=0;
			}
			int_2t_low_part(m_free_array[m_capacity-1])=-1;
		}
	}
	~queue()
	{
		if(m_capacity>1){
			delete [] m_data_queue;
			delete [] m_free_array;
			delete [] m_data_array;
		}
	}
	//enqueue one free node back to the tail
	bool enqueue_free(const T *p)
	{
		int_t index = p - m_data_queue;
		while(true){
			int_2t tail = sync_get_2t(&m_free_tail);
			int_2t head = sync_get_2t(&m_free_head);
			int_t tail_ptr = int_2t_low_part(tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_next_ptr = tail_ptr+1;
			if(tail_next_ptr==m_capacity) tail_next_ptr=0;
			if(tail!=sync_get_2t(&m_free_tail)) continue;
			if(tail_next_ptr==head_ptr) return false;
			int_2t tail_ptr_old_val = sync_get_2t(&(m_free_array[tail_ptr]));
			int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);
			if(tail_ptr_old_val_ptr==-1){
				int_2t tail_ptr_new_val = tail_ptr_old_val;
				int_2t_low_part(tail_ptr_new_val) = index;
				int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;
				if(sync_cas_2t(&(m_free_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){
					int_2t tail_new = tail;
					int_2t_low_part(tail_new)=tail_next_ptr;
					int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
					sync_cas_2t(&m_free_tail,tail,tail_new);
					return true;
				}
			}
			else{
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&m_free_tail,tail,tail_new);
			}
		}
	}
	//dequeue one free node from the head
	bool dequeue_free(T *(&p))
	{
		while(true){
			int_2t head = sync_get_2t(&m_free_head);
			int_2t tail = sync_get_2t(&m_free_tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_ptr = int_2t_low_part(tail);
			if(head!=sync_get_2t(&m_free_head)) continue;
			if(head_ptr==tail_ptr) return false;
			int_2t head_ptr_old_val = sync_get_2t(&(m_free_array[head_ptr]));
			int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);
			if(head_ptr_old_val_ptr!=-1){
				int_2t head_ptr_new_val = head_ptr_old_val;
				int_2t_low_part(head_ptr_new_val)=-1;
				int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;
				if(sync_cas_2t(&(m_free_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){
					int_2t head_new = head;
					int_t head_new_ptr = int_2t_low_part(head_new)+1;
					if(head_new_ptr==m_capacity) head_new_ptr=0;
					int_2t_low_part(head_new)=head_new_ptr;
					int_2t_high_part(head_new)=int_2t_high_part(head)+1;
					sync_cas_2t(&m_free_head,head,head_new);
					p=head_ptr_old_val_ptr + m_data_queue;
					return true;
				}
			}
			else{
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head_new)+1;
				if(head_new_ptr==m_capacity) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&m_free_head,head,head_new);
			}
		}
	}
	//enqueue one data node back to the tail
	bool enqueue_data(const T *p)
	{
		int_t index = p - m_data_queue;
		while(true){
			int_2t tail = sync_get_2t(&m_data_tail);
			int_2t head = sync_get_2t(&m_data_head);
			int_t tail_ptr = int_2t_low_part(tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_next_ptr = tail_ptr+1;
			if(tail_next_ptr==m_capacity) tail_next_ptr=0;
			if(tail!=sync_get_2t(&m_data_tail)) continue;
			if(tail_next_ptr==head_ptr) return false;
			int_2t tail_ptr_old_val = sync_get_2t(&(m_data_array[tail_ptr]));
			int_t tail_ptr_old_val_ptr = int_2t_low_part(tail_ptr_old_val);
			if(tail_ptr_old_val_ptr==-1){
				int_2t tail_ptr_new_val = tail_ptr_old_val;
				int_2t_low_part(tail_ptr_new_val) = index;
				int_2t_high_part(tail_ptr_new_val) = int_2t_high_part(tail_ptr_old_val)+1;
				if(sync_cas_2t(&(m_data_array[tail_ptr]),tail_ptr_old_val,tail_ptr_new_val)){
					int_2t tail_new = tail;
					int_2t_low_part(tail_new)=tail_next_ptr;
					int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
					sync_cas_2t(&m_data_tail,tail,tail_new);
					sync_inc_t(&m_size);
					return true;
				}
			}
			else{
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&m_data_tail,tail,tail_new);
			}
		}
	}
	//dequeue one data node from the head
	bool dequeue_data(T *(&p))
	{
		while(true){
			int_2t head = sync_get_2t(&m_data_head);
			int_2t tail = sync_get_2t(&m_data_tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_ptr = int_2t_low_part(tail);
			if(head!=sync_get_2t(&m_data_head)) continue;
			if(head_ptr==tail_ptr) return false;
			int_2t head_ptr_old_val = sync_get_2t(&(m_data_array[head_ptr]));
			int_t head_ptr_old_val_ptr = int_2t_low_part(head_ptr_old_val);
			if(head_ptr_old_val_ptr!=-1){
				int_2t head_ptr_new_val = head_ptr_old_val;
				int_2t_low_part(head_ptr_new_val)=-1;
				int_2t_high_part(head_ptr_new_val)=int_2t_high_part(head_ptr_old_val)+1;
				if(sync_cas_2t(&(m_data_array[head_ptr]),head_ptr_old_val,head_ptr_new_val)){
					int_2t head_new = head;
					int_t head_new_ptr = int_2t_low_part(head_new)+1;
					if(head_new_ptr==m_capacity) head_new_ptr=0;
					int_2t_low_part(head_new)=head_new_ptr;
					int_2t_high_part(head_new)=int_2t_high_part(head)+1;
					sync_cas_2t(&m_data_head,head,head_new);
					p=head_ptr_old_val_ptr + m_data_queue;
					sync_dec_t(&m_size);
					return true;
				}
			}
			else{
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head_new)+1;
				if(head_new_ptr==m_capacity) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&m_data_head,head,head_new);
			}
		}
	}
	const int_t size(){ return m_size; }
};

#endif

