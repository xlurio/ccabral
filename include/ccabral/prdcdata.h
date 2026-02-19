#ifndef CCABRAL_PRODUCTION_DATA_H
#define CCABRAL_PRODUCTION_DATA_H

#include <cbarroso/dblylnkdlist.h>
#include <ccabral/types.h>

typedef struct ProductionData ProductionData;

ProductionData *ProductionData__new(
    CCB_production_t id,
    CCB_nonterminal_t leftHandNonTerminal,
    CCB_grammar_t firstRightHandGrammar,
    CCB_grammartype_t firstRightHandGrammarType);

int8_t ProductionData__insertRightHandGrammar(
    ProductionData *self,
    CCB_grammar_t rightHandGrammar,
    CCB_grammartype_t rightHandGrammarType);

void ProductionData__del(ProductionData *self);

#endif
