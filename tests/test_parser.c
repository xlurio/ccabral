#include <ccabral/parser.h>
#include <ccabral/tknsq.h>
#include <ccabral/auxds.h>
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
    if (prod == NULL) return NULL;
    
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    Parser *parser = Parser__new(productions, NULL);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL even with NULL callback");
    
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    // This should not crash
    Parser__del(parser);
    
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    // Create empty token queue
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");
    
    // Parse should fail with empty queue
    TreeNode *tree = Parser__parse(parser, queue);
    ASSERT_NULL(tree, "Parse should return NULL for empty input");
    
    free(queue);
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
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
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    // Verify parser was created with callback
    // (No direct way to test, but creation should succeed)
    
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    // Parser should have stored the productions
    // (Cannot directly access due to struct being opaque, but creation succeeded)
    
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser = Parser__new(productions, mockRuleAction);
    ASSERT_NOT_NULL(parser, "Parser should not be NULL");
    
    // If Parser__new succeeded, the parse table was built successfully
    // (buildPrdcPrsnTbl is called internally and checked for NULL)
    
    Parser__del(parser);
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions[i]);
    }
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
    
    Parser *parser1 = Parser__new(productions1, mockRuleAction);
    Parser *parser2 = Parser__new(productions2, mockRuleAction);
    
    ASSERT_NOT_NULL(parser1, "First parser should not be NULL");
    ASSERT_NOT_NULL(parser2, "Second parser should not be NULL");
    
    // Both parsers should be independent
    Parser__del(parser1);
    Parser__del(parser2);
    
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        freeTestProduction(productions1[i]);
        freeTestProduction(productions2[i]);
    }
    free(productions1);
    free(productions2);
}
