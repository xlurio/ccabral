#include <stdio.h>
#include <stdlib.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdcprsntble.h>
#include <ccabral/_prsrstck.h>
#include <ccabral/types.h>
#include <ccabral/constants.h>
#include <ccabral/parser.h>

typedef struct Parser
{
    ProductionsHashMap *productions;
    PrdcPrsnTble *prdcPrsnTble;
    RunRuleActionCallback runRuleAction;
    uint8_t k;
} Parser;

Parser *Parser__new(ProductionsHashMap *productions,
                    RunRuleActionCallback runRuleAction,
                    uint8_t k)
{
    Parser *parser = malloc(sizeof(Parser));

    if (parser == NULL)
    {
        return NULL;
    }

    parser->productions = productions;
    parser->runRuleAction = runRuleAction;
    parser->k = k;

    parser->prdcPrsnTble = PrdcPrsnTble__new(parser->productions, k);

    if (parser->prdcPrsnTble == NULL)
    {
        fprintf(stderr, "Failed to create the predictive parsing table\n");
        free(parser);
        return NULL;
    }

    return parser;
}

static int8_t sUpdateLookahead(
    TokenQueue *input,
    CCB_terminal_t *lookahead,
    uint8_t k)
{
    const char *loggerName = "ParserStack__push";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return false;
    }

    for (uint8_t i = 0; i < k - 1; i++)
    {
        lookahead[i] = lookahead[i + 1];
    }

    if (TokenQueue__dequeue(input, &lookahead[k - 1]) == CCB_ERROR)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to dequeue token for lookahead",
            37);
        lookahead[k - 1] = CCB_END_OF_TEXT_TR;
    }

    ssize_t lookaheadStrLen = 1       // (
                              + k * 8 // TK%d,
                              + 1     // )
                              + 1;    // Null terminator
    ssize_t currLkahStrLen = lookaheadStrLen;
    char *lookaheadStr = calloc(lookaheadStrLen, sizeof(char));

    snprintf(lookaheadStr, currLkahStrLen--, "(");

    for (uint8_t i = 0; i < k; i++)
    {
        char *currTkStr = calloc(8, sizeof(char));

        snprintf(currTkStr, 8, "TK%d, ", lookahead[i]);
        strncat(lookaheadStr, currTkStr, currLkahStrLen);
        currLkahStrLen -= 8;
    }

    strncat(lookaheadStr, ")", currLkahStrLen--);

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "Updated lookahead: %s",
        19 + lookaheadStrLen,
        lookaheadStr);

    free(lookaheadStr);
    ClnLogger__del(logger);
    return CCB_SUCCESS;
}

