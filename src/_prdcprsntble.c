#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <cbarroso/hashmap.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/_frstfllw.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdcprsntble.h>
#include <ccabral/constants.h>

/* Check if the production with the given rule ID yields epsilon */
static bool sProductionYieldsEpsilon(
    ProductionsHashMap *productions,
    CCB_nonterminal_t nonterminal,
    CCB_production_t rule)
{
    ProductionsHashMapEntry *entry;
    if (HashMap__getItem(
            productions,
            &nonterminal,
            sizeof(CCB_nonterminal_t),
            (void **)&entry) <= CBR_ERROR)
    {
        return false;
    }

    DoublyLinkedListNode *currNode = entry->head;
    for (; currNode != NULL; currNode = currNode->next)
    {
        ProductionData *prodData = currNode->value;
        if (prodData->id == rule)
        {
            if (prodData->rightHandHead != NULL)
            {
                GrammarData *firstElement = (GrammarData *)prodData->rightHandHead->value;
                return firstElement->id == CCB_EMPTY_STRING_TR;
            }
            return false;
        }
    }

    return false;
}

static bool sDoesProductionMatchesFirstEntry(
    ProductionData *currProdData,
    CCB_terminal_t *currFirstKSeq,
    uint8_t k,
    ProductionsHashMap *productions,
    FirstFollow *first)
{
    assert(currFirstKSeq[0] != CCB_EMPTY_STRING_TR);

    DoublyLinkedListNode *currNode = currProdData->rightHandHead;
    uint8_t firstEntryIndex = 0;

    while (currNode != NULL //
           && firstEntryIndex < k)
    {
        if (currFirstKSeq[firstEntryIndex] == CCB_EMPTY_STRING_TR)
        {
            return true;
        }

        GrammarData *grammarData = (GrammarData *)currNode->value;

        if (grammarData->type == CCB_TERMINAL_GT)
        {
            if (grammarData->id != currFirstKSeq[firstEntryIndex])
            {
                return false;
            }
            firstEntryIndex++;
            currNode = currNode->next;
        }
        else if (grammarData->type == CCB_NONTERMINAL_GT)
        {
            bool wasFound = false;

            for (
                FirstFollowEntryNode *currFirstEntry = //
                first[grammarData->id]->entriesHead;
                currFirstEntry != NULL;
                currFirstEntry = currFirstEntry->next)
            {
                CCB_terminal_t *currFirstEntryValue = currFirstEntry->value;

                if (currFirstEntryValue[0] == currFirstKSeq[firstEntryIndex])
                {
                    firstEntryIndex++;
                    currNode = currNode->next;
                    wasFound = true;
                    break;
                }
            }

            if (!wasFound)
            {
                return false;
            }
        }
        else
            assert(false);
    }

    return true;
}

int8_t PrdcPrsnTble__getItem(
    PrdcPrsnTble *self,
    CCB_nonterminal_t nonterminal,
    CCB_terminal_t *kSeq,
    uint8_t k,
    CCB_production_t *production)
{
    CCB_production_t *prodPtr = NULL;

    if (HashMap__getItem(
            (HashMap *)self[nonterminal],
            kSeq,
            sizeof(CCB_terminal_t) * k,
            (void **)&prodPtr) <= CBR_ERROR)
    {
        fprintf(stderr, "Failed to get rule for nonterminal NT%d\n", nonterminal);
        return CCB_ERROR;
    }

    if (prodPtr == NULL)
    {
        *production = (CCB_production_t)-1;
    }
    else
    {
        *production = *prodPtr;
    }

    return CCB_SUCCESS;
}

