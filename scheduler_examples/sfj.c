#include "queue.h"
#include "msg.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

uint32_t remaining_ms(const pcb_t *p) {
    if (p->time_ms > p->ellapsed_time_ms)
        return p->time_ms - p->ellapsed_time_ms;
    else
        return 0;
}

pcb_t* dequeue_shortest_remaining(queue_t *q) {
  // Se a lista é null ou está vazia
    if (!q || !q->head) return NULL;

    // O elemento atual e o que tem menor tempo é o head agora
    queue_elem_t *it = q->head, *min_elem = q->head;
    uint32_t min_rem = remaining_ms(q->head->pcb);

    // Percorro os restantes elementos enquanto it != null (existir)
    while (it) {
      // r é o remaining deste "it"
        uint32_t r = remaining_ms(it->pcb);
        // Se r é menor
        if (r < min_rem) {
            min_rem = r;      // r é o novo minimo
            min_elem = it;    // elemento mais curto é agora este it
        }
        // Pass ao proximo da lista
        it = it->next;
    }

    // Depois de descobrir o mais curto removo-o da lista (função dada)
    queue_elem_t *removed = remove_queue_elem(q, min_elem);

    // guarda o processo removido em *best
    pcb_t *best = removed->pcb;

    // Liberto a memoria do no, quero devolver o processo, nao a estrututa
    free(removed);

    // Posso devolver a estrutura
    return best;
}

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Se existe um processo a correr (cpu_task != null)
    if (*cpu_task) {

        // O tempo de execuçao do processo é o seu tempo anterior mais o tempo que passou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        // Se o tempo que o processo executou (ellapsed_time_ms) é >= ao tempo que precisava (time_ms)
        // -> o processo terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {

            // Crio mensagem para avisar que o processo terminou.
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            // Posso libertar o processo porque já terminou
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }

    // Se nao existe processo a correr (cpu livre)
    if (*cpu_task == NULL) {
    // Criei essa funcao nova para retirar o mais curto da queue
        *cpu_task = dequeue_shortest_remaining(rq);
    }
}
