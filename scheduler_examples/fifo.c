#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>
#include "debug.h"

void fifo_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {

    // Se existe um processo a correr
    if (*cpu_task) {

        if ((*cpu_task)->ellapsed_time_ms == 0) {
            printf("Started at %d ms\n",(int)current_time_ms);
        }

        // O tempo de execuçao do processo é o seu tempo anterior mais o tempo que passou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        // Se o processo terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {

            // Mensagem
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            // Apago o processo, que terminou e cpu fica sem processo
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }

    // Se nao existe processo no cpu
    if (*cpu_task == NULL) {
      // O proximo a executar é o primeiro da fila, dequeue do que está no head
        *cpu_task = dequeue_pcb(rq);
    }
}