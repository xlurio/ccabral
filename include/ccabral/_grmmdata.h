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

#endif
