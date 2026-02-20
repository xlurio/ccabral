#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <cbarroso/dblylnkdlist.h>
#include <cbarroso/hashmap.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/constants.h>
#include <ccabral/prdcdata.h>
#include <ccabral/prdsmap.h>
#include <ccabral/types.h>

int8_t ProductionsHashMap__initializeTerminal(ProductionsHashMap *self,
                                              CCB_nonterminal_t nonterminal,
                                              ProductionData *production)
{
    ProductionsHashMapEntry *entry = malloc(sizeof(ProductionsHashMapEntry));

    if (entry == NULL)
    {
        fprintf(
            stderr,
            "Failed to create entry for NT%d",
            nonterminal);
        return CCB_ERROR;
    }

    entry->head = DoublyLinkedListNode__new(
        production, sizeof(ProductionData));

    if (entry->head == NULL)
    {
        fprintf(
            stderr,
            "Failed to create linked list for the productions for nonterminal NT%d",
            nonterminal);
        free(entry);
        return CCB_ERROR;
    }

    entry->tail = entry->head;

    CCB_nonterminal_t *keyPtr = malloc(sizeof(CCB_nonterminal_t));
    if (keyPtr == NULL)
    {
        fprintf(stderr, "Failed to allocate key for NT%d", nonterminal);
        DoublyLinkedListNode__del(entry->head);
        free(entry);
        return CCB_ERROR;
    }
    *keyPtr = nonterminal;

    if (HashMap__setItem(
            self,
            keyPtr,
            sizeof(CCB_nonterminal_t),
            entry,
            sizeof(ProductionsHashMapEntry)) <= CBR_ERROR)
    {
        fprintf(
            stderr,
            "Failed to map productions linked list to nonterminal NT%d",
            nonterminal);
        free(keyPtr);
        DoublyLinkedListNode__del(entry->head);
        free(entry);
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

int8_t ProductionsHashMap__insertProdForTerminal(ProductionsHashMap *self,
                                                 CCB_nonterminal_t nonterminal,
                                                 ProductionData *production)
{
    ProductionsHashMapEntry *entry;

    if (HashMap__getItem(
            self,
            &nonterminal,
            sizeof(CCB_nonterminal_t),
            (void **)&entry) <= CBR_ERROR)
    {
        fprintf(
            stderr,
            "Failed to get productions linked list for nonterminal NT%d",
            nonterminal);
        return CCB_ERROR;
    }

    if (DoublyLinkedListNode__insertAtTail(
            entry->tail,
            production,
            sizeof(ProductionData)) <= CBR_ERROR)
    {
        fprintf(
            stderr,
            "Failed to map new production to nonterminal NT%d",
            nonterminal);
        return CCB_ERROR;
    }

    entry->tail = entry->tail->next;

    return CCB_SUCCESS;
}

void ProductionsHashMap__del(ProductionsHashMap *self)
{
    HashMapEntry **entries = HashMap__getEntries(self);

    for (size_t i = 0; i < self->nentries; i++)
    {
        if (entries[i] == NULL)
        {
            continue;
        }

        ProductionsHashMapEntry *entry = (ProductionsHashMapEntry *)entries[i]->value;

        if (entry != NULL && entry->head != NULL)
        {
            DoublyLinkedListNode *currentNode = entry->head;

            while (currentNode != NULL)
            {
                DoublyLinkedListNode *nextNode = currentNode->next;
                free(currentNode->value);
                free(currentNode);
                currentNode = nextNode;
            }
        }

        free(entry);
        free(entries[i]->key);
    }

    HashMap__del(self);
}
