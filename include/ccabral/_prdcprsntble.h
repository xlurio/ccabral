#ifndef CCABRAL_PREDICTIVE_PARSING_TABLE_H
#define CCABRAL_PREDICTIVE_PARSING_TABLE_H

#include "prdcdata.h"
#include "types.h"

typedef CCB_production_t * PrdcPrsnTble;

PrdcPrsnTble *PrdcPrsnTble__new(ProductionData **productions);
void PrdcPrsnTble__del(PrdcPrsnTble *self);

#endif
