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

#define INITIALIZE_PRODUCTION_RIGHT_HAND(grammarId, grammarType, production)                            \
    do                                                                                                  \
    {                                                                                                   \
        GrammarData *firstGrammarData = malloc(sizeof(GrammarData));                                    \
        if (firstGrammarData == NULL)                                                                   \
        {                                                                                               \
            fprintf(stderr, "Failed to allocate memory for GrammarData\n");                             \
            free(production);                                                                           \
            return NULL;                                                                                \
        }                                                                                               \
        firstGrammarData->id = grammarId;                                                               \
        firstGrammarData->type = grammarType;                                                           \
        production->rightHandHead = DoublyLinkedListNode__new(firstGrammarData, sizeof(GrammarData *)); \
        if (production->rightHandHead == NULL)                                                          \
        {                                                                                               \
            fprintf(stderr, "Failed to create right hand doubly linked list for start rule 1");         \
            free(firstGrammarData);                                                                     \
            free(production);                                                                           \
            return NULL;                                                                                \
        }                                                                                               \
        production->rightHandTail = production->rightHandHead;                                          \
    } while (0);

#define INSERT_GRAMMAR_TO_PRODUCTION(grammarId, grammarType, production)                        \
    do                                                                                          \
    {                                                                                           \
        GrammarData *grammarData = malloc(sizeof(GrammarData));                                 \
        if (grammarData == NULL)                                                                \
        {                                                                                       \
            fprintf(stderr, "Failed to allocate memory for GrammarData\n");                     \
            DoublyLinkedListNode__del(production->rightHandHead);                               \
            free(production);                                                                   \
            return NULL;                                                                        \
        }                                                                                       \
        grammarData->id = grammarId;                                                            \
        grammarData->type = grammarType;                                                        \
        if (DoublyLinkedListNode__insertAtTail(                                                 \
                production->rightHandTail,                                                      \
                grammarData,                                                                    \
                sizeof(GrammarData *)) == CCB_ERROR)                                            \
        {                                                                                       \
            fprintf(stderr, "Failed to insert grammar in the right hand side of start rule 1"); \
            DoublyLinkedListNode__del(production->rightHandHead);                               \
            free(production);                                                                   \
            return NULL;                                                                        \
        }                                                                                       \
        production->rightHandTail = production->rightHandTail->next;                            \
    } while (0);

#endif
