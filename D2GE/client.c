#include "client.h"
#include "queue.h"

static QUEUE* client_queue;

void init_client() {
	client_queue = create_queue();
}

void push_client(SOCKET s) {
	append_tail(client_queue, (DWORD)s);
}

int wait_client(DWORD timeout) {
	return wait_one(client_queue, timeout);
}

SOCKET get_client() {
	return get_one(client_queue);
}

void destroy_client() {
	destroy_queue(client_queue);
}
