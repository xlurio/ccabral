#ifndef CCABRAL_GRAMMAR_DATA_H
#define CCABRAL_GRAMMAR_DATA_H

#include <stdbool.h>
#include "types.h"

typedef struct GrammarData
{
    CCB_grammar_t id;
    CCB_grammartype_t type;
} GrammarData;

bool GrammarData__isEndOfText(GrammarData *self);
bool GrammarData__isEmptyString(GrammarData *self);
char *GrammarData__str(GrammarData *self);
bool isGrammarValid(CCB_grammar_t grammarId, CCB_grammartype_t grammarType);

#endif
