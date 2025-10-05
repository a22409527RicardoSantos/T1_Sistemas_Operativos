#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"

void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);
pcb_t* dequeue_higher_priority_elem(queue_t *rq);

#endif //MLFQ_H
