#ifndef CCABRAL_PARSER_H
#define CCABRAL_PARSER_H

#include <cbarroso/tree.h>
#include <ccabral/auxds.h>
#include <ccabral/tknsq.h>

typedef int8_t (*RunRuleActionCallback)(TreeNode **, CCB_production_t);

typedef struct Parser Parser;

Parser *Parser__new(ProductionData **productions, RunRuleActionCallback runRuleAction);
TreeNode *Parser__parse(Parser *self, TokenQueue *input);
void Parser__del(Parser *self);

#endif
