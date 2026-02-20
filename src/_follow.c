#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/hashmap.h>
#include <cbarroso/sngllnkdlist.h>
#include <ccabral/_frstfllw.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/constants.h>

static int8_t sGetFirstOfSequenceIfNonTerminal(
    FirstFollow *first,
    CCB_terminal_t *kSeq,
    size_t *kSeqSizePtr,
    GrammarData *currGrammar,
    bool *foundNonEmpty,
    uint8_t k)
{
    FirstFollowEntry *firstEntry = first[currGrammar->id];

    for (SinglyLinkedListNode *firstNode = firstEntry->entriesHead;
         firstNode != NULL && *kSeqSizePtr < k;
         firstNode = firstNode->next)
    {
        CCB_terminal_t *terminals = (CCB_terminal_t *)firstNode->value;
        size_t numTerminals = firstNode->valueSize / sizeof(CCB_terminal_t);

        if (numTerminals > 0 && terminals[0] != CCB_EMPTY_STRING_TR)
        {
            for (size_t i = 0; i < numTerminals && *kSeqSizePtr < k; i++)
            {
                kSeq[*kSeqSizePtr] = terminals[i];
                (*kSeqSizePtr)++;
            }
            *foundNonEmpty = true;
        }
    }
}

static int8_t sGetFirstOfSequence(
    FirstFollow *first,
    DoublyLinkedListNode *startNode,
    CCB_terminal_t *kSeq,
    size_t *kSeqSizePtr,
    uint8_t k,
    uint8_t *canDeriveEmpty)
{
    *canDeriveEmpty = 1;

    for (DoublyLinkedListNode *currNode = startNode;
         currNode != NULL && *kSeqSizePtr < k;
         currNode = currNode->next)
    {
        GrammarData *currGrammar = currNode->value;

        if (currGrammar->type == CCB_TERMINAL_GT)
        {
            if (currGrammar->id != CCB_EMPTY_STRING_TR)
            {
                kSeq[*kSeqSizePtr] = currGrammar->id;
                (*kSeqSizePtr)++;
                *canDeriveEmpty = 0;
            }
            break;
        }
        else if (currGrammar->type == CCB_NONTERMINAL_GT)
        {
            bool foundNonEmpty = false;

            if (sGetFirstOfSequenceIfNonTerminal(
                    first,
                    kSeq,
                    kSeqSizePtr,
                    currGrammar,
                    &foundNonEmpty,
                    k) <= CCB_ERROR)
            {
                return CCB_ERROR;
            }

            if (foundNonEmpty)
            {
                *canDeriveEmpty = 0;
                break;
            }
        }
    }

    return CCB_SUCCESS;
}

static int8_t sFollow_handleCanDeriveEmpty(
    FirstFollow *self,
    CCB_nonterminal_t leftNonterminal,
    CCB_nonterminal_t currNonterminal)
{
    FirstFollowEntry *leftFollowEntry = self[leftNonterminal];

    for (SinglyLinkedListNode *followNode = leftFollowEntry->entriesHead;
         followNode != NULL;
         followNode = followNode->next)
    {
        if (FirstFollowEntry__insert(
                self[currNonterminal],
                followNode->value,
                followNode->valueSize) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

static int8_t sFollow__processProduction(
    FirstFollow *self,
    FirstFollow *first,
    DoublyLinkedListNode *currNode,
    CCB_nonterminal_t leftNonterminal,
    uint8_t k)
{
    GrammarData *currGrammar = currNode->value;

    if (currGrammar->type == CCB_NONTERMINAL_GT)
    {
        CCB_nonterminal_t currNonterminal = currGrammar->id;
        CCB_terminal_t kSeq[k];
        size_t kSeqSize = 0;
        uint8_t canDeriveEmpty = 0;

        memset(kSeq, 0x0, k * sizeof(CCB_terminal_t));

        if (currNode->next != NULL)
        {
            sGetFirstOfSequence(
                first,
                currNode->next,
                kSeq,
                &kSeqSize,
                k,
                &canDeriveEmpty);

            if (kSeqSize > 0)
            {
                if (FirstFollowEntry__insert(
                        self[currNonterminal],
                        kSeq,
                        kSeqSize * sizeof(CCB_terminal_t)) <= CCB_ERROR)
                {
                    return CCB_ERROR;
                }
            }
        }
        else
        {
            canDeriveEmpty = 1;
        }

        if (canDeriveEmpty && sFollow_handleCanDeriveEmpty(
                                  self,
                                  leftNonterminal,
                                  currNonterminal) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

static int8_t sFollow__processProductionNode(
    FirstFollow *self,
    FirstFollow *first,
    ProductionData *production,
    uint8_t k)
{
    CCB_nonterminal_t leftNonterminal = production->leftHand;

    for (DoublyLinkedListNode *currNode = production->rightHandHead;
         currNode != NULL;
         currNode = currNode->next)
    {
        if (sFollow__processProduction(
                self,
                first,
                currNode,
                leftNonterminal,
                k) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

int8_t sFollow__processProductionsEntry(
    FirstFollow *self,
    FirstFollow *first,
    HashMapEntry **productionEntries,
    uint8_t entryIdx,
    uint8_t k)
{
    ProductionsHashMapEntry *entry = (ProductionsHashMapEntry *)productionEntries[entryIdx]->value;

    for (DoublyLinkedListNode *prodNode = entry->head;
         prodNode != NULL;
         prodNode = prodNode->next)
    {
        ProductionData *production = (ProductionData *)prodNode->value;

        if (sFollow__processProductionNode(self, first, production, k) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

int8_t sFollow__processNonTerminal(
    FirstFollow *self,
    FirstFollow *first,
    ProductionsHashMap *productions,
    HashMapEntry **productionEntries,
    uint8_t k)
{
    for (uint8_t entryIdx = 0; entryIdx < productions->nentries; entryIdx++)
    {
        if (sFollow__processProductionsEntry(
                self,
                first,
                productionEntries,
                entryIdx,
                k) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

FirstFollow *Follow__new(
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    FirstFollow *follow = malloc(sizeof(FirstFollowEntry *) * CCB_NUM_OF_NONTERMINALS);

    if (follow == NULL)
    {
        return NULL;
    }

    for (uint8_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        follow[i] = FirstFollowEntry__new();
        if (follow[i] == NULL)
        {
            fprintf(stderr, "Failed to allocate follow entry for nonterminal %d\n", i);
            FirstFollow__del(follow);
            return NULL;
        }
    }

    CCB_terminal_t endOfText = CCB_END_OF_TEXT_TR;
    if (FirstFollowEntry__insert(
            follow[CCB_START_NT],
            &endOfText,
            sizeof(CCB_terminal_t)) <= CCB_ERROR)
    {
        fprintf(stderr, "Failed to insert end-of-text into start follow set\n");
        FirstFollow__del(follow);
        return NULL;
    }

    HashMapEntry **productionEntries = HashMap__getEntries(productions);

    for (uint8_t nonterminal = 0; nonterminal < CCB_NUM_OF_NONTERMINALS; nonterminal++)
    {
        if (sFollow__processNonTerminal(
                follow,
                first,
                productions,
                productionEntries,
                k) <= CCB_ERROR)
        {
            fprintf(stderr, "Failed to process productions NT%d\n", nonterminal);
            return NULL;
        }
    }

    return follow;
}
