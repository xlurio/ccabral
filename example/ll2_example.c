#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cbarroso/tree.h>
#include <ccabral/constants.h>
#include <ccabral/parser.h>
#include <ccabral/prdcdata.h>
#include <ccabral/prdsmap.h>
#include <ccabral/tknsq.h>
#include <ccabral/types.h>

// Nonterminals
#define BIG_A_1_NT (CCB_nonterminal_t)1
#define ONE_NT (CCB_nonterminal_t)2

// Terminals
#define ZERO_TR (CCB_terminal_t)2
#define ONE_TR (CCB_terminal_t)3

// Productions
#define START_RULE_1_PR (CCB_production_t)0
#define START_RULE_2_PR (CCB_production_t)1
#define BIG_A_1_RULE_1_PR (CCB_production_t)2
#define BIG_A_1_RULE_2_PR (CCB_production_t)3
#define ONE_RULE_PR (CCB_production_t)4

/* `S --> '0' A_1` */
static ProductionData *buildStartRule1()
{
    ProductionData *production = ProductionData__new(
        START_RULE_1_PR,
        CCB_START_NT,
        ZERO_TR);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production\n");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            BIG_A_1_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `S --> '0' one` */
static ProductionData *buildStartRule2()
{
    ProductionData *production = ProductionData__new(
        START_RULE_2_PR,
        CCB_START_NT,
        ZERO_TR);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production\n");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            ONE_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `A_1 --> '0' A_1 one` */
static ProductionData *buildBigA1Rule1()
{
    ProductionData *production = ProductionData__new(
        BIG_A_1_RULE_1_PR,
        BIG_A_1_NT,
        ZERO_TR);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production\n");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            BIG_A_1_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            ONE_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `A_1 --> '0' one one` */
static ProductionData *buildBigA1Rule2()
{
    ProductionData *production = ProductionData__new(
        BIG_A_1_RULE_2_PR,
        BIG_A_1_NT,
        ZERO_TR);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production\n");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            ONE_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(
            production,
            ONE_NT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `one --> '1'` */
static ProductionData *buildOneRule()
{
    ProductionData *production = ProductionData__new(
        ONE_RULE_PR,
        ONE_NT,
        ONE_TR);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production\n");
        return NULL;
    }

    return production;
}

ProductionsHashMap *buildProductions()
{
    ProductionsHashMap *productions = HashMap__new(4);

    if (productions == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for productions data\n");
        return NULL;
    }

    ProductionData *rule1 = buildStartRule1();
    if (rule1 == NULL)
    {
        ProductionsHashMap__del(productions);
        return NULL;
    }

    if (ProductionsHashMap__initializeTerminal(
            productions,
            CCB_START_NT,
            rule1) <= CCB_ERROR)
    {
        ProductionData__del(rule1);
        ProductionsHashMap__del(productions);
        return NULL;
    }

    ProductionData *rule2 = buildStartRule2();
    if (rule2 == NULL)
    {
        ProductionsHashMap__del(productions);
        return NULL;
    }

    if (ProductionsHashMap__insertProdForTerminal(
            productions,
            CCB_START_NT,
            rule2) <= CCB_ERROR)
    {
        ProductionData__del(rule2);
        ProductionsHashMap__del(productions);
        return NULL;
    }

    ProductionData *rule3 = buildBigA1Rule1();
    if (rule3 == NULL)
    {
        ProductionsHashMap__del(productions);
        return NULL;
    }

    if (ProductionsHashMap__initializeTerminal(
            productions,
            BIG_A_1_NT,
            rule3) <= CCB_ERROR)
    {
        ProductionData__del(rule3);
        ProductionsHashMap__del(productions);
        return NULL;
    }

    ProductionData *rule4 = buildBigA1Rule2();
    if (rule4 == NULL)
    {
        ProductionsHashMap__del(productions);
        return NULL;
    }

    if (ProductionsHashMap__insertProdForTerminal(
            productions,
            BIG_A_1_NT,
            rule4) <= CCB_ERROR)
    {
        ProductionData__del(rule4);
        ProductionsHashMap__del(productions);
        return NULL;
    }

    ProductionData *rule5 = buildOneRule();
    if (rule5 == NULL)
    {
        ProductionsHashMap__del(productions);
        return NULL;
    }

    if (ProductionsHashMap__initializeTerminal(
            productions,
            ONE_NT,
            rule5) <= CCB_ERROR)
    {
        ProductionData__del(rule5);
        ProductionsHashMap__del(productions);
        return NULL;
    }

    return productions;
}

void printAstHelper(TreeNode *tree, const char *prefix, int isLast);

/* Displays an ASCII representation of the abstract syntax tree */
void printAst(TreeNode *tree)
{
    printAstHelper(tree, "", 1);
}

void printAstHelper(TreeNode *tree, const char *prefix, int isLast)
{
    if (tree == NULL)
    {
        return;
    }

    printf("%s", prefix);
    printf("%s", isLast ? "└── " : "├── ");
    printf("%s\n", (char *)tree->value);

    SinglyLinkedListNode *current = tree->childrenHead;
    int childCount = 0;
    SinglyLinkedListNode *temp = current;
    while (temp != NULL)
    {
        childCount++;
        temp = temp->next;
    }

    int index = 0;
    while (current != NULL)
    {
        TreeNode *child = (TreeNode *)current->value;
        char newPrefix[256];
        snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "│   ");
        printAstHelper(child, newPrefix, index == childCount - 1);
        current = current->next;
        index++;
    }
}

int8_t runRuleOnlyAction(TreeNode **ast, CCB_production_t rule, char *label)
{
    TreeNode *newLeaf = TreeNode__new(label, strlen(label) + 1);

    if (newLeaf == NULL)
    {
        fprintf(stderr, "Failed to create AST\n");
        return CCB_ERROR;
    }

    if (*ast == NULL)
    {
        *ast = newLeaf;
    }
    else
    {
        if (TreeNode__insert(*ast, newLeaf) == CCB_ERROR)
        {
            fprintf(stderr, "Failed to insert node into AST\n");
            TreeNode__del(newLeaf);
            return CCB_ERROR;
        }
    }

    return CCB_SUCCESS;
}

int8_t runRuleAction(TreeNode **ast, CCB_production_t rule)
{
    switch (rule)
    {
    case START_RULE_1_PR:
        return runRuleOnlyAction(ast, rule, "0");
    case START_RULE_2_PR:
        return runRuleOnlyAction(ast, rule, "0");
    case BIG_A_1_RULE_1_PR:
        return runRuleOnlyAction(ast, rule, "0");
    case BIG_A_1_RULE_2_PR:
        return runRuleOnlyAction(ast, rule, "0");
    case ONE_RULE_PR:
        return runRuleOnlyAction(ast, rule, "1");
    }
    return CCB_ERROR;
}

int main()
{
    TokenQueue *queue = Queue__new();

    // 000111
    TokenQueue__enqueue(queue, ZERO_TR);
    TokenQueue__enqueue(queue, ZERO_TR);
    TokenQueue__enqueue(queue, ZERO_TR);
    TokenQueue__enqueue(queue, ONE_TR);
    TokenQueue__enqueue(queue, ONE_TR);
    TokenQueue__enqueue(queue, ONE_TR);
    TokenQueue__enqueue(queue, CCB_END_OF_TEXT_TR);

    ProductionsHashMap *productions = buildProductions();
    Parser *parser = Parser__new(productions, runRuleAction, 2);

    if (parser == NULL)
    {
        fprintf(stderr, "Failed to create parser\n");
        return EXIT_FAILURE;
    }

    TreeNode *tree = Parser__parse(parser, queue);
    printAst(tree);

    TreeNode__del(tree);
    Parser__del(parser);
    HashMap__del(productions);
    Queue__del(queue);

    return EXIT_SUCCESS;
}
