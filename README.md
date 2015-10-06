# lockfree
lock free data struct, thread safe

queue.hpp : c++ template for lock free ring queue

queue_int.hpp : c++ queue, lock free ring queue for int only

lfrq_queue.c lfrq_queue.h : c functions and data struct for lock free ring queue

lfrq_queue_int.c lfrq_queue_int.h : c functions and data struct for lock free ring queue, int only

lfrq.h : similar to lfrq_queue.c, replace all the functions with macros
