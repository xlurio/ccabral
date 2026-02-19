#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cbarroso/tree.h>
#include <ccabral/types.h>
#include <ccabral/constants.h>
#include <ccabral/parser.h>
#include <ccabral/tknsq.h>
#include <ccabral/prdcdata.h>

// Terminals
#define PLUS_TR (CCB_terminal_t)2
#define MINUS_TR (CCB_terminal_t)3
#define LOWER_A_TR (CCB_terminal_t)4

// Productions
#define START_RULE_1_PR (CCB_production_t)0
#define START_RULE_2_PR (CCB_production_t)1
#define START_RULE_3_PR (CCB_production_t)2

void destroyProductions(ProductionData **productions, int8_t productionsSize)
{
    if (productionsSize < 0)
    {
        productionsSize = CCB_NUM_OF_PRODUCTIONS;
    }

    for (uint8_t productionsIndex = 0;
         productionsIndex < productionsSize;
         productionsIndex++)
    {
        ProductionData__del(productions[productionsIndex]);
    }

    free(productions);
}

/* `S --> '+' S S` */
static ProductionData *buildStartRule1()
{
    ProductionData *production = ProductionData__new(
        START_RULE_1_PR,
        CCB_START_NT,
        PLUS_TR,
        CCB_TERMINAL_GT);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(production, CCB_START_NT, CCB_NONTERMINAL_GT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(production, CCB_START_NT, CCB_NONTERMINAL_GT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `S --> '-' S S` */
static ProductionData *buildStartRule2()
{
    ProductionData *production = ProductionData__new(
        START_RULE_2_PR,
        CCB_START_NT,
        MINUS_TR,
        CCB_TERMINAL_GT);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production");
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(production, CCB_START_NT, CCB_NONTERMINAL_GT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    if (ProductionData__insertRightHandGrammar(production, CCB_START_NT, CCB_NONTERMINAL_GT) <= CCB_ERROR)
    {
        ProductionData__del(production);
        return NULL;
    }

    return production;
}

/* `S --> 'a'` */
static ProductionData *buildStartRule3()
{
    ProductionData *production = ProductionData__new(
        START_RULE_3_PR,
        CCB_START_NT,
        LOWER_A_TR,
        CCB_TERMINAL_GT);

    if (production == NULL)
    {
        fprintf(stderr, "Failed to create production");
        return NULL;
    }

    return production;
}

ProductionData **buildProductions()
{
    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);

    if (productions == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for productions data\n");
        return NULL;
    }

    productions[START_RULE_1_PR] = buildStartRule1();
    if (productions[START_RULE_1_PR] == NULL)
    {
        destroyProductions(productions, 0);
        return NULL;
    }

    productions[START_RULE_2_PR] = buildStartRule2();
    if (productions[START_RULE_2_PR] == NULL)
    {
        destroyProductions(productions, 1);
        return NULL;
    }

    productions[START_RULE_3_PR] = buildStartRule3();
    if (productions[START_RULE_3_PR] == NULL)
    {
        destroyProductions(productions, 2);
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

int8_t runStartRule1Action(TreeNode **ast, CCB_production_t rule, char *label)
{
    TreeNode *newHead = TreeNode__new(label, strlen(label) + 1);

    if (newHead == NULL)
    {
        fprintf(stderr, "Failed to create AST\n");
        return CCB_ERROR;
    }

    if (*ast == NULL)
    {
        *ast = newHead;
    }
    else
    {
        if (TreeNode__insert(newHead, *ast) == CCB_ERROR)
        {
            fprintf(stderr, "Failed to insert node into AST\n");
            TreeNode__del(newHead);
            return CCB_ERROR;
        }
        *ast = newHead;
    }

    return CCB_SUCCESS;
}

int8_t runStartRule2Action(TreeNode **ast, CCB_production_t rule)
{
    TreeNode *newLeaf = TreeNode__new("a", 2);

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
        return runStartRule1Action(ast, rule, "+");
    case START_RULE_2_PR:
        return runStartRule1Action(ast, rule, "-");
    case START_RULE_3_PR:
        return runStartRule2Action(ast, rule);
    }
    return CCB_ERROR;
}

int main()
{
    TokenQueue *queue = Queue__new();

    // Enqueue: + + a a - a a $
    TokenQueue__enqueue(queue, PLUS_TR);
    TokenQueue__enqueue(queue, PLUS_TR);
    TokenQueue__enqueue(queue, LOWER_A_TR);
    TokenQueue__enqueue(queue, LOWER_A_TR);
    TokenQueue__enqueue(queue, MINUS_TR);
    TokenQueue__enqueue(queue, LOWER_A_TR);
    TokenQueue__enqueue(queue, LOWER_A_TR);
    TokenQueue__enqueue(queue, CCB_END_OF_TEXT_TR);

    ProductionData **productions = buildProductions();
    Parser *parser = Parser__new(productions, runRuleAction);

    TreeNode *tree = Parser__parse(parser, queue);
    printAst(tree);

    TreeNode__del(tree);
    Parser__del(parser);
    destroyProductions(productions, CCB_NUM_OF_PRODUCTIONS);
    Queue__del(queue);

    return EXIT_SUCCESS;
}
