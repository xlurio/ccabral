#ifndef TOKENQUEUE_H
#define TOKENQUEUE_H

#include "_types.h"
#include <cbarroso/queue.h>

typedef Queue TokenQueue;

int8_t TokenQueue__enqueue(TokenQueue *self, CCB_terminal_t newValue);
int8_t TokenQueue__dequeue(TokenQueue *self, CCB_terminal_t *valueAddress);

#endif
