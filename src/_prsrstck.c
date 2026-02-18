#include <stdlib.h>
#include <stdio.h>
#include <ccabral/_types.h>
#include <ccabral/constants.h>
#include <ccabral/_prsrstck.h>
#include <cbarroso/stack.h>

int8_t ParserStack__push(ParserStack *self,
                         CCB_grammar_t newValue,
                         CCB_grammartype_t grammarType)
{
    GrammarData *gd = malloc(sizeof(GrammarData));

    if (gd == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for GrammarData\n");
        return CCB_ERROR;
    }

    gd->id = newValue;
    gd->type = grammarType;

    if (Stack__push(self, gd, sizeof(GrammarData *)) == -1)
    {
        fprintf(stderr, "Failed to push to stack\n");
        free(gd);
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

int8_t ParserStack__pop(ParserStack *self,
                        GrammarData **value)
{
    if (Stack__pop(self, (void **)value) == -1)
    {
        fprintf(stderr, "Failed to pop from stack\n");
        return CCB_ERROR;
    }

    return CCB_SUCCESS;
}

ParserStack *ParserStack__new()
{
    ParserStack *stack = Stack__new();

    if (stack == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for the parser stack\n");
        return NULL;
    }

    if (ParserStack__push(stack, CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT) == CCB_ERROR)
    {
        fprintf(stderr, "Failed to push end of text terminal to the parser stack\n");
        Stack__del(stack);
        return NULL;
    }

    return stack;
}
