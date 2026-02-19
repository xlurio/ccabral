#ifndef CCABRAL__FIRST_FOLLOW_H
#define CCABRAL__FIRST_FOLLOW_H

#include <stdlib.h>
#include <cbarroso/sngllnkdlist.h>
#include "prdcdata.h"
#include "types.h"
#include "constants.h"

typedef struct FirstFollowEntry
{
    SinglyLinkedListNode *entriesHead;
    SinglyLinkedListNode *entriesTail;
} FirstFollowEntry;

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

#endif
