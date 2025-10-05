#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>
#include "config.h"



// Funcao recebe current_time_ms que é o tempo atual do sistema (nao relevante para o FIFO)
// Recebe a lista de processos (Fila)
// Recebe o processo que esta a ser executado ou null

uint32_t priority_boost_counter = 0;

void priority_boost(queue_t *rq) {
    if (!rq || !rq->head) {
        return;
    }
    queue_elem_t *it = rq->head;
    while (it) {
        it->pcb->priority = 0;
        it->pcb->slice_start_ms = 0;
        it = it->next;
    }
}

int exists_higher_prio_task(pcb_t *running_task, queue_t *rq) {
    // Verifica se a lista é nula ou vazia
    if (!rq || !rq->head || !running_task) return 0;

    queue_elem_t *it = rq->head;
    while (it) {
        if (it->pcb->priority < running_task->priority) {
            return 1;
        }
        it = it->next;
    }

    return 0;
}

pcb_t* dequeue_higher_priority_elem(queue_t *rq) {
    // Se a lista é null ou está vazia
    if (!rq || !rq->head) return NULL;

    // O elemento atual e o que tem maior prio
    queue_elem_t *it = rq->head, *max_prio_elem = rq->head;
    uint32_t max_prio = rq->head->pcb->priority;

    // Percorro os restantes elementos enquanto it != null (existir)
    while (it) {
        // p é a priority deste "it"
        uint32_t p = it->pcb->priority;

        // Se p é mair
        if (p < max_prio) {
            max_prio = p;      // p é o novo maximo prio
            max_prio_elem = it;    // elemento com maior prioridade é este
        }
        // Passa ao proximo da lista
        it = it->next;
    }

    // Depois de descobrir o primeiro maios prioritario removo-o da lista (função dada)
    queue_elem_t *removed = remove_queue_elem(rq, max_prio_elem);

    // guarda o processo removido em *best
    pcb_t *next = removed->pcb;

    // Liberto a memoria do no, quero devolver o processo, nao a estrututa
    free(removed);

    // Posso devolver a estrutura
    return next;
}


void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Incremento contador MLFQ_PRIORITY_BOOST_MS
    priority_boost_counter += TICKS_MS;

    // Se existe um processo a correr
    if (*cpu_task) {

        if ((*cpu_task)->ellapsed_time_ms == 0) {
            printf("Started at %d ms\n",(int)current_time_ms);
        }

        // O tempo de execuçao do processo é o seu tempo anterior mais o tempo que passou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        // Atualizo o tempo passado do time
        (*cpu_task)->slice_start_ms   += TICKS_MS;



        // Se processo terminou
        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            // Se ja acabou posso resetar o time_slice
            (*cpu_task)->slice_start_ms = 0;
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



            // Se esgotou o quantum
        } else if ((*cpu_task)->slice_start_ms >= prio_quantum((*cpu_task)->priority)) {
            // Reseto time slice
            (*cpu_task)->slice_start_ms = 0;
            // Baixo a prioridade
            decrease_priority(*cpu_task);
            // Coloco no fim da fila
            enqueue_pcb(rq, *cpu_task);
            // CPU esta livre
            *cpu_task = NULL;
        }
    }

    // Se existe um processo com maior prioridade disponivel (falta condiçao)
    if (*cpu_task && exists_higher_prio_task(*cpu_task, rq)) {
        enqueue_pcb(rq, *cpu_task);
        *cpu_task = NULL;
    }

    if (priority_boost_counter >= MLFQ_PRIORITY_BOOST_MS) {
        // Coloco todos os processos com prioridade maxima
        priority_boost(rq);
        priority_boost_counter = 0;
    }

    if (*cpu_task == NULL) {
        // Escolho proceso com maior prioridade da ready queue
        pcb_t *next = dequeue_higher_priority_elem(rq);
        if (next) {
            next->slice_start_ms = 0;
            *cpu_task = next;
        }
    }
}
