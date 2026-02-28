#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/hashmap.h>
#include <cbarroso/sngllnkdlist.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/_frstfllw.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/constants.h>

static int8_t sInsertNewTerminalFromCurrGrammar(
    DoublyLinkedListNode *currRightHandNode,
    CCB_terminal_t *kSeq,
    size_t *kSeqSizePtr,
    ProductionsHashMap *productions,
    DoublyLinkedListNode **nextNodePtr)
{
    GrammarData *currGrammar = currRightHandNode->value;

    if (currGrammar->type == CCB_TERMINAL_GT)
    {
        kSeq[*kSeqSizePtr] = currGrammar->id;
        (*kSeqSizePtr)++;
        *nextNodePtr = currRightHandNode->next;
    }

    if (currGrammar->type == CCB_NONTERMINAL_GT)
    {
        ProductionsHashMapEntry *productionEntry;

        if (HashMap__getItem(
                productions,
                &(currGrammar->id),
                sizeof(CCB_nonterminal_t),
                (void **)&productionEntry) <= CBR_ERROR)
        {
            fprintf(
                stderr,
                "Failed to get production for nonterminal NT%d",
                currGrammar->id);
            return CCB_ERROR;
        }

        for (
            DoublyLinkedListNode *currSubProductionEntry = productionEntry->head;
            currSubProductionEntry != NULL;
            currSubProductionEntry = currSubProductionEntry->next)
        {
            ProductionData *currSubProduction = (ProductionData *)currSubProductionEntry->value;
            GrammarData *currSubProdFirstGrammar = currSubProduction->rightHandHead->value;

            if (!GrammarData__isEmptyString(currSubProdFirstGrammar))
            {
                *nextNodePtr = currSubProduction->rightHandHead;
                break;
            }
        }
    }

    return CCB_SUCCESS;
}

static int8_t sInsertTerminalsFromProd(
    ProductionsHashMap *productions,
    FirstFollowEntry *entry,
    ProductionData *currProduction,
    uint8_t k)
{
    size_t kSeqSize = 0;
    CCB_terminal_t kSeq[k];

    memset(kSeq, 0x0, k * sizeof(CCB_terminal_t));

    DoublyLinkedListNode *currRightHandNode = currProduction->rightHandHead;
    
    while (currRightHandNode != NULL && kSeqSize < k)
    {
        DoublyLinkedListNode *nextNode = NULL;
        
        if (sInsertNewTerminalFromCurrGrammar(
                currRightHandNode,
                kSeq,
                &kSeqSize,
                productions,
                &nextNode) <= CCB_ERROR)
        {
            GrammarData *currGrammar = currRightHandNode->value;
            fprintf(
                stderr,
                "Failed to process grammar {id=%d, type=%d}\n",
                currGrammar->id,
                currGrammar->type);

            return CCB_ERROR;
        }

        currRightHandNode = nextNode;
    }

    if (FirstFollowEntry__insert(
            entry,
            kSeq,
            kSeqSize * sizeof(CCB_terminal_t)) <= CCB_ERROR)
    {
        fprintf(stderr, "Failed to insert terminals into first/follow entry\n");
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

static int8_t sFirst__insertTerminalsFromNonterminal(
    FirstFollow *self,
    ProductionsHashMap *productions,
    HashMapEntry *currMapEntry,
    uint8_t k)
{
    ProductionsHashMap *productionsCopy = ProductionsHashMap__deepCopy(productions);

    if (productionsCopy == NULL)
    {
        fprintf(stderr, "Failed to copy productions\n");
        return CCB_ERROR;
    }

    CCB_nonterminal_t nonterminal = *(CCB_nonterminal_t *)(currMapEntry->key);

    self[nonterminal] = FirstFollowEntry__new();

    if (self[nonterminal] == NULL)
    {
        return CCB_ERROR;
    }

    DoublyLinkedListNode *currNode = ((ProductionsHashMapEntry *)currMapEntry //
                                          ->value)
                                         ->head;

    for (; currNode != NULL; currNode = currNode->next)
    {
        if (sInsertTerminalsFromProd(
                productionsCopy,
                self[nonterminal],
                currNode->value,
                k) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
    }

    ProductionsHashMap__del(productionsCopy);

    return CCB_SUCCESS;
}

FirstFollow *First__new(ProductionsHashMap *productions, uint8_t k)
{
    CCB_nonterminal_t visited[CCB_NUM_OF_NONTERMINALS];
    FirstFollow *first = calloc(sizeof(FirstFollowEntry *), CCB_NUM_OF_NONTERMINALS);

    if (first == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for first/follow table\n");
        return NULL;
    }

    HashMapEntry **productionEntries = HashMap__getEntries(productions);

    for (
        uint8_t productionEntriesIdx = 0;
        productionEntriesIdx < productions->nentries;
        productionEntriesIdx++)
    {
        if (sFirst__insertTerminalsFromNonterminal(
                first,
                productions,
                productionEntries[productionEntriesIdx],
                k) <= CCB_ERROR)
        {
            fprintf(stderr, "Failed to process production\n");
            FirstFollow__del(first);
            return NULL;
        }
    }

    ClnLogger *logger = ClnLogger__new("_first.c", 11);

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '_first.c'");

        FirstFollow__del(first);
        return NULL;
    }

    char *firstStr = FirstFollow__str(first, k);

    if (firstStr == NULL)
    {
        fprintf(stderr, "Failed to strigify FIRST table");

        FirstFollow__del(first);
        ClnLogger__del(logger);
        return NULL;
    }

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "FIRST:\n%s",
        9 + sizeof(firstStr),
        firstStr);

    free(firstStr);
    ClnLogger__del(logger);

    return first;
}
