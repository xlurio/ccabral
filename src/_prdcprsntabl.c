#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ccabral/_auxds.h>
#include <ccabral/constants.h>

static uint8_t sSetPrdc4NtNTrInPrdtPrsnTable(
    CCB_production_t **prdtPrsnTable,
    CCB_production_t rule,
    CCB_nonterminal_t nonterminal,
    CCB_terminal_t terminal)
{
    if (prdtPrsnTable[nonterminal][terminal] > CCB_ERROR_PR)
    {
        fprintf(
            stderr,
            "A collision was found for rule %d, nonterminal %d and terminal %d\n",
            rule,
            nonterminal,
            terminal);
        return CCB_ERROR;
    }

    prdtPrsnTable[nonterminal][terminal] = rule;

    return CCB_SUCCESS;
}

static int8_t sPopulatePrdtPrsnTable(
    CCB_production_t **prdtPrsnTable,
    FirstFollowEntry **first,
    FirstFollowEntry **follow,
    ProductionData **productions)
{
    for (uint8_t prdcIndex = 0; prdcIndex < CCB_NUM_OF_PRODUCTIONS; prdcIndex++)
    {
        ProductionData *productionData = productions[prdcIndex];
        CCB_nonterminal_t productionNonterminal = productionData->leftHand;

        GrammarData *firstElement = (GrammarData *)productionData->rightHandHead->value;
        CCB_grammar_t firstTerminalOfProduction = firstElement->id;

        if (firstTerminalOfProduction == CCB_EMPTY_STRING_TR)
        {
            FirstFollowEntry *followEntryForNonterminal = follow[productionNonterminal];

            SinglyLinkedListNode *currFollowNode = followEntryForNonterminal->entriesHead;
            while (currFollowNode != NULL)
            {
                CCB_terminal_t currFollowTerminal = *(CCB_terminal_t *)currFollowNode->value;

                if (sSetPrdc4NtNTrInPrdtPrsnTable(
                        prdtPrsnTable, productionData->id,
                        productionNonterminal,
                        currFollowTerminal) == CCB_ERROR)
                {
                    return CCB_ERROR;
                }
                
                currFollowNode = currFollowNode->next;
            }
        }
        else
        {
            FirstFollowEntry *firstEntryForNonterminal = first[productionNonterminal];
            uint8_t didProductionMatched = 0;

            SinglyLinkedListNode *currFirstNode = firstEntryForNonterminal->entriesHead;
            while (currFirstNode != NULL)
            {
                CCB_terminal_t currFirstTerminal = *(CCB_terminal_t *)currFirstNode->value;

                if (firstTerminalOfProduction == currFirstTerminal)
                {
                    if (sSetPrdc4NtNTrInPrdtPrsnTable(
                            prdtPrsnTable,
                            productionData->id,
                            productionNonterminal,
                            currFirstTerminal) == CCB_ERROR)
                    {
                        return CCB_ERROR;
                    }

                    didProductionMatched = 1;
                    break;
                }
                
                currFirstNode = currFirstNode->next;
            }

            if (!didProductionMatched)
            {
                fprintf(
                    stderr,
                    "No entry in FIRST matched production %d\n",
                    productionData->id);
                return CCB_ERROR;
            }
        }
    }

    return CCB_SUCCESS;
}

void destroyPrdtPrsnTable(CCB_production_t **prdtPrsnTable)
{
    for (uint8_t prdtPrsnTableIndex = 0; prdtPrsnTableIndex < CCB_NUM_OF_NONTERMINALS; prdtPrsnTableIndex++)
    {
        free(prdtPrsnTable[prdtPrsnTableIndex]);
    }

    free(prdtPrsnTable);
}

CCB_production_t **buildPrdcPrsnTbl(ProductionData **productions)
{
    FirstFollowEntry **first = buildFirst(productions);

    if (first == NULL)
    {
        return NULL;
    }

    FirstFollowEntry **follow = buildFollow(productions, first);

    if (follow == NULL)
    {
        destroyFirstFollow(first);
        return NULL;
    }

    CCB_production_t **prdtPrsnTable = malloc(
        sizeof(CCB_production_t **) * CCB_NUM_OF_NONTERMINALS);

    if (prdtPrsnTable == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for the predictive parsing table\n");
        destroyFirstFollow(first);
        destroyFirstFollow(follow);
        return NULL;
    }

    for (uint8_t nonTerminalSlot = 0;
         nonTerminalSlot < CCB_NUM_OF_NONTERMINALS;
         nonTerminalSlot++)
    {
        prdtPrsnTable[nonTerminalSlot] = malloc(
            sizeof(CCB_production_t *) * CCB_NUM_OF_TERMINALS);

        if (prdtPrsnTable[nonTerminalSlot] == NULL)
        {
            fprintf(
                stderr,
                "Failed to allocate memory for the nonterminal %d in the predictive parsing table\n",
                nonTerminalSlot);
            destroyFirstFollow(first);
            destroyFirstFollow(follow);

            for (uint8_t allocatedNonTerminalSlot = 0;
                 allocatedNonTerminalSlot < nonTerminalSlot;
                 allocatedNonTerminalSlot++)
            {
                free(prdtPrsnTable[allocatedNonTerminalSlot]);
            }

            free(prdtPrsnTable);
            return NULL;
        }

        memset(prdtPrsnTable[nonTerminalSlot],
               CCB_ERROR_PR,
               sizeof(CCB_production_t *) * CCB_NUM_OF_TERMINALS);
    }

    if (sPopulatePrdtPrsnTable(
            prdtPrsnTable, first, follow, productions) == CCB_ERROR)
    {
        destroyFirstFollow(first);
        destroyFirstFollow(follow);
        destroyPrdtPrsnTable(prdtPrsnTable);
        return NULL;
    }

    return prdtPrsnTable;
}
