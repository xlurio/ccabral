#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <cbarroso/dblylnkdlist.h>
#include <cbarroso/hashmap.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/constants.h>
#include <ccabral/prdsmap.h>
#include <ccabral/prdcdata.h>
#include <ccabral/types.h>

int8_t ProductionsHashMap__getFirstProdListNodeForTerminal(
    ProductionsHashMap *self,
    CCB_nonterminal_t nonterminal,
    ProductionData **productionAddr)
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

    memcpy(*productionAddr, entry->head, sizeof(DoublyLinkedListNode *));

    return CCB_SUCCESS;
}

int8_t ProductionsHashMap__getProdDataFromNonterminalNRuleId(
    ProductionsHashMap *self,
    CCB_nonterminal_t nonterminal,
    CCB_production_t production,
    ProductionData **prodDataAddr)
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
            "Failed to get productions linked list for nonterminal NT%d\n",
            nonterminal);
        return CCB_ERROR;
    }

    DoublyLinkedListNode *currentNode = entry->head;
    while (currentNode != NULL)
    {
        ProductionData *prodData = currentNode->value;
        if (prodData->id == production)
        {
            *prodDataAddr = prodData;
            return CCB_SUCCESS;
        }
        currentNode = currentNode->next;
    }

    fprintf(
        stderr,
        "Failed to find production %d for nonterminal NT%d\n",
        production,
        nonterminal);
    return CCB_ERROR;
}

ProductionsHashMap *ProductionsHashMap__deepCopy(ProductionsHashMap *self)
{
    if (self == NULL)
    {
        return NULL;
    }

    ProductionsHashMap *copy = HashMap__new(self->log2_size);
    if (copy == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for ProductionsHashMap copy\n");
        return NULL;
    }

    HashMapEntry **entries = HashMap__getEntries(self);

    for (size_t i = 0; i < self->nentries; i++)
    {
        if (entries[i] == NULL)
        {
            continue;
        }

        CCB_nonterminal_t *nonterminalKey = (CCB_nonterminal_t *)entries[i]->key;
        ProductionsHashMapEntry *originalEntry = (ProductionsHashMapEntry *)entries[i]->value;

        ProductionsHashMapEntry *newEntry = malloc(sizeof(ProductionsHashMapEntry));
        if (newEntry == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for ProductionsHashMapEntry copy\n");
            HashMap__del(copy);
            return NULL;
        }

        newEntry->head = NULL;
        newEntry->tail = NULL;

        if (originalEntry->head != NULL)
        {
            DoublyLinkedListNode *currentNode = originalEntry->head;

            ProductionData *originalProduction = currentNode->value;
            ProductionData *copiedProduction = ProductionData__deepCopy(originalProduction);

            if (copiedProduction == NULL)
            {
                free(newEntry);
                HashMap__del(copy);
                return NULL;
            }

            newEntry->head = DoublyLinkedListNode__new(
                copiedProduction,
                sizeof(ProductionData));

            if (newEntry->head == NULL)
            {
                ProductionData__del(copiedProduction);
                free(newEntry);
                HashMap__del(copy);
                return NULL;
            }

            free(copiedProduction);
            newEntry->tail = newEntry->head;
            currentNode = currentNode->next;

            while (currentNode != NULL)
            {
                originalProduction = currentNode->value;
                copiedProduction = ProductionData__deepCopy(originalProduction);

                if (copiedProduction == NULL)
                {
                    DoublyLinkedListNode__del(newEntry->head);
                    free(newEntry);
                    HashMap__del(copy);
                    return NULL;
                }

                if (DoublyLinkedListNode__insertAtTail(
                        newEntry->tail,
                        copiedProduction,
                        sizeof(ProductionData)) <= CBR_ERROR)
                {
                    ProductionData__del(copiedProduction);
                    DoublyLinkedListNode__del(newEntry->head);
                    free(newEntry);
                    HashMap__del(copy);
                    return NULL;
                }

                free(copiedProduction);
                newEntry->tail = newEntry->tail->next;
                currentNode = currentNode->next;
            }
        }

        CCB_nonterminal_t *newKey = malloc(sizeof(CCB_nonterminal_t));
        if (newKey == NULL)
        {
            fprintf(stderr, "Failed to allocate key for deep copy\n");
            ProductionsHashMap__del(copy);
            return NULL;
        }
        *newKey = *nonterminalKey;

        if (HashMap__setItem(
                copy,
                newKey,
                sizeof(CCB_nonterminal_t),
                newEntry,
                sizeof(ProductionsHashMapEntry)) <= CBR_ERROR)
        {
            free(newKey);
            if (newEntry->head != NULL)
            {
                DoublyLinkedListNode__del(newEntry->head);
            }
            free(newEntry);
            HashMap__del(copy);
            return NULL;
        }
    }

    return copy;
}
