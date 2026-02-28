#include <assert.h>
#include <string.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <cbarroso/hashmap.h>
#include <ccabral/_frstfllw.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/types.h>

static int8_t sFollow__PopulateFromFirstEntry(
    FirstFollow *self,
    FirstFollowEntryNode *currFirstEntry,
    GrammarData *currGrammar,
    CCB_nonterminal_t nonterminal,
    ProductionsHashMap *productions,
    FirstFollow *first,
    CCB_terminal_t **kSeq0Ptr,
    size_t *kSeq0LenPtr,
    CCB_terminal_t **kSeq1Ptr,
    size_t *kSeq1LenPtr,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromFirstEntry";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    size_t kSeq2Len = *kSeq0LenPtr;
    CCB_terminal_t kSeq2[k];
    memcpy(kSeq2, *kSeq0Ptr, k * sizeof(CCB_terminal_t));

    CCB_terminal_t *firstKSeq = (CCB_terminal_t *)currFirstEntry->value;

    for (
        int8_t firstEntryIdx = 0;
        firstEntryIdx < k &&                                         //
        firstKSeq[firstEntryIdx] != CCB_EMPTY_STRING_TR &&           //
        isGrammarValid(firstKSeq[firstEntryIdx], CCB_TERMINAL_GT) && //
        kSeq2Len < k;
        firstEntryIdx++)
    {
        kSeq2[kSeq2Len++] = firstKSeq[firstEntryIdx];
    }

    /* If the `sFollow__PopulateFromProdMapEntryNodeSuffix` + this counter does not
    reach `k` */
    if (kSeq2Len < k - 1)
    {
        /* Update `sFollow__PopulateFromNonterminal` `kSeq` and `kSeqLen` */
        memcpy(*kSeq1Ptr, kSeq2, k * sizeof(CCB_terminal_t));
        *kSeq1LenPtr = kSeq2Len;
    }
    else
    {
        /* Insert the found k-tuple as a FOLLOW entry */
        if (FirstFollowEntry__insert(
                self[nonterminal],
                kSeq2,
                k * sizeof(CCB_terminal_t)) <= CCB_ERROR)
        {
            ClnLogger__log(
                logger,
                CLN_ERROR_LL,
                "Failed to insert node into FOLLOW entry",
                39);
            ClnLogger__del(logger);
            return CCB_ERROR;
        }
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

static int8_t sFollow__PopulateFromNonterminal(
    FirstFollow *self,
    GrammarData *currGrammar,
    CCB_nonterminal_t nonterminal,
    ProductionsHashMap *productions,
    FirstFollow *first,
    CCB_terminal_t **kSeq0Ptr,
    size_t *kSeq0LenPtr,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromNonterminal";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    size_t kSeq1Len = *kSeq0LenPtr;
    CCB_terminal_t kSeq1[k];
    memcpy(kSeq1, *kSeq0Ptr, k * sizeof(CCB_terminal_t));

    if (first[nonterminal] == NULL)
    {
        ClnLogger__del(logger);
        return CCB_SUCCESS;
    }

    for (
        FirstFollowEntryNode *currFirstEntry = first[currGrammar->id]->entriesHead;
        currFirstEntry != NULL;
        currFirstEntry = currFirstEntry->next)
    {
        if (sFollow__PopulateFromFirstEntry(
                self,
                currFirstEntry,
                currGrammar,
                nonterminal,
                productions,
                first,
                kSeq0Ptr,
                kSeq0LenPtr,
                (CCB_terminal_t **)&kSeq1,
                &kSeq1Len,
                k) <= CCB_ERROR)
        {
            ClnLogger__log(
                logger,
                CLN_ERROR_LL,
                "Failed to populate FOLLOW from FIRST entry",
                42);
            ClnLogger__del(logger);
            return CCB_ERROR;
        }
    }

    /* If the `sFollow__PopulateFromProdMapEntryNodeSuffix` + this counter does not
    reach `k` */
    if (kSeq1Len < k - 1)
    {
        /* Update `sFollow__PopulateFromProdMapEntryNodeSuffix` `kSeq` and `kSeqLen` */
        memcpy(*kSeq0Ptr, kSeq1, kSeq1Len * sizeof(CCB_terminal_t));
        *kSeq0LenPtr = kSeq1Len;
    }
    else
    {
        /* Go to the next nonterminal */
        *kSeq0LenPtr = k;
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

static int8_t sFollow__PopulateFromProdMapEntryNodeSuffix(
    FirstFollow *self,
    CCB_nonterminal_t nonterminal,
    DoublyLinkedListNode *prodRightNode,
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromProdMapEntryNodeSuffix";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    CCB_terminal_t kSeq[k];

    memset(kSeq, 0x0, k * sizeof(CCB_terminal_t));

    size_t kSeqLen = 0;

    for (
        DoublyLinkedListNode *currProdRightNode = prodRightNode;
        currProdRightNode != NULL;
        currProdRightNode = currProdRightNode->next)
    {
        GrammarData *currGrammar = currProdRightNode->value;

        if (currGrammar->type == CCB_NONTERMINAL_GT)
        {
            if (sFollow__PopulateFromNonterminal(
                    self,
                    currGrammar,
                    nonterminal,
                    productions,
                    first,
                    (CCB_terminal_t **)&kSeq,
                    &kSeqLen,
                    k) <= CCB_ERROR)
            {
                ClnLogger__log(
                    logger,
                    CLN_ERROR_LL,
                    "Failed to populate FOLLOW from nonterminal",
                    42);
                ClnLogger__del(logger);
                return CCB_ERROR;
            }
        }
        else
            assert(false);
    }

    if (kSeqLen > 0)
    {
        if (FirstFollowEntry__insert(
                self[nonterminal],
                kSeq,
                kSeqLen * sizeof(CCB_terminal_t)) <= CCB_ERROR)
        {
            ClnLogger__log(
                logger,
                CLN_ERROR_LL,
                "Failed to insert node into FOLLOW entry",
                39);
            ClnLogger__del(logger);
            return CCB_ERROR;
        }
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

static int8_t sFollow__PopulateFromProdMapEntryNode(
    FirstFollow *self,
    DoublyLinkedListNode *prodEntry,
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromProdMapEntryNode";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    ProductionData *prodData = prodEntry->value;

    for (
        DoublyLinkedListNode *currProdRightNode = prodData->rightHandHead;
        currProdRightNode != NULL;
        currProdRightNode = currProdRightNode->next)
    {
        GrammarData *currGrammar = currProdRightNode->value;

        if (currGrammar->type == CCB_NONTERMINAL_GT //
            && currProdRightNode->next != NULL)
        {
            if (self[currGrammar->id] == NULL)
            {
                self[currGrammar->id] = FirstFollowEntry__new();

                if (self[currGrammar->id] == NULL)
                {
                    ClnLogger__log(
                        logger,
                        CLN_ERROR_LL,
                        "Failed to create FOLLOW entry",
                        29);
                    ClnLogger__del(logger);
                    return CCB_ERROR;
                }
            }

            if (sFollow__PopulateFromProdMapEntryNodeSuffix(
                    self,
                    currGrammar->id,
                    currProdRightNode->next,
                    productions,
                    first,
                    k) <= CCB_ERROR)
            {
                ClnLogger__log(
                    logger,
                    CLN_ERROR_LL,
                    "Failed to populate FOLLOW from productions map entry linked%s",
                    75,
                    "list node suffix");
                ClnLogger__del(logger);
                return CCB_ERROR;
            }
        }
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

static int8_t sFollow__PopulateFromProdEntry(
    FirstFollow *self,
    HashMapEntry *prodEntry,
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromProdEntry";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    ProductionsHashMapEntry *prodMapEntry = prodEntry->value;

    for (
        DoublyLinkedListNode *currProdMapEntryNode = prodMapEntry->head;
        currProdMapEntryNode != NULL;
        currProdMapEntryNode = currProdMapEntryNode->next)
    {
        if (sFollow__PopulateFromProdMapEntryNode(
                self,
                currProdMapEntryNode,
                productions,
                first,
                k) <= CCB_ERROR)
        {
            ClnLogger__log(
                logger,
                CLN_ERROR_LL,
                "Failed to populate FOLLOW from productions map entry linked list node",
                69);
            ClnLogger__del(logger);
            return CCB_ERROR;
        }
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

static int8_t sFollow__PopulateFromProductions(
    FirstFollow *self,
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    const char *loggerName = "sFollow__PopulateFromProductions";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return CCB_ERROR;
    }

    HashMapEntry **prodEntries = HashMap__getEntries(productions);

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "Populating FOLLOW from %d nonterminal productions",
        128,
        productions->nentries);

    for (
        int8_t prodEntriesIdx = 0;
        prodEntriesIdx < productions->nentries;
        prodEntriesIdx++)
    {
        if (sFollow__PopulateFromProdEntry(
                self,
                prodEntries[prodEntriesIdx],
                productions,
                first,
                k) <= CCB_ERROR)
        {
            ClnLogger__log(
                logger,
                CLN_ERROR_LL,
                "Failed to populate FOLLOW from entry",
                36);
            ClnLogger__del(logger);
            return CCB_ERROR;
        }
    }

    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

FirstFollow *Follow__new(
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k)
{
    const char *loggerName = "Follow__new";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return NULL;
    }

    FirstFollow *follow = calloc(sizeof(FirstFollowEntry *), CCB_NUM_OF_NONTERMINALS);

    if (follow == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to allocate memory for FOLLOW",
            36);
        ClnLogger__del(logger);
        return NULL;
    }

    follow[CCB_START_NT] = FirstFollowEntry__new();

    if (follow[CCB_START_NT] == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to create FOLLOW entry for start nonterminal",
            51);
        ClnLogger__del(logger);
        FirstFollow__del(follow);
        return NULL;
    }

    CCB_terminal_t endOfTextEntry[] = {CCB_END_OF_TEXT_TR, 0x0};

    if (FirstFollowEntry__insert(
            follow[CCB_START_NT],
            endOfTextEntry,
            k * sizeof(CCB_terminal_t)) <= CCB_ERROR)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to insert end of text for start nonterminal",
            50);
        ClnLogger__del(logger);
        FirstFollow__del(follow);
        return NULL;
    }

    if (sFollow__PopulateFromProductions(follow, productions, first, k) <= CCB_ERROR)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to populate FOLLOW",
            25);
        ClnLogger__del(logger);
        FirstFollow__del(follow);
        return NULL;
    }

    char *followStr = FirstFollow__str(follow, k);

    if (followStr == NULL)
    {
        fprintf(stderr, "Failed to strigify FOLLOW table");

        ClnLogger__del(logger);
        FirstFollow__del(follow);
        return NULL;
    }

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "FOLLOW:\n%s",
        8 + sizeof(followStr),
        followStr);

    free(followStr);
    ClnLogger__del(logger);

    return follow;
}
