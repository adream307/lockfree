/*
 * lock free queue, thread safe, int only
 * this paper "A practical nonblocking queue algorithm using compare-and-swap" provide the main idea
 * queue.hpp
 * enqueue into the tail
 * dequeue from head
 */
#ifndef __QUEUE_INT_HPP__
#define __QUEUE_INT_HPP__

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

class queue_int
{
private:
	int_2t *m_data;
	int_2t m_head;
	int_2t m_tail;
	const int_t  m_cap;
	const int_t m_null;
	int_t  m_size;
public:
	queue_int(const int_t cap=1024,const int_t null_val=-1):
		m_cap(cap),m_null(null_val),m_size(0)
	{
		if(m_cap<1) return;
		m_data = new int_2t[m_cap];
		for(int i=0;i<m_cap;++i){
			int_2t_low_part(m_data[i])=m_null;
			int_2t_high_part(m_data[i])=0;
		}
		m_head=0;
		m_tail=0;
		m_size=0;
	}
	~queue_int()
	{
		if(m_cap>0) delete [] m_data;
	}
	bool enqueue(int_t val)
	{
		while(true){
			int_2t tail = sync_get_2t(&m_tail);
			int_2t head = sync_get_2t(&m_head);
			int_t tail_ptr = int_2t_low_part(tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_next_ptr = tail_ptr+1;
			if(tail_next_ptr==m_cap) tail_next_ptr=0;
			if(tail!=sync_get_2t(&m_tail)) continue;
			if(tail_next_ptr==head_ptr) return false;
			int_2t tail_ptr_old = sync_get_2t(&(m_data[tail_ptr]));
			int_t tail_ptr_old_val = int_2t_low_part(tail_ptr_old);
			if(tail_ptr_old_val==m_null){
				int_2t tail_ptr_new = tail_ptr_old;
				int_2t_low_part(tail_ptr_new) = val;
				int_2t_high_part(tail_ptr_new) = int_2t_high_part(tail_ptr_old)+1;
				if(sync_cas_2t(&(m_data[tail_ptr]),tail_ptr_old,tail_ptr_new)){
					int_2t tail_new = tail;
					int_2t_low_part(tail_new)=tail_next_ptr;
					int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
					sync_cas_2t(&m_tail,tail,tail_new);
					sync_inc_t(&m_size);
					return true;
				}
			}
			else{
				int_2t tail_new = tail;
				int_2t_low_part(tail_new)=tail_next_ptr;
				int_2t_high_part(tail_new)=int_2t_high_part(tail)+1;
				sync_cas_2t(&m_tail,tail,tail_new);
			}
		}
	}
	bool dequeue(int_t &val)
	{
		while(true){
			int_2t head = sync_get_2t(&m_head);
			int_2t tail = sync_get_2t(&m_tail);
			int_t head_ptr = int_2t_low_part(head);
			int_t tail_ptr = int_2t_low_part(tail);
			if(head!=sync_get_2t(&m_head)) continue;
			if(head_ptr==tail_ptr) return false;
			int_2t head_ptr_old = sync_get_2t(&(m_data[head_ptr]));
			int_t head_ptr_old_val = int_2t_low_part(head_ptr_old);
			if(head_ptr_old_val!=m_null){
				int_2t head_ptr_new = head_ptr_old;
				int_2t_low_part(head_ptr_new)=m_null;
				int_2t_high_part(head_ptr_new)=int_2t_high_part(head_ptr_old)+1;
				if(sync_cas_2t(&(m_data[head_ptr]),head_ptr_old,head_ptr_new)){
					int_2t head_new = head;
					int_t head_new_ptr = int_2t_low_part(head)+1;
					if(head_new_ptr==m_cap) head_new_ptr=0;
					int_2t_low_part(head_new)=head_new_ptr;
					int_2t_high_part(head_new)=int_2t_high_part(head)+1;
					sync_cas_2t(&m_head,head,head_new);
					val=head_ptr_old_val;
					sync_dec_t(&m_size);
					return true;
				}
			}
			else{
				int_2t head_new = head;
				int_t head_new_ptr = int_2t_low_part(head)+1;
				if(head_new_ptr==m_cap) head_new_ptr=0;
				int_2t_low_part(head_new)=head_new_ptr;
				int_2t_high_part(head_new)=int_2t_high_part(head)+1;
				sync_cas_2t(&m_head,head,head_new);
			}
		}
	}
	const int_t size(){return m_size;}
	const int_t null(){return m_null;}
};

#endif

