#include "queue.h"

QUEUE* create_queue() {
	QUEUE* q = (QUEUE*)malloc(sizeof(QUEUE));
	memset(q->data, 0, sizeof(DWORD) * QUEUE_MAX_SIZE);
	q->head = -1;
	q->tail = -1;
	q->semaphore = CreateSemaphore(NULL, 0, QUEUE_MAX_SIZE, NULL);
	return q;
}

void append_tail(QUEUE* q, DWORD item) {
	q->data[QUEUE_NEXT(q->tail)] = item;
	ReleaseSemaphore(q->semaphore, 1, NULL);
}

int wait_one(QUEUE* q, DWORD timeout) {
	if (WaitForSingleObject(q->semaphore, timeout) == WAIT_OBJECT_0) return 1;
	return 0;
}

DWORD get_one(QUEUE* q) {
	return q->data[QUEUE_NEXT(q->head)];
}

void destroy_queue(QUEUE* q) {
	CloseHandle(q->semaphore);
	free(q);
}