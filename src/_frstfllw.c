#include <assert.h>
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
    FirstFollowEntry *newEntry = calloc(1, sizeof(FirstFollowEntry));

    if (newEntry == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for first/follow table\n");
        return NULL;
    }

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

char *FirstFollowEntryNode__str(FirstFollowEntryNode *node, uint8_t k)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Calculate required buffer size: "(" + k terminals "T%u, " + ")"
    size_t bufferSize = 2 + k * 10 + 1; // generous estimate

    char *result = calloc(bufferSize, 1);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for FirstFollowEntryNode string\n");
        return NULL;
    }

    char *ptr = result;
    size_t remaining = bufferSize;

    int written = snprintf(ptr, remaining, "(");
    if (written < 0 || (size_t)written >= remaining)
    {
        free(result);
        return NULL;
    }
    ptr += written;
    remaining -= written;

    CCB_terminal_t *terminals = (CCB_terminal_t *)node->value;
    size_t numTerminals = node->valueSize / sizeof(CCB_terminal_t);

    for (size_t j = 0; j < numTerminals; j++)
    {
        if (j > 0)
        {
            written = snprintf(ptr, remaining, ", ");
            if (written < 0 || (size_t)written >= remaining)
            {
                free(result);
                return NULL;
            }
            ptr += written;
            remaining -= written;
        }
        written = snprintf(ptr, remaining, "T%u", terminals[j]);
        if (written < 0 || (size_t)written >= remaining)
        {
            free(result);
            return NULL;
        }
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, ")");
    if (written < 0 || (size_t)written >= remaining)
    {
        free(result);
        return NULL;
    }

    return result;
}

char *FirstFollowEntry__str(FirstFollowEntry *self, uint8_t k)
{
    if (self == NULL || self->entriesHead == NULL)
    {
        return NULL;
    }

    // Calculate required buffer size
    size_t bufferSize = 1; // for null terminator
    bufferSize += 4;       // "{ } "

    size_t entryCount = 0;
    for (SinglyLinkedListNode *curr = self->entriesHead;
         curr != NULL;
         curr = curr->next)
    {
        entryCount++;
        // "(" + k terminals "T%u, " + ")" + possible ", "
        bufferSize += 2 + k * 10 + 2;
    }

    char *result = calloc(bufferSize, 1);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for FirstFollowEntry string\n");
        return NULL;
    }

    char *ptr = result;
    size_t remaining = bufferSize;

    int written = snprintf(ptr, remaining, "{ ");
    if (written < 0 || (size_t)written >= remaining)
    {
        free(result);
        return NULL;
    }
    ptr += written;
    remaining -= written;

    int isFirst = 1;
    for (SinglyLinkedListNode *curr = self->entriesHead;
         curr != NULL;
         curr = curr->next)
    {
        if (!isFirst)
        {
            written = snprintf(ptr, remaining, ", ");
            if (written < 0 || (size_t)written >= remaining)
                break;
            ptr += written;
            remaining -= written;
        }
        isFirst = 0;

        char *nodeStr = FirstFollowEntryNode__str(curr, k);
        if (nodeStr == NULL)
            break;

        written = snprintf(ptr, remaining, "%s", nodeStr);
        free(nodeStr);

        if (written < 0 || (size_t)written >= remaining)
            break;
        ptr += written;
        remaining -= written;
    }

    written = snprintf(ptr, remaining, " }");
    if (written < 0 || (size_t)written >= remaining)
    {
        free(result);
        return NULL;
    }

    return result;
}

char *FirstFollow__str(FirstFollow *self, uint8_t k)
{
    assert(self != NULL);

    FirstFollowEntry **entries = self;

    // Calculate required buffer size
    size_t bufferSize = 1; // for null terminator
    for (size_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        if (entries[i] != NULL && entries[i]->entriesHead != NULL)
        {
            // "NT%zu | " + entry string + "\n"
            bufferSize += 20;

            // Count entries
            for (SinglyLinkedListNode *curr = entries[i]->entriesHead;
                 curr != NULL;
                 curr = curr->next)
            {
                // "(" + k terminals "T%u, " + ")" + possible ", "
                bufferSize += 2 + k * 10 + 2;
            }

            // " { }\n"
            bufferSize += 5;
        }
    }

    char *result = calloc(bufferSize, 1);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for FirstFollow string\n");
        return NULL;
    }

    char *ptr = result;
    size_t remaining = bufferSize;

    for (size_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        if (entries[i] != NULL && entries[i]->entriesHead != NULL)
        {
            char *entryStr = FirstFollowEntry__str(entries[i], k);
            if (entryStr == NULL)
                continue;

            int written = snprintf(ptr, remaining, "NT%zu | %s\n", i, entryStr);
            free(entryStr);

            if (written < 0 || (size_t)written >= remaining)
                break;
            ptr += written;
            remaining -= written;
        }
    }

    return result;
}
