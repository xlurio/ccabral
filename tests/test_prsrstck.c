#include <ccabral/_prsrstck.h>
#include <ccabral/constants.h>
#include <ccauchy.h>

// Test: Create a new ParserStack
TEST(test_prsrstck_new)
{
    ParserStack *stack = ParserStack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");
    ASSERT_EQ(stack->stackSize, 1, "ParserStack should have 1 element (EOT)");
    ASSERT_NOT_NULL(stack->top, "ParserStack top should not be NULL");
    
    // Verify that CCB_END_OF_TEXT_TR was pushed
    GrammarData *data;
    int8_t result = ParserStack__pop(stack, &data);
    ASSERT_EQ(result, CCB_SUCCESS, "Pop should succeed");
    ASSERT_EQ(data->id, CCB_END_OF_TEXT_TR, "First element should be CCB_END_OF_TEXT_TR");
    ASSERT_EQ(data->type, CCB_TERMINAL_GT, "Type should be CCB_TERMINAL_GT");
    
    free(data);
    free(stack);
}

// Test: Push single grammar symbol
TEST(test_prsrstck_push_single)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    CCB_grammar_t grammar_id = 5;
    int8_t result = ParserStack__push(stack, grammar_id, CCB_NONTERMINAL_GT);
    ASSERT_EQ(result, CCB_SUCCESS, "Push should return CCB_SUCCESS");
    ASSERT_EQ(stack->stackSize, 1, "Stack size should be 1");
    ASSERT_NOT_NULL(stack->top, "Stack top should not be NULL");

    // Cleanup
    GrammarData *data;
    ParserStack__pop(stack, &data);
    free(data);
    free(stack);
}

// Test: Push multiple grammar symbols
TEST(test_prsrstck_push_multiple)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    for (CCB_grammar_t i = 0; i < 10; i++)
    {
        CCB_grammartype_t type = (i % 2 == 0) ? CCB_TERMINAL_GT : CCB_NONTERMINAL_GT;
        int8_t result = ParserStack__push(stack, i, type);
        ASSERT_EQ(result, CCB_SUCCESS, "Push should succeed");
        ASSERT_EQ(stack->stackSize, (size_t)(i + 1), "Stack size should increment");
    }

    ASSERT_EQ(stack->stackSize, 10, "Stack size should be 10");

    // Cleanup
    GrammarData *data;
    while (stack->stackSize > 0)
    {
        ParserStack__pop(stack, &data);
        free(data);
    }
    free(stack);
}

// Test: Pop single grammar symbol
TEST(test_prsrstck_pop_single)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    CCB_grammar_t grammar_id = 42;
    ParserStack__push(stack, grammar_id, CCB_TERMINAL_GT);

    GrammarData *data;
    int8_t result = ParserStack__pop(stack, &data);
    ASSERT_EQ(result, CCB_SUCCESS, "Pop should return CCB_SUCCESS");
    ASSERT_EQ(data->id, grammar_id, "Popped grammar id should match pushed value");
    ASSERT_EQ(data->type, CCB_TERMINAL_GT, "Popped type should be CCB_TERMINAL_GT");
    ASSERT_EQ(stack->stackSize, 0, "Stack should be empty after pop");

    free(data);
    free(stack);
}

// Test: Pop from empty stack
TEST(test_prsrstck_pop_empty)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    GrammarData *data;
    int8_t result = ParserStack__pop(stack, &data);
    ASSERT_EQ(result, CCB_ERROR, "Pop from empty stack should return CCB_ERROR");

    free(stack);
}

// Test: LIFO order
TEST(test_prsrstck_lifo_order)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    CCB_grammar_t grammar_ids[] = {1, 2, 3, 4, 5};
    CCB_grammartype_t types[] = {CCB_TERMINAL_GT, CCB_NONTERMINAL_GT, CCB_TERMINAL_GT, 
                                  CCB_NONTERMINAL_GT, CCB_TERMINAL_GT};
    size_t count = sizeof(grammar_ids) / sizeof(grammar_ids[0]);

    // Push all grammar symbols
    for (size_t i = 0; i < count; i++)
    {
        ParserStack__push(stack, grammar_ids[i], types[i]);
    }

    // Pop and verify LIFO order
    for (int i = count - 1; i >= 0; i--)
    {
        GrammarData *data;
        int8_t result = ParserStack__pop(stack, &data);
        ASSERT_EQ(result, CCB_SUCCESS, "Pop should succeed");
        ASSERT_EQ(data->id, grammar_ids[i], "Grammar id should match LIFO order");
        ASSERT_EQ(data->type, types[i], "Type should match LIFO order");
        free(data);
    }

    ASSERT_EQ(stack->stackSize, 0, "Stack should be empty");
    free(stack);
}

