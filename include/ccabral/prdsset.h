#ifndef CCABRAL_PRODUCTIONS_SET_H
#define CCABRAL_PRODUCTIONS_SET_H

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
