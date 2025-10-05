#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>
#include "config.h"


// Funcao recebe current_time_ms que é o tempo atual do sistema (nao relevante para o FIFO)
// Recebe a lista de processos (Fila)
// Recebe o processo que esta a ser executado ou null


void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    // Se existe um processo a correr (cpu_task != null)
    if (*cpu_task) {

        if ((*cpu_task)->ellapsed_time_ms == 0) {
            printf("Started at %d ms\n",(int)current_time_ms);
        }

        // O tempo de execuçao do processo é o seu tempo anterior mais o tempo que passou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        // Atualizo o tempo passado do time
        (*cpu_task)->slice_start_ms   += TICKS_MS;

        // Se o tempo que o processo executou (ellapsed_time_ms) é >= ao tempo que precisava (time_ms)
        // -> o processo terminou
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

            // Se nao terminou mas ja acabou o tempo do slice
        } else if ((*cpu_task)->slice_start_ms >= SLICE_MS) {

            // Reseto time slice
            (*cpu_task)->slice_start_ms = 0;
            // Coloco no fim da fila
            enqueue_pcb(rq, *cpu_task);
            // CPU esta livre
            *cpu_task = NULL;
        }
    }

    // Se nao existe processo a correr (cpu livre)
    if (*cpu_task == NULL) {
      // O proximo a executar é o primeiro da fila (head) (dequeue - retirado da lista)
        *cpu_task = dequeue_pcb(rq);
        if (*cpu_task) (*cpu_task)->slice_start_ms = 0;
    }
}