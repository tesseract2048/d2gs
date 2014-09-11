#ifndef INCLUDED_QUEUE_H
#define INCLUDED_QUEUE_H

#include <Windows.h>

#define QUEUE_MAX_SIZE 1024
#define QUEUE_NEXT(x) (InterlockedIncrement((unsigned int*)&(x)) % QUEUE_MAX_SIZE)

typedef struct {
	DWORD	data[QUEUE_MAX_SIZE];
	int		head;
	int		tail;
	HANDLE	semaphore;
} QUEUE, *PQUEUE, *LPQUEUE;

QUEUE* create_queue();
void append_tail(QUEUE* q, DWORD item);
int wait_one(QUEUE* q, DWORD timeout);
DWORD get_one(QUEUE* q);
void destroy_queue(QUEUE* q);

#endif