#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cbarroso/stack.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/types.h>
#include <ccabral/constants.h>
#include <ccabral/_prsrstck.h>

int8_t ParserStack__push(ParserStack *self,
                         CCB_grammar_t newValue,
                         CCB_grammartype_t grammarType)
{
    assert(self != NULL);

    const char *loggerName = "ParserStack__push";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return false;
    }

    GrammarData *gd = malloc(sizeof(GrammarData));

    if (gd == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to allocate memory for GrammarData",
            41);
        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    gd->id = newValue;
    gd->type = grammarType;

    if (Stack__push(self, gd, sizeof(GrammarData)) == -1)
    {
        ClnLogger__log(logger, CLN_ERROR_LL, "Failed to push to stack", 23);
        free(gd);
        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    char *grammarDataStr = GrammarData__str(gd);

    if (grammarDataStr == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to strigify popped grammar",
            33);

        free(gd);
        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "Pushed %s",
        10,
        grammarDataStr);

    free(grammarDataStr);
    ClnLogger__del(logger);

    return CCB_SUCCESS;
}

int8_t ParserStack__pop(ParserStack *self,
                        GrammarData **value)
{
    assert(self != NULL);

    const char *loggerName = "ParserStack__pop";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return false;
    }

    if (Stack__pop(self, (void **)value) == -1)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to pop from stack",
            24);

        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    char *grammarDataStr = GrammarData__str(*value);

    if (grammarDataStr == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to strigify popped grammar",
            33);

        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "Popped %s",
        10,
        grammarDataStr);

    ClnLogger__del(logger);

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
