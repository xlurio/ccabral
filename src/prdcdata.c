#include <stdio.h>
#include <ccabral/_auxds.h>
#include <cbarroso/dblylnkdlist.h>

ProductionData *ProductionData__new(
    CCB_production_t id,
    CCB_nonterminal_t leftHandNonTerminal,
    CCB_grammar_t firstRightHandGrammar,
    CCB_grammartype_t firstRightHandGrammarType)
{
    ProductionData *production = malloc(sizeof(ProductionData));
    if (production == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for production\n");
        return NULL;
    }

    production->id = id;
    production->leftHand = leftHandNonTerminal;

    GrammarData *grammarData = malloc(sizeof(GrammarData));

    if (grammarData == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for GrammarData\n");
        free(production);
        return NULL;
    }

    grammarData->id = firstRightHandGrammar;
    grammarData->type = firstRightHandGrammarType;
    production->rightHandHead = DoublyLinkedListNode__new(
        grammarData,
        sizeof(GrammarData *));

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
    CCB_grammar_t rightHandGrammar,
    CCB_grammartype_t rightHandGrammarType)
{
    GrammarData *grammarData = malloc(sizeof(GrammarData));

    if (grammarData == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for GrammarData\n");
        return CCB_ERROR;
    }

    grammarData->id = CCB_START_NT;
    grammarData->type = CCB_NONTERMINAL_GT;

    if (DoublyLinkedListNode__insertAtTail(
            self->rightHandTail,
            grammarData, sizeof(GrammarData *)) <= CCB_ERROR)
    {
        fprintf(stderr, "Failed to insert right hand grammar\n");
        free(grammarData);
        return CCB_ERROR;
    }
    self->rightHandTail = self->rightHandTail->next;

    return CCB_SUCCESS;
}

void ProductionData__del(ProductionData *self)
{
    DoublyLinkedListNode__del(self->rightHandHead);
    free(self);
}
