//
//  MsgQueue
//  GameSrv
//  采用了云风仿erlang的skynet的mq，屏了多播，
//  Created by 麦_Mike on 13-4-15.
//
//

#ifndef __shared__MsgQueue__
#define __shared__MsgQueue__

#include <stdlib.h>
#include <stdint.h>

struct skynet_message {
	uint32_t source;
	int session;
	void * data;
	size_t sz;
};

struct message_queue;

struct message_queue * skynet_globalmq_pop(void);

struct message_queue * skynet_mq_create(uint32_t handle);
void skynet_mq_mark_release(struct message_queue *q);
int skynet_mq_release(struct message_queue *q);
uint32_t skynet_mq_handle(struct message_queue *);

// 0 for success
int skynet_mq_pop(struct message_queue *q, struct skynet_message *message);
void skynet_mq_push(struct message_queue *q, struct skynet_message *message);
void skynet_mq_lock(struct message_queue *q, int session);

void skynet_mq_force_push(struct message_queue *q);
void skynet_mq_pushglobal(struct message_queue *q);

void skynet_mq_init();

#endif /* defined(__GameSrv__MsgQueue__) */
