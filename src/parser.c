#include <stdio.h>
#include <stdlib.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdcprsntble.h>
#include <ccabral/_prsrstck.h>
#include <ccabral/constants.h>
#include <ccabral/parser.h>

typedef struct Parser
{
    ProductionData **productions;
    PrdcPrsnTble *prdcPrsnTble;
    RunRuleActionCallback runRuleAction;
} Parser;

Parser *Parser__new(ProductionData **productions,
                    RunRuleActionCallback runRuleAction)
{
    Parser *parser = malloc(sizeof(Parser));

    if (parser == NULL)
    {
        return NULL;
    }

    parser->productions = productions;
    parser->runRuleAction = runRuleAction;

    parser->prdcPrsnTble = PrdcPrsnTble__new(parser->productions);

    if (parser->prdcPrsnTble == NULL)
    {
        fprintf(stderr, "Failed to create the predictive parsing table\n");
        free(parser);
        return NULL;
    }

    return parser;
}

TreeNode *Parser__parse(Parser *self, TokenQueue *input)
{
    CCB_terminal_t currToken;

    TreeNode *tree = NULL;

    if (TokenQueue__dequeue(input, &currToken) == CCB_ERROR)
    {
        fprintf(stderr, "Failed to dequeue input\n");
        return NULL;
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

    while (!GrammarData__isEndOfText(stackTop))
    {
        if (stackTop->type == CCB_TERMINAL_GT)
        {
            if (stackTop->id == currToken)
            {
                if (TokenQueue__dequeue(input, &currToken) == CCB_ERROR)
                {
                    fprintf(stderr, "Failed to dequeue input\n");
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
                fprintf(stderr, "Unexpected token %d\n", currToken);
                free(stackTop);
                Stack__del(stack);
                return NULL;
            }
        }
        else if (self->prdcPrsnTble[stackTop->id][currToken] == CCB_ERROR_PR)
        {
            fprintf(stderr, "Unexpected token %d\n", currToken);
            free(stackTop);
            Stack__del(stack);
            return NULL;
        }
        else
        {
            if (self->runRuleAction != NULL)
            {
                self->runRuleAction(&tree, self->prdcPrsnTble[stackTop->id][currToken]);
            }

            ProductionData *productionData = self->productions[self->prdcPrsnTble[stackTop->id][currToken]];
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

    if (currToken != CCB_END_OF_TEXT_TR)
    {
        fprintf(stderr, "Unexpected token %d after parsing completed\n", currToken);
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
