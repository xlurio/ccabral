#include <stdio.h>
#include <stdlib.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdcprsntble.h>
#include <ccabral/_prsrstck.h>
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
    for (uint8_t i = 0; i < k - 1; i++)
    {
        lookahead[i] = lookahead[i + 1];
    }

    if (TokenQueue__dequeue(input, &lookahead[k - 1]) == CCB_ERROR)
    {
        fprintf(stderr, "Failed to dequeue token for lookahead\n");
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

TreeNode *Parser__parse(Parser *self, TokenQueue *input)
{
    CCB_terminal_t lookahead[self->k];

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
                    fprintf(stderr, "Failed to update lookahead\n");
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
        }

        else if (PrdcPrsnTble__getItem(
                     self->prdcPrsnTble, stackTop->id,
                     lookahead,
                     self->k,
                     &foundRule) == CCB_ERROR_PR)
        {
            fprintf(stderr, "Unexpected token %d\n", lookahead[0]);
            free(stackTop);
            Stack__del(stack);
            return NULL;
        }
        else
        {
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
