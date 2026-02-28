#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <clinschoten/constants.h>
#include <clinschoten/logger.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/constants.h>

bool isGrammarValid(CCB_grammar_t grammarId, CCB_grammartype_t grammarType)
{
    const char *loggerName = "isGrammarValid";
    ClnLogger *logger = ClnLogger__new(loggerName, strlen(loggerName));

    if (logger == NULL)
    {
        fprintf(
            stderr,
            "Failed to create logger '%s'\n", loggerName);

        return false;
    }

    if (grammarType == CCB_TERMINAL_GT && grammarId < CCB_NUM_OF_TERMINALS)
        return true;

    else if (grammarType == CCB_NONTERMINAL_GT && grammarId < CCB_NUM_OF_NONTERMINALS)
        return true;

    ClnLogger__log(
        logger,
        CLN_CRITICAL_LL,
        "GrammarData {id=%d, type=%d} is invalid\n\tCCB_NUM_OF_TERMINALS=%d\n\tCCB_NUM_OF_NONTERMINALS=%d",
        128,
        grammarId,
        grammarType,
        CCB_NUM_OF_TERMINALS,
        CCB_NUM_OF_NONTERMINALS);

    ClnLogger__del(logger);
    return false;
}

bool GrammarData__isEndOfText(GrammarData *self)
{
    assert(self != NULL);

    return self->id == CCB_END_OF_TEXT_TR && self->type == CCB_TERMINAL_GT;
}

bool GrammarData__isEmptyString(GrammarData *self)
{
    assert(self != NULL);

    return self->id == CCB_EMPTY_STRING_TR && self->type == CCB_TERMINAL_GT;
}

char *GrammarData__str(GrammarData *self)
{
    assert(self != NULL);

    const size_t MAX_LENGTH = 6;
    char *grammarDataStr = calloc(MAX_LENGTH, sizeof(char));

    if (self->type == CCB_TERMINAL_GT)
        snprintf(
            grammarDataStr,
            MAX_LENGTH,
            "T%d",
            self->id);
    else if (self->type == CCB_NONTERMINAL_GT)
        snprintf(
            grammarDataStr,
            MAX_LENGTH,
            "NT%d",
            self->id);
    else
        assert(false);

    return grammarDataStr;
}
