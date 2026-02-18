#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ccabral/auxds.h>
#include <ccabral/constants.h>

static bool isTerminalInList(SinglyLinkedListNode *list, CCB_terminal_t terminal)
{
    SinglyLinkedListNode *node = list;
    while (node != NULL)
    {
        CCB_terminal_t *value = (CCB_terminal_t *)node->value;
        if (*value == terminal)
        {
            return true;
        }
        node = node->next;
    }
    return false;
}

FirstFollowEntry **buildFirst(ProductionData **productions)
{
    FirstFollowEntry **first = malloc(sizeof(FirstFollowEntry *) * CCB_NUM_OF_NONTERMINALS);

    if (first == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for FIRST\n");
        return NULL;
    }

    // Initialize all FIRST sets
    for (uint8_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        first[i] = malloc(sizeof(FirstFollowEntry));
        if (first[i] == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for FIRST entry %d\n", i);
            for (uint8_t j = 0; j < i; j++)
            {
                if (first[j]->entriesHead != NULL)
                {
                    SinglyLinkedListNode__del(first[j]->entriesHead);
                }
                free(first[j]);
            }
            free(first);
            return NULL;
        }
        first[i]->entriesHead = NULL;
        first[i]->entriesTail = NULL;
    }

    // Compute FIRST sets from productions
    for (uint8_t prodIdx = 0; prodIdx < CCB_NUM_OF_PRODUCTIONS; prodIdx++)
    {
        ProductionData *prod = productions[prodIdx];
        CCB_nonterminal_t leftNT = prod->leftHand;

        // Get first symbol on right-hand side
        if (prod->rightHandHead != NULL)
        {
            GrammarData *firstSymbol = (GrammarData *)prod->rightHandHead->value;

            if (firstSymbol->type == CCB_TERMINAL_GT)
            {
                // If first symbol is terminal, add to FIRST set
                CCB_terminal_t terminal = (CCB_terminal_t)firstSymbol->id;
                if (!isTerminalInList(first[leftNT]->entriesHead, terminal))
                {
                    CCB_terminal_t *terminalPtr = malloc(sizeof(CCB_terminal_t));
                    if (terminalPtr == NULL)
                    {
                        fprintf(stderr, "Failed to allocate memory for terminal\n");
                        continue;
                    }
                    *terminalPtr = terminal;
                    
                    if (first[leftNT]->entriesHead == NULL)
                    {
                        first[leftNT]->entriesHead = SinglyLinkedListNode__new(terminalPtr, sizeof(CCB_terminal_t));
                        first[leftNT]->entriesTail = first[leftNT]->entriesHead;
                    }
                    else
                    {
                        first[leftNT]->entriesTail->next = SinglyLinkedListNode__new(terminalPtr, sizeof(CCB_terminal_t));
                        first[leftNT]->entriesTail = first[leftNT]->entriesTail->next;
                    }
                }
            }
        }
    }

    return first;
}