int8_t PrdcPrsnTble__setItem(
    PrdcPrsnTble *self,
    CCB_nonterminal_t nonterminal,
    CCB_terminal_t *kSeq,
    uint8_t k,
    CCB_production_t production)
{
    CCB_production_t *prodPtr = malloc(sizeof(CCB_production_t));
    if (prodPtr == NULL)
    {
        fprintf(stderr, "Failed to allocate production for nonterminal NT%d\n", nonterminal);
        return CCB_ERROR;
    }
    *prodPtr = production;

    if (HashMap__setItem(
            (HashMap *)self[nonterminal],
            kSeq,
            k * sizeof(CCB_terminal_t),
            prodPtr,
            sizeof(CCB_production_t)) <= CBR_ERROR)
    {
        fprintf(stderr, "Failed to set rule for nonterminal NT%d\n", nonterminal);
        free(prodPtr);
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

static int8_t sSetPrdc4NtNTrInPrdcPrsnTble(
    PrdcPrsnTble *prdtPrsnTable,
    CCB_production_t rule,
    CCB_nonterminal_t nonterminal,
    CCB_terminal_t *kSeq,
    ProductionsHashMap *productions,
    uint8_t k)
{
    CCB_production_t existingRule;

    if (PrdcPrsnTble__getItem(
            prdtPrsnTable,
            nonterminal,
            kSeq,
            k,
            &existingRule) <= CCB_ERROR)
    {
        return CCB_ERROR;
    }

    if (existingRule != (CCB_production_t)-1)
    {
        uint8_t existingYieldsEpsilon = sProductionYieldsEpsilon(
            productions,
            nonterminal,
            existingRule);
        uint8_t newYieldsEpsilon = sProductionYieldsEpsilon(
            productions,
            nonterminal,
            rule);

        if (existingYieldsEpsilon && !newYieldsEpsilon)
        {
            if (PrdcPrsnTble__setItem(
                    prdtPrsnTable,
                    nonterminal,
                    kSeq,
                    k,
                    rule) <= CCB_ERROR)
            {
                return CCB_ERROR;
            }

            return CCB_SUCCESS;
        }
        else if (!existingYieldsEpsilon && newYieldsEpsilon)
        {
            return CCB_SUCCESS;
        }

        else
        {
            fprintf(
                stderr,
                "A collision was found for rule P%d and nonterminal NT%d\n",
                rule,
                nonterminal);
            return CCB_ERROR;
        }
    }

    if (PrdcPrsnTble__setItem(prdtPrsnTable, nonterminal, kSeq, k, rule) <= CCB_ERROR)
    {
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

static int8_t sPopulatePrdtPrsnTable(
    PrdcPrsnTble *prdtPrsnTable,
    FirstFollowEntry **first,
    FirstFollowEntry **follow,
    ProductionsHashMap *productions,
    uint8_t k)
{
    for (uint8_t prdcIndex = 0; prdcIndex < productions->nentries; prdcIndex++)
    {
        DoublyLinkedListNode *currNode = ((ProductionsHashMapEntry *)HashMap__getEntries(
                                              productions)[prdcIndex]
                                              ->value)
                                             ->head;

        for (; currNode != NULL; currNode = currNode->next)
        {
            ProductionData *currProdData = currNode->value;
            CCB_nonterminal_t productionNonterminal = currProdData->leftHand;

            GrammarData *firstElement = (GrammarData *)currProdData->rightHandHead->value;
            CCB_grammar_t firstTerminalOfProduction = firstElement->id;

            if (firstTerminalOfProduction == CCB_EMPTY_STRING_TR)
            {
                FirstFollowEntry *followEntryForNonterminal = follow[productionNonterminal];

                SinglyLinkedListNode *currFollowNode = followEntryForNonterminal->entriesHead;
                for (; currFollowNode != NULL; currFollowNode = currFollowNode->next)
                {
                    CCB_terminal_t *currFollowKSeq = currFollowNode->value;

                    if (sSetPrdc4NtNTrInPrdcPrsnTble(
                            prdtPrsnTable,
                            currProdData->id,
                            productionNonterminal,
                            currFollowKSeq,
                            productions,
                            k) <= CCB_ERROR)
                    {
                        return CCB_ERROR;
                    }
                }
            }
            else
            {
                FirstFollowEntry *firstEntryForNonterminal = first[productionNonterminal];
                uint8_t didProductionMatched = false;

                for (
                    FirstFollowEntryNode *currFirstNode = firstEntryForNonterminal //
                                                              ->entriesHead;
                    currFirstNode != NULL;
                    currFirstNode = currFirstNode->next)
                {
                    CCB_terminal_t *currFirstKSeq = currFirstNode->value;

                    if (sDoesProductionMatchesFirstEntry(
                            currProdData,
                            currFirstKSeq,
                            k,
                            productions,
                            first))
                    {
                        if (sSetPrdc4NtNTrInPrdcPrsnTble(
                                prdtPrsnTable,
                                currProdData->id,
                                productionNonterminal,
                                currFirstKSeq,
                                productions,
                                k) <= CCB_ERROR)
                        {
                            return CCB_ERROR;
                        }

                        didProductionMatched = true;
                        break;
                    }
                }

                if (!didProductionMatched)
                {
                    fprintf(
                        stderr,
                        "No entry in FIRST matched production P%d\n",
                        currProdData->id);
                    return CCB_ERROR;
                }
            }
        }
    }

    return CCB_SUCCESS;
}

void PrdcPrsnTble__del(PrdcPrsnTble *self)
{
    for (uint8_t prdcPrsnTbleIndex = 0; prdcPrsnTbleIndex < CCB_NUM_OF_NONTERMINALS; prdcPrsnTbleIndex++)
    {
        HashMap__del((HashMap *)self[prdcPrsnTbleIndex]);
    }

    free(self);
}

void PrdcPrsnTble__log(PrdcPrsnTble *self)
{
    const char *loggerName = "PrdcPrsnTble__log";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(stderr, "Failed to create logger '%s'\n", loggerName);
        return;
    }

    size_t bufferSize = 4096;
    char *buffer = malloc(bufferSize);
    if (buffer == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to allocate buffer for logging\n",
            39);
        ClnLogger__del(logger);
        return;
    }

    size_t offset = snprintf(buffer, bufferSize, "Predictive Parsing Table:\n");

    for (uint8_t ntIndex = 0; ntIndex < CCB_NUM_OF_NONTERMINALS; ntIndex++)
    {
        HashMap *ntHashMap = (HashMap *)self[ntIndex];

        if (ntHashMap == NULL || ntHashMap->nentries == 0)
        {
            continue;
        }

        if (offset >= bufferSize - 256)
        {
            bufferSize *= 2;
            char *newBuffer = realloc(buffer, bufferSize);
            if (newBuffer == NULL)
            {
                fprintf(stderr, "Failed to reallocate buffer for logging\n");
                free(buffer);
                ClnLogger__del(logger);
                return;
            }
            buffer = newBuffer;
        }

        offset += snprintf(buffer + offset, bufferSize - offset, "  NT%d:\n", ntIndex);

        HashMapEntry **entries = HashMap__getEntries(ntHashMap);

        for (ssize_t entryIndex = 0; entryIndex < ntHashMap->nentries; entryIndex++)
        {
            HashMapEntry *entry = entries[entryIndex];

            if (entry == NULL || entry->key == NULL || entry->value == NULL)
            {
                continue;
            }

            if (offset >= bufferSize - 256)
            {
                bufferSize *= 2;
                char *newBuffer = realloc(buffer, bufferSize);
                if (newBuffer == NULL)
                {
                    fprintf(stderr, "Failed to reallocate buffer for logging\n");
                    free(buffer);
                    ClnLogger__del(logger);
                    return;
                }
                buffer = newBuffer;
            }

            CCB_terminal_t *kSeq = (CCB_terminal_t *)entry->key;
            CCB_production_t *production = (CCB_production_t *)entry->value;
            uint8_t k = entry->keySize / sizeof(CCB_terminal_t);

            offset += snprintf(buffer + offset, bufferSize - offset, "    [");

            for (uint8_t i = 0; i < k; i++)
            {
                if (i > 0)
                {
                    offset += snprintf(buffer + offset, bufferSize - offset, ", ");
                }
                offset += snprintf(buffer + offset, bufferSize - offset, "T%d", kSeq[i]);
            }

            offset += snprintf(buffer + offset, bufferSize - offset, "] -> P%d\n", *production);
        }
    }

    ClnLogger__log(logger, CLN_DEBUG_LL, buffer, offset);

    free(buffer);
    ClnLogger__del(logger);
}

PrdcPrsnTble *PrdcPrsnTble__new(ProductionsHashMap *productions, uint8_t k)
{
    FirstFollowEntry **first = First__new(productions, k);

    if (first == NULL)
    {
        return NULL;
    }

    FirstFollowEntry **follow = Follow__new(productions, first, k);

    if (follow == NULL)
    {
        FirstFollow__del(first);
        return NULL;
    }

    PrdcPrsnTble *prdtPrsnTable = malloc(
        sizeof(PrdcPrsnTble) * CCB_NUM_OF_NONTERMINALS);

    if (prdtPrsnTable == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for the predictive parsing table\n");
        FirstFollow__del(first);
        FirstFollow__del(follow);
        return NULL;
    }

    for (uint8_t nonTerminalSlot = 0;
         nonTerminalSlot < CCB_NUM_OF_NONTERMINALS;
         nonTerminalSlot++)
    {
        prdtPrsnTable[nonTerminalSlot] = HashMap__new(LOG2_MINSIZE);

        if (prdtPrsnTable[nonTerminalSlot] == NULL)
        {
            fprintf(
                stderr,
                "Failed to allocate memory for the nonterminal %d in the predictive parsing table\n",
                nonTerminalSlot);
            FirstFollow__del(follow);
            FirstFollow__del(first);
            PrdcPrsnTble__del(prdtPrsnTable);

            return NULL;
        }
    }

    if (sPopulatePrdtPrsnTable(
            prdtPrsnTable, first, follow, productions, k) == CCB_ERROR)
    {
        FirstFollow__del(follow);
        FirstFollow__del(first);
        PrdcPrsnTble__del(prdtPrsnTable);
        return NULL;
    }

    PrdcPrsnTble__log(prdtPrsnTable);

    return prdtPrsnTable;
}
