//
// Created by Ricardo on 04/10/2025.
//

#ifndef CONFIG_H
#define CONFIG_H

// Definicoes RR
#define SLICE_MS 500

#define MLFQ_PRIORITY_BOOST_MS 1000
#define MIN_PRIORITY 3

#define Q_P0  20
#define Q_P1  40
#define Q_P2  80
#define Q_P3  160

static const uint32_t Q_FOR_PRIO[] = {50,100,200,400};
#define prio_quantum(p) (Q_FOR_PRIO[(p) > 3 ? 3 : (p)])


#endif //CONFIG_H
