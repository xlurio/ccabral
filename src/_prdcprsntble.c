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
    ProductionsHashMap *productions)
{
    DoublyLinkedListNode *currNode = currProdData->rightHandHead;
    uint8_t matchedCount = 0;

    while (currNode != NULL && matchedCount < k)
    {
        GrammarData *grammarData = (GrammarData *)currNode->value;

        if (grammarData->type == CCB_TERMINAL_GT)
        {
            if (grammarData->id != currFirstKSeq[matchedCount])
            {
                return false;
            }
            matchedCount++;
            currNode = currNode->next;
        }
        else
        {
            CCB_nonterminal_t nonterminal = grammarData->id;
            ProductionsHashMapEntry *entry;

            if (HashMap__getItem(
                    productions,
                    &nonterminal,
                    sizeof(CCB_nonterminal_t),
                    (void **)&entry) <= CBR_ERROR)
            {
                return false;
            }

            DoublyLinkedListNode *prodNode = entry->head;
            DoublyLinkedListNode *expandedNode = NULL;

            for (; prodNode != NULL; prodNode = prodNode->next)
            {
                ProductionData *prodData = prodNode->value;
                if (prodData->rightHandHead != NULL)
                {
                    GrammarData *firstElem = (GrammarData *)prodData->rightHandHead->value;
                    if (firstElem->id != CCB_EMPTY_STRING_TR)
                    {
                        expandedNode = prodData->rightHandHead;
                        break;
                    }
                }
            }

            if (expandedNode == NULL)
            {
                return false;
            }

            currNode = expandedNode;
        }
    }

    return matchedCount == k;
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
            (void **) &prodPtr) <= CBR_ERROR)
    {
        fprintf(stderr, "Failed to get rule for nonterminal NT%d\n", nonterminal);
        return CCB_ERROR;
    }

    if (prodPtr == NULL)
    {
        *production = (CCB_production_t)-1;  // Indicate not found
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
                uint8_t didProductionMatched = 0;

                SinglyLinkedListNode *currFirstNode = firstEntryForNonterminal->entriesHead;

                for (; currFirstNode != NULL; currFirstNode = currFirstNode->next)
                {
                    CCB_terminal_t *currFirstKSeq = currFirstNode->value;

                    if (sDoesProductionMatchesFirstEntry(
                            currProdData,
                            currFirstKSeq,
                            k,
                            productions))
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

                        didProductionMatched = 1;
                        break;
                    }
                }

                if (!didProductionMatched)
                {
                    fprintf(
                        stderr,
                        "No entry in FIRST matched production %d\n",
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
    const char *loggerName = "_prdcprsntabl";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    // Calculate buffer size needed
    size_t bufferSize = 10000; // Adjust as needed based on table size
    char *tableStr = malloc(bufferSize);
    if (tableStr == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for table string\n");
        return;
    }

    size_t offset = 0;

    // Add title
    offset += snprintf(tableStr + offset, bufferSize - offset, "Predictive Parsing Table:\n");

    // Print header
    offset += snprintf(tableStr + offset, bufferSize - offset, "     |");
    for (uint8_t terminal = 0; terminal < CCB_NUM_OF_TERMINALS; terminal++)
    {
        offset += snprintf(tableStr + offset, bufferSize - offset, " T%-3d |", terminal);
    }
    offset += snprintf(tableStr + offset, bufferSize - offset, "\n");

    // Print separator
    offset += snprintf(tableStr + offset, bufferSize - offset, "-----+");
    for (uint8_t terminal = 0; terminal < CCB_NUM_OF_TERMINALS; terminal++)
    {
        offset += snprintf(tableStr + offset, bufferSize - offset, "------+");
    }
    offset += snprintf(tableStr + offset, bufferSize - offset, "\n");

    // Print table rows
    for (uint8_t nonterminal = 0; nonterminal < CCB_NUM_OF_NONTERMINALS; nonterminal++)
    {
        offset += snprintf(tableStr + offset, bufferSize - offset, "NT%-3d|", nonterminal);

        for (uint8_t terminal = 0; terminal < CCB_NUM_OF_TERMINALS; terminal++)
        {
            CCB_terminal_t terminalSeq[1] = {terminal};
            CCB_production_t *productionPtr = NULL;

            HashMap__getItem(
                (HashMap *)self[nonterminal],
                terminalSeq,
                sizeof(CCB_terminal_t),
                (void **) &productionPtr);

            if (productionPtr != NULL)
            {
                offset += snprintf(tableStr + offset, bufferSize - offset, " P%-3d |", *productionPtr);
            }
            else
            {
                offset += snprintf(tableStr + offset, bufferSize - offset, "   -  |");
            }
        }
        offset += snprintf(tableStr + offset, bufferSize - offset, "\n");
    }

    ClnLogger__log(logger, CLN_DEBUG_LL, "%s", strlen("%s"), tableStr);
    free(tableStr);
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
