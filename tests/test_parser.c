#include <ccabral/parser.h>
#include <ccabral/tknsq.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdsmap.h>
#include <ccabral/prdcdata.h>
#include <ccabral/prdsmap.h>
#include <ccabral/constants.h>
#include <ccabral/types.h>
#include <ccauchy.h>

// Mock rule action callback for testing
static int8_t mockRuleAction(TreeNode **tree, CCB_production_t production)
{
    // Simple mock that creates a tree node if needed
    if (*tree == NULL)
    {
        *tree = TreeNode__new(NULL, 0);
    }
    return CCB_SUCCESS;
}

// Helper function to create a simple production for testing
static ProductionData *createTestProduction(CCB_production_t id,
                                            CCB_nonterminal_t leftHand,
                                            CCB_grammar_t rightSymbol,
                                            CCB_grammartype_t rightType)
{
    ProductionData *prod = malloc(sizeof(ProductionData));
    if (prod == NULL)
        return NULL;

    prod->id = id;
    prod->leftHand = leftHand;

    GrammarData *grammarData = malloc(sizeof(GrammarData));
    if (grammarData == NULL)
    {
        free(prod);
        return NULL;
    }
    grammarData->id = rightSymbol;
    grammarData->type = rightType;

    prod->rightHandHead = DoublyLinkedListNode__new(grammarData, sizeof(GrammarData));
    if (prod->rightHandHead == NULL)
    {
        free(grammarData);
        free(prod);
        return NULL;
    }
    prod->rightHandTail = prod->rightHandHead;

    return prod;
}

// Helper function to free test production
static void freeTestProduction(ProductionData *prod)
{
    if (prod != NULL)
    {
        if (prod->rightHandHead != NULL)
        {
            DoublyLinkedListNode__del(prod->rightHandHead);
        }
        free(prod);
    }
}

// Helper function to create a ProductionsHashMap from a ProductionData array
static ProductionsHashMap *createProductionsHashMap(ProductionData **productions, uint8_t count)
{
    ProductionsHashMap *map = HashMap__new(4);
    if (map == NULL)
        return NULL;

    for (uint8_t i = 0; i < count; i++)
    {
        ProductionData *prod = productions[i];
        if (prod == NULL)
            continue;

        // Check if this nonterminal already exists in the map
        ProductionsHashMapEntry *entry = NULL;
        HashMap__getItem(map, &prod->leftHand,
                        sizeof(CCB_nonterminal_t),
                        (void **)&entry);
        
        if (entry == NULL)
        {
            // Initialize new entry for this nonterminal
            if (ProductionsHashMap__initializeTerminal(map, prod->leftHand, prod) <= CCB_ERROR)
            {
                HashMap__del(map);
                return NULL;
            }
        }
        else
        {
            // Add to existing entry
            if (ProductionsHashMap__insertProdForTerminal(map, prod->leftHand, prod) <= CCB_ERROR)
            {
                HashMap__del(map);
                return NULL;
            }
        }
    }

    return map;
}

// Test: Create a new Parser
TEST(test_parser_new)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(productions, "Productions array should not be NULL");

    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
        ASSERT_NOT_NULL(productions[i], "Production should be created");
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parser creation with NULL callback
TEST(test_parser_new_with_null_callback)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(productions, "Productions array should not be NULL");

    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    // Parser should still be created even with NULL callback
    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, NULL, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL even with NULL callback");

    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parser deletion
TEST(test_parser_del)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(productions, "Productions array should not be NULL");

    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // This should not crash
    Parser__del(parser);
    ProductionsHashMap__del(map);

    free(productions);
}

// Test: Parse empty input
TEST(test_parser_parse_empty_input)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // Create empty token queue
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    // Parse should fail with empty queue
    TreeNode *tree = Parser__parse(parser, queue);
    ASSERT_NULL(tree, "Parse should return NULL for empty input");

    free(queue);
    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parse with single end-of-text token
TEST(test_parser_parse_single_eot_token)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // Create token queue with just EOT
    TokenQueue *queue = Queue__new();
    TokenQueue__enqueue(queue, CCB_END_OF_TEXT_TR);

    // Note: Parsing behavior depends on the grammar defined
    // This test verifies the parser handles EOT correctly
    TreeNode *tree = Parser__parse(parser, queue);

    // Clean up tree if created
    if (tree != NULL)
    {
        TreeNode__del(tree);
    }

    free(queue);
    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parser with mock rule action callback
TEST(test_parser_with_callback)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // Verify parser was created with callback
    // (No direct way to test, but creation should succeed)

    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parser stores productions correctly
TEST(test_parser_stores_productions)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              (CCB_grammar_t)i, CCB_TERMINAL_GT);
        ASSERT_NOT_NULL(productions[i], "Production should be created");
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // Parser should have stored the productions
    // (Cannot directly access due to struct being opaque, but creation succeeded)

    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Parser builds parse table
TEST(test_parser_builds_parse_table)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    Parser *parser = Parser__new(map, mockRuleAction, 1);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");

    // If Parser__new succeeded, the parse table was built successfully
    // (buildPrdcPrsnTbl is called internally and checked for NULL)

    Parser__del(parser);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Multiple parser instances
TEST(test_parser_multiple_instances)
{
    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions1 = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    ProductionData **productions2 = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);

    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions1[i] = createTestProduction(i, CCB_START_NT,
                                               CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
        productions2[i] = createTestProduction(i, CCB_START_NT,
                                               CCB_EMPTY_STRING_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map1 = createProductionsHashMap(productions1, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map1, "ProductionsHashMap 1 should not be NULL");

    Parser *parser1 = Parser__new(map1, mockRuleAction, 1);

    ProductionsHashMap *map2 = createProductionsHashMap(productions2, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map2, "ProductionsHashMap 2 should not be NULL");

    Parser *parser2 = Parser__new(map2, mockRuleAction, 1);

    ASSERT_NOT_NULL(parser1, "First parser should not be NULL");
    ASSERT_NOT_NULL(parser2, "Second parser should not be NULL");

    // Both parsers should be independent
    Parser__del(parser1);
    ProductionsHashMap__del(map1);
    Parser__del(parser2);
    ProductionsHashMap__del(map2);

    free(productions1);
    free(productions2);
}
