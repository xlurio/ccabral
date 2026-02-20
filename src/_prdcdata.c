#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/constants.h>

ProductionData *ProductionData__deepCopy(ProductionData *self)
{
    ProductionData *copy = malloc(sizeof(ProductionData));
    if (copy == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ProductionData copy\n");
        return NULL;
    }

    copy->id = self->id;
    copy->leftHand = self->leftHand;

    copy->rightHandHead = NULL;
    copy->rightHandTail = NULL;

    if (self->rightHandHead != NULL)
    {
        DoublyLinkedListNode *current = self->rightHandHead;

        copy->rightHandHead = DoublyLinkedListNode__new(
            current->value,
            current->valueSize);
        if (copy->rightHandHead == NULL)
        {
            free(copy);
            return NULL;
        }

        copy->rightHandTail = copy->rightHandHead;
        current = current->next;

        while (current != NULL)
        {
            if (DoublyLinkedListNode__insertAtTail(
                    copy->rightHandHead,
                    current->value,
                    current->valueSize) == CBR_ERROR)
            {
                DoublyLinkedListNode__del(copy->rightHandHead);
                free(copy);
                return NULL;
            }

            copy->rightHandTail = copy->rightHandTail->next;
            current = current->next;
        }
    }

    return copy;
}
