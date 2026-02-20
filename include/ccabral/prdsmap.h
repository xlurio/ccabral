#include <stdio.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <cbarroso/hashmap.h>
#include <cbarroso/dblylnkdlist.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/constants.h>
#include <ccabral/types.h>

typedef HashMap ProductionsHashMap;

int8_t ProductionsHashMap__initializeTerminal(ProductionsHashMap *self,
                                              CCB_nonterminal_t nonterminal,
                                              ProductionData *production);

int8_t ProductionsHashMap__insertProdForTerminal(ProductionsHashMap *self,
                                                 CCB_nonterminal_t nonterminal,
                                                 ProductionData *production);

void ProductionsHashMap__del(ProductionsHashMap *self);
