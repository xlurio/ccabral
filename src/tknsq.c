#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/types.h>
#include <ccabral/constants.h>
#include <ccabral/tknsq.h>

int8_t TokenQueue__enqueue(TokenQueue *self, CCB_terminal_t newValue)
{
    assert(self != NULL);

    const char *loggerName = "ProductionsHashMap__getProdDataFromNonterminalNRuleId";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n",
            loggerName);
        return CCB_ERROR;
    }

    if (!isGrammarValid(newValue, CCB_TERMINAL_GT))
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "TK%d is not a valid token",
            32,
            newValue);

        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    CCB_terminal_t *value = malloc(sizeof(CCB_terminal_t));
    if (value == NULL)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to allocate memory for terminal value",
            46);

        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    *value = newValue;

    if (Queue__enqueue(self, value, sizeof(CCB_terminal_t)) == -1)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Failed to enqueue terminal",
            26);

        free(value);
        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    ClnLogger__del(logger);

    return CCB_SUCCESS;
}

int8_t TokenQueue__dequeue(TokenQueue *self, CCB_terminal_t *valueAddress)
{
    assert(self != NULL);

    const char *loggerName = "TokenQueue__dequeue";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return false;
    }

    CCB_terminal_t *dequeuedPtr = NULL;
    if (Queue__dequeue(self, (void **)&dequeuedPtr) == -1)
    {
        ClnLogger__log(
            logger,
            CLN_ERROR_LL,
            "Empty queue",
            13);

        ClnLogger__del(logger);
        return CCB_ERROR;
    }

    *valueAddress = *dequeuedPtr;
    free(dequeuedPtr);

    ClnLogger__log(
        logger,
        CLN_DEBUG_LL,
        "Dequeue T%d",
        12,
        *valueAddress);

    ClnLogger__del(logger);

    return CCB_SUCCESS;
}