// Test: Push nonterminals
TEST(test_prsrstck_push_nonterminals)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    ParserStack__push(stack, CCB_START_NT, CCB_NONTERMINAL_GT);
    
    GrammarData *data;
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, CCB_START_NT, "Should be CCB_START_NT");
    ASSERT_EQ(data->type, CCB_NONTERMINAL_GT, "Should be nonterminal type");

    free(data);
    free(stack);
}

// Test: Push terminals
TEST(test_prsrstck_push_terminals)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    ParserStack__push(stack, CCB_EMPTY_STRING_TR, CCB_TERMINAL_GT);
    ParserStack__push(stack, CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    
    GrammarData *data;
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, CCB_END_OF_TEXT_TR, "Should be CCB_END_OF_TEXT_TR");
    ASSERT_EQ(data->type, CCB_TERMINAL_GT, "Should be terminal type");
    free(data);
    
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, CCB_EMPTY_STRING_TR, "Should be CCB_EMPTY_STRING_TR");
    ASSERT_EQ(data->type, CCB_TERMINAL_GT, "Should be terminal type");
    free(data);

    free(stack);
}

// Test: Mixed operations
TEST(test_prsrstck_mixed_operations)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    // Push some values
    for (CCB_grammar_t i = 0; i < 5; i++)
    {
        ParserStack__push(stack, i, CCB_NONTERMINAL_GT);
    }

    // Pop some values
    GrammarData *data;
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, 4, "First pop should return 4");
    free(data);
    
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, 3, "Second pop should return 3");
    free(data);

    // Push more values
    ParserStack__push(stack, 10, CCB_TERMINAL_GT);
    ParserStack__push(stack, 11, CCB_TERMINAL_GT);

    // Verify remaining values
    ASSERT_EQ(stack->stackSize, 5, "Stack should have 5 elements");

    // Cleanup
    while (stack->stackSize > 0)
    {
        ParserStack__pop(stack, &data);
        free(data);
    }
    free(stack);
}

// Test: Large number of operations
TEST(test_prsrstck_large_operations)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    size_t large_count = 100;

    // Push many values
    for (size_t i = 0; i < large_count; i++)
    {
        CCB_grammartype_t type = (i % 2 == 0) ? CCB_TERMINAL_GT : CCB_NONTERMINAL_GT;
        int8_t result = ParserStack__push(stack, (CCB_grammar_t)i, type);
        ASSERT_EQ(result, CCB_SUCCESS, "Push should succeed");
    }

    ASSERT_EQ(stack->stackSize, large_count, "Stack size should be correct");

    // Pop all values
    for (int i = large_count - 1; i >= 0; i--)
    {
        GrammarData *data;
        int8_t result = ParserStack__pop(stack, &data);
        ASSERT_EQ(result, CCB_SUCCESS, "Pop should succeed");
        ASSERT_EQ(data->id, (CCB_grammar_t)i, "Value should match");
        free(data);
    }

    ASSERT_EQ(stack->stackSize, 0, "Stack should be empty");
    free(stack);
}

// Test: Verify GrammarData structure
TEST(test_prsrstck_grammar_data)
{
    ParserStack *stack = Stack__new();
    ASSERT_NOT_NULL(stack, "ParserStack should not be NULL");

    // Push different types of grammar symbols
    ParserStack__push(stack, 10, CCB_TERMINAL_GT);
    ParserStack__push(stack, 20, CCB_NONTERMINAL_GT);

    GrammarData *data;
    
    // Check nonterminal
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, 20, "Grammar id should be 20");
    ASSERT_EQ(data->type, CCB_NONTERMINAL_GT, "Type should be nonterminal");
    free(data);

    // Check terminal
    ParserStack__pop(stack, &data);
    ASSERT_EQ(data->id, 10, "Grammar id should be 10");
    ASSERT_EQ(data->type, CCB_TERMINAL_GT, "Type should be terminal");
    free(data);

    free(stack);
}
