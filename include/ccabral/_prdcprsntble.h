#ifndef CCABRAL_PREDICTIVE_PARSING_TABLE_H
#define CCABRAL_PREDICTIVE_PARSING_TABLE_H

#include <cbarroso/hashmap.h>
#include "_prdsmap.h"
#include "prdcdata.h"
#include "types.h"

typedef HashMap *PrdcPrsnTble;

PrdcPrsnTble *PrdcPrsnTble__new(ProductionsHashMap *productions, uint8_t k);

int8_t PrdcPrsnTble__getItem(
    PrdcPrsnTble *self,
    CCB_nonterminal_t nonterminal,
    CCB_terminal_t *kSeq,
    uint8_t k,
    CCB_production_t *production);

void PrdcPrsnTble__del(PrdcPrsnTble *self);

#endif
