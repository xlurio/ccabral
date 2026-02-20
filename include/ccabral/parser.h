#ifndef CCABRAL_PARSER_H
#define CCABRAL_PARSER_H

#include <stdint.h>
#include <cbarroso/tree.h>
#include "prdcdata.h"
#include "prdsmap.h"
#include "tknsq.h"

typedef int8_t (*RunRuleActionCallback)(TreeNode **, CCB_production_t);

typedef struct Parser Parser;

Parser *Parser__new(ProductionsHashMap *productions,
                    RunRuleActionCallback runRuleAction,
                    uint8_t k);
TreeNode *Parser__parse(Parser *self, TokenQueue *input);
void Parser__del(Parser *self);

#endif
