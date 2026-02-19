#ifndef CCABRAL__PRODUCTION_DATA_H
#define CCABRAL__PRODUCTION_DATA_H

#include <cbarroso/dblylnkdlist.h>
#include <ccabral/types.h>

typedef struct ProductionData
{
    CCB_production_t id;
    CCB_nonterminal_t leftHand;

    /* Head of the linked list of grammars in the right hand side of the production */
    DoublyLinkedListNode *rightHandHead;

    /* Head of the linked list of grammars in the right hand side of the production */
    DoublyLinkedListNode *rightHandTail;
} ProductionData;

#endif
