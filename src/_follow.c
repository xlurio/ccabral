#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ccabral/_frstfllw.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/constants.h>

static bool isTerminalInList(SinglyLinkedListNode *list, CCB_terminal_t terminal)
{
    for (SinglyLinkedListNode *node = list; node != NULL; node = node->next)
    {
        CCB_terminal_t *value = (CCB_terminal_t *)node->value;
        if (*value == terminal)
        {
            return true;
        }
    }
    return false;
}

static bool addTerminalToSet(FirstFollowEntry *entry, CCB_terminal_t terminal)
{
    if (isTerminalInList(entry->entriesHead, terminal))
    {
        return false;
    }

    CCB_terminal_t *terminalPtr = malloc(sizeof(CCB_terminal_t));
    if (terminalPtr == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for terminal\n");
        return false;
    }
    *terminalPtr = terminal;

    SinglyLinkedListNode *newNode = SinglyLinkedListNode__new(terminalPtr, sizeof(CCB_terminal_t));
    if (newNode == NULL)
    {
        free(terminalPtr);
        return false;
    }

    if (entry->entriesHead == NULL)
    {
        entry->entriesHead = newNode;
        entry->entriesTail = newNode;
    }
    else
    {
        entry->entriesTail->next = newNode;
        entry->entriesTail = newNode;
    }

    return true;
}

static bool addTerminalsFromSet(FirstFollowEntry *dest, SinglyLinkedListNode *src)
{
    bool modified = false;

    for (SinglyLinkedListNode *node = src; node != NULL; node = node->next)
    {
        CCB_terminal_t *terminal = (CCB_terminal_t *)node->value;
        if (addTerminalToSet(dest, *terminal))
        {
            modified = true;
        }
    }

    return modified;
}

static FirstFollowEntry **initializeFollowSets(void)
{
    FirstFollowEntry **follow = malloc(sizeof(FirstFollowEntry *) * CCB_NUM_OF_NONTERMINALS);
    if (follow == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for FOLLOW\n");
        return NULL;
    }

    for (uint8_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        follow[i] = malloc(sizeof(FirstFollowEntry));
        if (follow[i] == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for FOLLOW entry %d\n", i);
            // Cleanup previously allocated entries
            for (uint8_t j = 0; j < i; j++)
            {
                if (follow[j]->entriesHead != NULL)
                {
                    SinglyLinkedListNode__del(follow[j]->entriesHead);
                }
                free(follow[j]);
            }
            free(follow);
            return NULL;
        }
        follow[i]->entriesHead = NULL;
        follow[i]->entriesTail = NULL;
    }

    return follow;
}

static bool processNonterminalFollow(FirstFollowEntry **follow,
                                     FirstFollowEntry **first,
                                     CCB_nonterminal_t currentNT,
                                     CCB_nonterminal_t leftHandNT,
                                     DoublyLinkedListNode *suffix)
{
    bool modified = false;
    bool allCanDeriveEpsilon = true;

    // Process all symbols in the suffix after the current nonterminal
    for (DoublyLinkedListNode *node = suffix; node != NULL; node = node->next)
    {
        GrammarData *symbol = (GrammarData *)node->value;

        if (symbol->type == CCB_TERMINAL_GT)
        {
            CCB_terminal_t terminal = (CCB_terminal_t)symbol->id;
            if (terminal != CCB_EMPTY_STRING_TR)
            {
                // Found a non-epsilon terminal, add it to FOLLOW(currentNT)
                if (addTerminalToSet(follow[currentNT], terminal))
                {
                    modified = true;
                }
                allCanDeriveEpsilon = false;
                break;  // Stop processing suffix
            }
            // If it's epsilon terminal, continue to next symbol
        }
        else if (symbol->type == CCB_NONTERMINAL_GT)
        {
            CCB_nonterminal_t nt = (CCB_nonterminal_t)symbol->id;
            
            // Add FIRST(nt) \ {ε} to FOLLOW(currentNT)
            bool ntHasEpsilon = false;
            for (SinglyLinkedListNode *firstNode = first[nt]->entriesHead; 
                 firstNode != NULL; 
                 firstNode = firstNode->next)
            {
                CCB_terminal_t *terminal = (CCB_terminal_t *)firstNode->value;
                if (*terminal == CCB_EMPTY_STRING_TR)
                {
                    ntHasEpsilon = true;
                }
                else
                {
                    if (addTerminalToSet(follow[currentNT], *terminal))
                    {
                        modified = true;
                    }
                }
            }
            
            if (!ntHasEpsilon)
            {
                // This nonterminal cannot derive epsilon, so we stop
                allCanDeriveEpsilon = false;
                break;
            }
            // If it can derive epsilon, continue to next symbol in suffix
        }
    }

    // If all symbols in suffix can derive epsilon (or suffix is empty),
    // add FOLLOW(leftHandNT) to FOLLOW(currentNT)
    if (allCanDeriveEpsilon)
    {
        if (addTerminalsFromSet(follow[currentNT], follow[leftHandNT]->entriesHead))
        {
            modified = true;
        }
    }

    return modified;
}

/**
 * Build FOLLOW sets for all nonterminals in the grammar.
 *
 * FOLLOW(A) is the set of terminals that can appear immediately after nonterminal A
 * in any derivation. This is computed using these rules:
 * 1. Add $ to FOLLOW(start symbol)
 * 2. If A → αBβ, then add FIRST(β) \ {ε} to FOLLOW(B)
 * 3. If A → αB or A → αBβ where ε ∈ FIRST(β), then add FOLLOW(A) to FOLLOW(B)
 *
 * @param productions Array of all grammar productions
 * @param first Array of FIRST sets for all nonterminals
 * @return Array of FOLLOW sets, or NULL on failure
 */
FirstFollowEntry **buildFollow(ProductionData **productions, FirstFollowEntry **first)
{
    FirstFollowEntry **follow = initializeFollowSets();
    if (follow == NULL)
    {
        return NULL;
    }

    // Rule 1: Add end-of-text marker to FOLLOW of start symbol
    addTerminalToSet(follow[CCB_START_NT], CCB_END_OF_TEXT_TR);

    // Iterate until fixed-point is reached (no more changes)
    bool changed = true;
    while (changed)
    {
        changed = false;

        // Process each production
        for (uint8_t prodIdx = 0; prodIdx < CCB_NUM_OF_PRODUCTIONS; prodIdx++)
        {
            ProductionData *prod = productions[prodIdx];
            CCB_nonterminal_t leftHandNT = prod->leftHand;

            // Process each symbol in the right-hand side
            for (DoublyLinkedListNode *node = prod->rightHandHead;
                 node != NULL;
                 node = node->next)
            {
                GrammarData *symbol = (GrammarData *)node->value;

                // Only process nonterminals (terminals don't have FOLLOW sets)
                if (symbol->type == CCB_NONTERMINAL_GT)
                {
                    CCB_nonterminal_t currentNT = (CCB_nonterminal_t)symbol->id;

                    if (processNonterminalFollow(follow, first, currentNT,
                                                 leftHandNT, node->next))
                    {
                        changed = true;
                    }
                }
            }
        }
    }

    return follow;
}
