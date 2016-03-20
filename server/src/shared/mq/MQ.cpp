
//
//  MsgQueue
//  GameSrv
//
//  Created by 麦_Mike on 13-4-15.
//
//

#include "MQ.h"
//#include "skynet.h"
//#include "skynet_mq.h"
//#include "skynet_handle.h"
//#include "skynet_multicast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_QUEUE_SIZE 64;
#define MAX_GLOBAL_MQ 0x10//0000

struct message_queue {
	uint32_t handle;
	int cap;
	int head;
	int tail;
	int lock;
	int release;
	int lock_session;
	int in_global;
	struct skynet_message *queue;
};

struct global_queue {
	uint32_t head;
	uint32_t tail;
	struct message_queue ** queue;
	bool * flag;
    
};

static struct global_queue *Q = NULL;

#define LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define UNLOCK(q) __sync_lock_release(&(q)->lock);

#define GP(p) ((p) % (MAX_GLOBAL_MQ-1))

static void
skynet_globalmq_push(struct message_queue * queue) {
	struct global_queue *q= Q;
    
	uint32_t tail = GP(__sync_fetch_and_add(&q->tail,1));
    assert(!q->flag[tail]);
	q->queue[tail] = queue;
	__sync_synchronize();
	q->flag[tail] = true;
	__sync_synchronize();
}

struct message_queue *
skynet_globalmq_pop() {
	struct global_queue *q = Q;
	uint32_t head =  q->head;
	uint32_t head_ptr = GP(head);
	if (head_ptr == GP(q->tail)) {
		return NULL;
	}
    
	if(!q->flag[head_ptr]) {
		return NULL;
	}
    
	struct message_queue * mq = q->queue[head_ptr];
	if (!__sync_bool_compare_and_swap(&q->head, head, head+1)) {
		return NULL;
	}
	q->flag[head_ptr] = false;
    
	return mq;
}

struct message_queue *
skynet_mq_create(uint32_t handle) {
	struct message_queue *q = (message_queue*)malloc(sizeof(*q));
	q->handle = handle;
	q->cap = DEFAULT_QUEUE_SIZE;
	q->head = 0;
	q->tail = 0;
	q->lock = 0;
	q->in_global = 1;
	q->release = 0;
	q->lock_session = 0;
	q->queue = (skynet_message*)malloc(sizeof(struct skynet_message) * q->cap);
    
	return q;
}

static void
_release(struct message_queue *q) {
	free(q->queue);
	free(q);
}

uint32_t
skynet_mq_handle(struct message_queue *q) {
	return q->handle;
}


int
skynet_mq_pop(struct message_queue *q, struct skynet_message *message) {
	int ret = 1;
	LOCK(q)
    
	if (q->head != q->tail) {
		*message = q->queue[q->head];
		ret = 0;
		if ( ++ q->head >= q->cap) {
			q->head = 0;
		}
	}
	
	UNLOCK(q)
    
	return ret;
}

static void
expand_queue(struct message_queue *q) {
	struct skynet_message *new_queue = (skynet_message*)malloc(sizeof(struct skynet_message) * q->cap * 2);
	int i;
	for (i=0;i<q->cap;i++) {
		new_queue[i] = q->queue[(q->head + i) % q->cap];
	}
	q->head = 0;
	q->tail = q->cap;
	q->cap *= 2;
	
	free(q->queue);
	q->queue = new_queue;
}

static void
_pushhead(struct message_queue *q, struct skynet_message *message) {
	int head = q->head - 1;
	if (head < 0) {
		head = q->cap - 1;
	}
	if (head == q->tail) {
		expand_queue(q);
		--q->tail;
		head = q->cap - 1;
	}
    
	q->queue[head] = *message;
	q->head = head;
    
	// this api use in push a unlock message, so the in_global flags must be 1 , but the q is not exist in global queue.
	assert(q->in_global);
	skynet_globalmq_push(q);
}

void
skynet_mq_push(struct message_queue *q, struct skynet_message *message) {
	assert(message);
	LOCK(q)
	
    q->queue[q->tail] = *message;
    if (++ q->tail >= q->cap) {
        q->tail = 0;
    }
    
    if (q->head == q->tail) {
        expand_queue(q);
    }
    
	UNLOCK(q)
}

void
skynet_mq_lock(struct message_queue *q, int session) {
	LOCK(q)
	assert(q->lock_session == 0);
	q->lock_session = session;
	UNLOCK(q)
}

void
skynet_mq_init() {
	struct global_queue *q = (global_queue*)malloc(sizeof(*q));
	memset(q,0,sizeof(*q));
	q->queue = (message_queue **)malloc(MAX_GLOBAL_MQ * sizeof(struct message_queue *));
	q->flag = (bool*)malloc(MAX_GLOBAL_MQ * sizeof(bool));
	memset(q->flag, 0, sizeof(bool) * MAX_GLOBAL_MQ);
	Q=q;
}

void
skynet_mq_force_push(struct message_queue * queue) {
	assert(queue->in_global);
	skynet_globalmq_push(queue);
}

void
skynet_mq_pushglobal(struct message_queue *queue) {
	assert(queue->in_global);
	if (queue->lock_session == 0) {
		skynet_globalmq_push(queue);
	}
}

void
skynet_mq_mark_release(struct message_queue *q) {
	assert(q->release == 0);
	q->release = 1;
}

static int
_drop_queue(struct message_queue *q) {
	// todo: send message back to message source
	struct skynet_message msg;
	int s = 0;
	while(!skynet_mq_pop(q, &msg)) {
		++s;
//		int type = msg.sz >> HANDLE_REMOTE_SHIFT;
//		if (type == PTYPE_MULTICAST) {
//			assert((msg.sz & HANDLE_MASK) == 0);
//			skynet_multicast_dispatch((struct skynet_multicast_message *)msg.data, NULL, NULL);
//		} else
        {
			free(msg.data);
		}
	}
	_release(q);
	return s;
}

int
skynet_mq_release(struct message_queue *q) {
	int ret = 0;
	LOCK(q)
	
	if (q->release) {
		UNLOCK(q)
		ret = _drop_queue(q);
	} else {
		skynet_mq_force_push(q);
		UNLOCK(q)
	}
	
	return ret;
}
