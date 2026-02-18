#include <stdlib.h>
#include <stdio.h>
#include <ccabral/_types.h>
#include <ccabral/constants.h>
#include <ccabral/tknsq.h>

int8_t TokenQueue__enqueue(TokenQueue *self, CCB_terminal_t newValue)
{
    CCB_terminal_t *value = malloc(sizeof(CCB_terminal_t));
    if (value == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for terminal value\n");
        return CCB_ERROR;
    }

    *value = newValue;

    if (Queue__enqueue(self, &value, sizeof(CCB_terminal_t*)) == -1)
    {
        fprintf(stderr, "Failed to enqueue terminal\n");
        free(value);
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

int8_t TokenQueue__dequeue(TokenQueue *self, CCB_terminal_t *valueAddress)
{
    void *value;
    if (Queue__dequeue(self, &value) == -1)
    {
        fprintf(stderr, "Empty queue\n");
        return CCB_ERROR;
    }

    CCB_terminal_t *ptrValue = *(CCB_terminal_t **)value;
    *valueAddress = *ptrValue;
    free(ptrValue);
    free(value);

    return CCB_SUCCESS;
}
