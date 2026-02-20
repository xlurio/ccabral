#include <stdio.h>
#include <ccabral/_frstfllw.h>

void FirstFollow__del(FirstFollow *self)
{
    FirstFollowEntry **entries = self;
    for (size_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        if (entries[i] != NULL)
        {
            SinglyLinkedListNode__del(entries[i]->entriesHead);
            free(entries[i]);
            entries[i] = NULL;
        }
    }

    free(self);
}

FirstFollowEntry *FirstFollowEntry__new()
{
    FirstFollowEntry *newEntry = malloc(sizeof(FirstFollowEntry));

    if (newEntry == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for first/follow table\n");
        return NULL;
    }

    newEntry->entriesHead = NULL;
    newEntry->entriesTail = NULL;

    return newEntry;
}

static int8_t sFirstFollowEntry__initialize(
    FirstFollowEntry *self,
    CCB_terminal_t *firstKTerminals,
    size_t sizeOfKTerminals)
{
    self->entriesHead = SinglyLinkedListNode__new(firstKTerminals, sizeOfKTerminals);

    if (self->entriesHead == NULL)
    {
        fprintf(stderr, "Failed to create linked list for first/follow entry");
        return CCB_ERROR;
    }

    self->entriesTail = self->entriesHead;

    return CCB_SUCCESS;
}

static int8_t sFirstFollowEntry__containsSequence(
    FirstFollowEntry *self,
    CCB_terminal_t *kTerminals,
    size_t sizeOfKTerminals)
{
    if (self->entriesHead == NULL)
    {
        return 0;
    }

    for (SinglyLinkedListNode *curr = self->entriesHead;
         curr != NULL;
         curr = curr->next)
    {
        if (curr->valueSize == sizeOfKTerminals &&
            memcmp(curr->value, kTerminals, sizeOfKTerminals) == 0)
        {
            return 1;
        }
    }

    return 0;
}

int8_t FirstFollowEntry__insert(
    FirstFollowEntry *self,
    CCB_terminal_t *kTerminals,
    size_t sizeOfKTerminals)
{
    if (sFirstFollowEntry__containsSequence(self, kTerminals, sizeOfKTerminals))
    {
        return CCB_SUCCESS;
    }

    if (self->entriesHead == NULL)
    {
        if (sFirstFollowEntry__initialize(self, kTerminals, sizeOfKTerminals) <= CCB_ERROR)
        {
            return CCB_ERROR;
        }
        return CCB_SUCCESS;
    }

    SinglyLinkedListNode__insertAtTail(
        self->entriesTail,
        kTerminals,
        sizeOfKTerminals);

    return CCB_SUCCESS;
}
