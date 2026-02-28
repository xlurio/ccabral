#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/prdcdata.h>
#include <ccabral/constants.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>

ProductionData *ProductionData__deepCopy(ProductionData *self)
{
    const char *loggerName = "ProductionData__deepCopy";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger 'ProductionData__deepCopy'");
        return NULL;
    }

    char *originProdDataStr = ProductionData__str(self);

    if (originProdDataStr == NULL)
    {
        fprintf(
            stderr,
            "Failed to strigify `originProdDataStr` in `ProductionData__deepCopy` for P%d\n",
            self->id);
        ClnLogger__del(logger);
        return NULL;
    }

    ProductionData *copy = malloc(sizeof(ProductionData));
    if (copy == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ProductionData copy\n");
        free(originProdDataStr);
        ClnLogger__del(logger);
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

            char *grammarDataStr = GrammarData__str(current->value);

            if (grammarDataStr == NULL)
            {
                fprintf(
                    stderr,
                    "Failed to strigify grammar while copying %s\n",
                    originProdDataStr);
                return NULL;
            }

            fprintf(
                stderr,
                "Failed to create linked list for the right hand %s copy and insert the %s grammar\n",
                originProdDataStr,
                grammarDataStr);

            free(grammarDataStr);
            free(originProdDataStr);
            ClnLogger__del(logger);
            free(copy);
            return NULL;
        }

        copy->rightHandTail = copy->rightHandHead;
        current = current->next;

        while (current != NULL)
        {
            if (DoublyLinkedListNode__insertAtTail(
                    copy->rightHandTail,
                    current->value,
                    current->valueSize) <= CBR_ERROR)
            {
                char *grammarDataStr;

                if (current->valueSize == sizeof(GrammarData))
                {
                    grammarDataStr = GrammarData__str(current->value);

                    if (grammarDataStr == NULL)
                    {
                        fprintf(
                            stderr,
                            "Failed to strigify grammar while copying %s\n",
                            originProdDataStr);
                        return NULL;
                    }
                }
                else
                    grammarDataStr = "{corrupted}";

                fprintf(
                    stderr,
                    "Failed to copy %s from %s to its copy\n",
                    grammarDataStr,
                    originProdDataStr);

                if (current->valueSize == sizeof(GrammarData))
                    free(grammarDataStr);

                free(originProdDataStr);
                ClnLogger__del(logger);
                DoublyLinkedListNode__del(copy->rightHandHead);
                free(copy);
                return NULL;
            }

            copy->rightHandTail = copy->rightHandTail->next;
            current = current->next;
        }
    }

    free(originProdDataStr);
    ClnLogger__del(logger);

    return copy;
}
