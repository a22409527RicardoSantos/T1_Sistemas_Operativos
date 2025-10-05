#ifndef SFJ_H
#define SFJ_H

#include "queue.h"

uint32_t remaining_ms(const pcb_t *p);
pcb_t* dequeue_shortest_remaining(queue_t *q);
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);

#endif //SFJ_H
