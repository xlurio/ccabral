#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/dblylnkdlist.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/constants.h>
#include <ccabral/types.h>

ProductionData *ProductionData__new(
    CCB_production_t id,
    CCB_nonterminal_t leftHandNonTerminal,
    CCB_terminal_t firstRightHandGrammar)
{
    if (!isGrammarValid(leftHandNonTerminal, CCB_NONTERMINAL_GT))
    {
        return NULL;
    }

    if (!isGrammarValid(firstRightHandGrammar, CCB_TERMINAL_GT))
    {
        return NULL;
    }

    ProductionData *production = calloc(1, sizeof(ProductionData));
    if (production == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for production\n");
        return NULL;
    }

    production->id = id;
    production->leftHand = leftHandNonTerminal;

    GrammarData *grammarData = calloc(1, sizeof(GrammarData));

    if (grammarData == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for GrammarData\n");
        free(production);
        return NULL;
    }

    grammarData->id = firstRightHandGrammar;
    grammarData->type = CCB_TERMINAL_GT;
    production->rightHandHead = DoublyLinkedListNode__new(
        grammarData,
        sizeof(GrammarData));

    if (production->rightHandHead == NULL)
    {
        fprintf(stderr, "Failed to create production right hand doubly linked list\n");
        free(grammarData);
        free(production);
        return NULL;
    }
    production->rightHandTail = production->rightHandHead;

    return production;
}

int8_t ProductionData__insertRightHandGrammar(
    ProductionData *self,
    CCB_nonterminal_t rightHandGrammar)
{
    if (!isGrammarValid(rightHandGrammar, CCB_NONTERMINAL_GT))
    {
        return CCB_ERROR;
    }

    GrammarData *grammarData = calloc(1, sizeof(GrammarData));

    if (grammarData == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for GrammarData\n");
        return CCB_ERROR;
    }

    grammarData->id = rightHandGrammar;
    grammarData->type = CCB_NONTERMINAL_GT;

    if (DoublyLinkedListNode__insertAtTail(
            self->rightHandTail,
            grammarData, sizeof(GrammarData)) <= CCB_ERROR)
    {
        fprintf(stderr, "Failed to insert right hand grammar\n");
        free(grammarData);
        return CCB_ERROR;
    }
    self->rightHandTail = self->rightHandTail->next;

    return CCB_SUCCESS;
}

static char *sProductionData__rightHandStr(ProductionData *self)
{
    assert(self != NULL);

    ssize_t maxLength = 128;
    char *rightHandStr = calloc(maxLength, sizeof(char));

    if (rightHandStr == NULL)
    {
        fprintf(
            stderr,
            "Failed to allocate memory to stringify P%d right hand side",
            self->id);
        return NULL;
    }

    for (
        DoublyLinkedListNode *currentGrammarNode = self->rightHandHead;
        currentGrammarNode != NULL || maxLength <= 0;
        currentGrammarNode = currentGrammarNode->next)
    {
        if (currentGrammarNode->valueSize > sizeof(GrammarData))
        {
            strncat(rightHandStr, "corrupted", maxLength);
            maxLength -= 10;
            break;
        }

        char *currGrammarStr = GrammarData__str((GrammarData *) //
                                                currentGrammarNode->value);

        if (currGrammarStr == NULL)
        {
            fprintf(stderr, "Failed to stringify grammar\n");
            return NULL;
        }

        strncat(rightHandStr, currGrammarStr, maxLength);
        maxLength -= strlen(currGrammarStr);

        if (maxLength >= 4 && currentGrammarNode->next != NULL)
        {
            strncat(rightHandStr, " -> ", maxLength);
            maxLength -= 4;
        }
    }

    return rightHandStr;
}

char *ProductionData__str(ProductionData *self)
{
    const ssize_t maxLength = 255;
    char *prodDataStr = calloc(maxLength, sizeof(char));

    if (prodDataStr == NULL)
    {
        fprintf(
            stderr,
            "Failed to allocate memory for the stringified P%d\n",
            self->id);
        return NULL;
    }

    char *rightHandStr = sProductionData__rightHandStr(self);

    if (rightHandStr == NULL)
    {
        fprintf(
            stderr,
            "Failed to strigify production P%d right hand side\n",
            self->id);
        return NULL;
    }

    snprintf(
        prodDataStr,
        maxLength,
        "ProductionData {id=P%d, leftHand=NT%d, rightHand={%s}}",
        self->id,
        self->leftHand,
        rightHandStr);

    return prodDataStr;
}

void ProductionData__del(ProductionData *self)
{
    DoublyLinkedListNode__del(self->rightHandHead);
    free(self);
}
