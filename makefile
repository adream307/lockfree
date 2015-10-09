all: queue_test queue_int_test lfrq_queue_test lfrq_queue_int_test lfrq_test

queue_test:queue.hpp queue_test.cpp
	g++ queue_test.cpp -o queue_test -lpthread
queue_int_test:queue_int.hpp queue_int_test.cpp
	g++ queue_int_test.cpp -o queue_int_test -lpthread
lfrq_queue_test:lfrq_queue.c lfrq_queue_test.c lfrq_queue.h
	gcc lfrq_queue.c lfrq_queue_test.c -o lfrq_queue_test -lpthread
lfrq_queue_int_test:lfrq_queue_int.c lfrq_queue_int.h lfrq_queue_int_test.c
	gcc lfrq_queue_int.c lfrq_queue_int_test.c -o lfrq_queue_int_test -lpthread
lfrq_test:lfrq.h lfrq_test.c
	gcc lfrq_test.c -o lfrq_test -lpthread
test:
	./queue_test
	./queue_int_test
	./lfrq_queue_test
	./lfrq_queue_int_test
	./lfrq_test
clean:
	rm queue_test queue_int_test lfrq_queue_test lfrq_queue_int_test lfrq_test