TreeNode *Parser__parse(Parser *self, TokenQueue *input)
{
    CCB_terminal_t lookahead[self->k];
    memset(lookahead, CCB_END_OF_TEXT_TR, self->k * sizeof(CCB_terminal_t));

    TreeNode *tree = NULL;

    for (uint8_t i = 0; i < self->k; i++)
    {
        if (TokenQueue__dequeue(input, &lookahead[i]) == CCB_ERROR)
        {
            fprintf(stderr, "Failed to initialize lookahead[%d]\n", i);
            return NULL;
        }
    }

    ParserStack *stack = ParserStack__new();

    if (stack == NULL)
    {
        fprintf(stderr, "Failed to create parser stack\n");
        return NULL;
    }

    if (ParserStack__push(stack, CCB_START_NT, CCB_NONTERMINAL_GT) == CCB_ERROR)
    {
        fprintf(stderr, "Failed to push start nonterminal\n");
        Stack__del(stack);
        return NULL;
    }

    GrammarData *stackTop = NULL;

    if (ParserStack__pop(stack, &stackTop) == CCB_ERROR)
    {
        fprintf(stderr, "Failed to pop stack\n");
        Stack__del(stack);
        return NULL;
    }

    CCB_production_t foundRule = -1;

    while (!GrammarData__isEndOfText(stackTop))
    {
        if (stackTop->type == CCB_TERMINAL_GT)
        {
            if (stackTop->id == lookahead[0])
            {
                if (sUpdateLookahead(input, lookahead, self->k) <= CCB_ERROR)
                {
                    char *grammarDataStr = GrammarData__str(stackTop);

                    if (grammarDataStr == NULL)
                    {
                        fprintf(stderr, "Failed to strigify top of stack\n");
                        free(stackTop);
                        Stack__del(stack);
                        return NULL;
                    }

                    fprintf(
                        stderr,
                        "Failed to update lookahead. stackTop=%s\n",
                        grammarDataStr);

                    free(grammarDataStr);
                    free(stackTop);
                    Stack__del(stack);
                    return NULL;
                }

                free(stackTop);
                if (ParserStack__pop(stack, &stackTop) == CCB_ERROR)
                {
                    fprintf(stderr, "Failed to pop the parser stack\n");
                    free(stackTop);
                    Stack__del(stack);
                    return NULL;
                }
            }
            else
            {
                fprintf(stderr, "Unexpected token %d\n", lookahead[0]);
                free(stackTop);
                Stack__del(stack);
                return NULL;
            }

            continue;
        }

        for (
            int16_t lookaheadIdx = self->k;
            lookaheadIdx > 0;
            lookaheadIdx--)
        {
            CCB_terminal_t lkAheadSubset[self->k];

            for (
                int16_t firstSubsetIx = 0;
                firstSubsetIx < self->k;
                firstSubsetIx++)
            {
                lkAheadSubset[firstSubsetIx] = lookahead[firstSubsetIx];
            }

            for (
                int16_t sndSubsetIx = lookaheadIdx;
                sndSubsetIx < self->k;
                sndSubsetIx++)
            {
                lkAheadSubset[sndSubsetIx] = CCB_EMPTY_STRING_TR;
            }

            if (PrdcPrsnTble__getItem(
                    self->prdcPrsnTble,
                    stackTop->id,
                    lkAheadSubset,
                    self->k,
                    &foundRule) <= CCB_ERROR_PR)
            {
                fprintf(stderr, "Failed to lookup the predictive parsing table");

                free(stackTop);
                Stack__del(stack);
                return NULL;
            }

            if (foundRule >= 0)
            {
                break;
            }
            else
            {
                fprintf(stderr, "No production found for the sequence:\n\t(");

                for (
                    int16_t lkAheadSubsetIx = 0;
                    lkAheadSubsetIx < self->k;
                    lkAheadSubsetIx++)
                {
                    fprintf(stderr, "T%d, ", lkAheadSubset[lkAheadSubsetIx]);
                }

                fprintf(stderr, ")\n");
            }
        }

        if (foundRule < 0)
        {
            fprintf(stderr,
                    "Unexpected token TK%d for nonterminal NT%d. The available options are:\n",
                    lookahead[0],
                    stackTop->id);

            HashMapEntry **entries = HashMap__getEntries(self->prdcPrsnTble[stackTop->id]);

            for (
                ssize_t entryIx = 0;
                entryIx < self->prdcPrsnTble[stackTop->id]->nentries;
                entryIx++)
            {
                CCB_terminal_t *currKSeq = entries[entryIx]->value;

                fprintf(stderr, "\t(");

                for (
                    int16_t kSeqIx = 0;
                    kSeqIx < self->k;
                    kSeqIx++)
                {
                    fprintf(stderr, "T%d, ", currKSeq[kSeqIx]);
                }

                fprintf(stderr, ")\n");
            }

            free(stackTop);
            Stack__del(stack);
            return NULL;
        }

        if (self->runRuleAction != NULL)
        {
            self->runRuleAction(&tree, foundRule);
        }

        ProductionData *productionData;

        if (ProductionsHashMap__getProdDataFromNonterminalNRuleId(
                self->productions,
                stackTop->id,
                foundRule,
                &productionData) <= CCB_ERROR)
        {
            fprintf(
                stderr,
                "Failed to get production data for rule P%d",
                foundRule);
            return NULL;
        }

        DoublyLinkedListNode *currentGrammarNode = productionData->rightHandTail;
        GrammarData *currentGrammar = currentGrammarNode->value;

        if (!GrammarData__isEmptyString(currentGrammar))
        {
            if (ParserStack__push(
                    stack,
                    currentGrammar->id,
                    currentGrammar->type) == CCB_ERROR)
            {
                fprintf(stderr, "Failed to push the grammar to stack\n");
                free(stackTop);
                Stack__del(stack);
                return NULL;
            }

            while (currentGrammarNode->prev != NULL)
            {
                currentGrammarNode = currentGrammarNode->prev;
                currentGrammar = currentGrammarNode->value;

                if (ParserStack__push(
                        stack,
                        currentGrammar->id,
                        currentGrammar->type) == CCB_ERROR)
                {
                    fprintf(stderr, "Failed to push the grammar to stack\n");
                    free(stackTop);
                    Stack__del(stack);
                    return NULL;
                }
            }
        }

        free(stackTop);
        if (ParserStack__pop(stack, &stackTop) == -1)
        {
            fprintf(stderr, "Failed to pop the parser stack\n");
            Stack__del(stack);
            return NULL;
        }
    }

    free(stackTop);

    if (lookahead[0] != CCB_END_OF_TEXT_TR)
    {
        fprintf(stderr, "Unexpected token %d after parsing completed\n", lookahead[0]);
        if (tree != NULL)
        {
            TreeNode__del(tree);
        }
        Stack__del(stack);
        return NULL;
    }

    Stack__del(stack);

    return tree;
}

void Parser__del(Parser *self)
{
    PrdcPrsnTble__del(self->prdcPrsnTble);
    free(self);
}
