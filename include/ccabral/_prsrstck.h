#ifndef CCABRAL__PRSRSTCK_H
#define CCABRAL__PRSRSTCK_H

#include "types.h"
#include "auxds.h"
#include <cbarroso/stack.h>

typedef Stack ParserStack;

ParserStack *ParserStack__new();
int8_t ParserStack__push(ParserStack *self,
                         CCB_grammar_t newValue,
                         CCB_grammartype_t grammarType);
int8_t ParserStack__pop(ParserStack *self,
                        GrammarData **value);

#endif
