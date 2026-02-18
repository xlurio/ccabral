#ifndef CCABRAL__AUXDS_H
#define CCABRAL__AUXDS_H

#include <stdlib.h>
#include "_types.h"
#include "constants.h"
#include <cbarroso/dblylnkdlist.h>
#include <cbarroso/sngllnkdlist.h>

typedef struct FirstFollowEntry
{
    SinglyLinkedListNode *entriesHead;
    SinglyLinkedListNode *entriesTail;
} FirstFollowEntry;

typedef struct GrammarData
{
    CCB_grammar_t id;
    CCB_grammartype_t type;
} GrammarData;

typedef struct ProductionData
{
    CCB_production_t id;
    CCB_nonterminal_t leftHand;
    DoublyLinkedListNode *rightHandHead;
    DoublyLinkedListNode *rightHandTail;
} ProductionData;

FirstFollowEntry **buildFirst(ProductionData **productions);
FirstFollowEntry **buildFollow(ProductionData **productions, FirstFollowEntry **first);

static inline void destroyFirstFollow(FirstFollowEntry **firstFollow)
{
    for (uint8_t firstFollowIndex = 0;
         firstFollowIndex < CCB_NUM_OF_NONTERMINALS;
         firstFollowIndex++)
    {
        if (firstFollow[firstFollowIndex]->entriesHead != NULL)
        {
            SinglyLinkedListNode__del(firstFollow[firstFollowIndex]->entriesHead);
        }
        free(firstFollow[firstFollowIndex]);
    }

    free(firstFollow);
}

CCB_production_t **buildPrdcPrsnTbl(ProductionData **productions);
void destroyPrdtPrsnTable(CCB_production_t **prdtPrsnTable);

#endif
