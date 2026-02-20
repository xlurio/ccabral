#include <stdio.h>
#include <string.h>
#include <cbarroso/constants.h>
#include <cbarroso/hashmap.h>
#include <cbarroso/dblylnkdlist.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/constants.h>
#include <ccabral/types.h>
#include <ccabral/prdsmap.h>

typedef struct ProductionsHashMapEntry
{
    DoublyLinkedListNode *head;
    DoublyLinkedListNode *tail;
} ProductionsHashMapEntry;

int8_t ProductionsHashMap__getFirstProdListNodeForTerminal(
    ProductionsHashMap *self,
    CCB_nonterminal_t nonterminal,
    ProductionData **productionAddr);

int8_t ProductionsHashMap__getProdDataFromNonterminalNRuleId(
    ProductionsHashMap *self,
    CCB_nonterminal_t nonterminal,
    CCB_production_t production,
    ProductionData **prodDataAddr);

ProductionsHashMap *ProductionsHashMap__deepCopy(ProductionsHashMap *self);
