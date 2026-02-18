#include <ccabral/tknsq.h>
#include <ccabral/constants.h>
#include <ccauchy.h>

// Test: Create a new TokenQueue
TEST(test_tknsq_new)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");
    ASSERT_EQ(queue->numberOfNodes, 0, "TokenQueue should be empty");
    ASSERT(queue->head == NULL, "TokenQueue head should be NULL");
    ASSERT(queue->tail == NULL, "TokenQueue tail should be NULL");
    free(queue);
}

// Test: Enqueue single terminal
TEST(test_tknsq_enqueue_single)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    CCB_terminal_t terminal = 5;
    int8_t result = TokenQueue__enqueue(queue, terminal);
    ASSERT_EQ(result, CCB_SUCCESS, "Enqueue should return CCB_SUCCESS");
    ASSERT_EQ(queue->numberOfNodes, 1, "Queue size should be 1");
    ASSERT_NOT_NULL(queue->head, "Queue head should not be NULL");
    ASSERT_NOT_NULL(queue->tail, "Queue tail should not be NULL");

    // Cleanup
    CCB_terminal_t value;
    TokenQueue__dequeue(queue, &value);
    free(queue);
}

// Test: Enqueue multiple terminals
TEST(test_tknsq_enqueue_multiple)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    for (CCB_terminal_t i = 0; i < 10; i++)
    {
        int8_t result = TokenQueue__enqueue(queue, i);
        ASSERT_EQ(result, CCB_SUCCESS, "Enqueue should succeed");
        ASSERT_EQ(queue->numberOfNodes, (size_t)(i + 1), "Queue size should increment");
    }

    ASSERT_EQ(queue->numberOfNodes, 10, "Queue size should be 10");

    // Cleanup
    CCB_terminal_t value;
    while (queue->numberOfNodes > 0)
    {
        TokenQueue__dequeue(queue, &value);
    }
    free(queue);
}

// Test: Dequeue single terminal
TEST(test_tknsq_dequeue_single)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    CCB_terminal_t terminal = 42;
    TokenQueue__enqueue(queue, terminal);

    CCB_terminal_t value;
    int8_t result = TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(result, CCB_SUCCESS, "Dequeue should return CCB_SUCCESS");
    ASSERT_EQ(value, terminal, "Dequeued value should match enqueued value");
    ASSERT_EQ(queue->numberOfNodes, 0, "Queue should be empty after dequeue");

    free(queue);
}

// Test: Dequeue from empty queue
TEST(test_tknsq_dequeue_empty)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    CCB_terminal_t value;
    int8_t result = TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(result, CCB_ERROR, "Dequeue from empty queue should return CCB_ERROR");

    free(queue);
}

// Test: FIFO order
TEST(test_tknsq_fifo_order)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    CCB_terminal_t terminals[] = {1, 2, 3, 4, 5};
    size_t count = sizeof(terminals) / sizeof(terminals[0]);

    // Enqueue all terminals
    for (size_t i = 0; i < count; i++)
    {
        TokenQueue__enqueue(queue, terminals[i]);
    }

    // Dequeue and verify FIFO order
    for (size_t i = 0; i < count; i++)
    {
        CCB_terminal_t value;
        int8_t result = TokenQueue__dequeue(queue, &value);
        ASSERT_EQ(result, CCB_SUCCESS, "Dequeue should succeed");
        ASSERT_EQ(value, terminals[i], "Value should match FIFO order");
    }

    ASSERT_EQ(queue->numberOfNodes, 0, "Queue should be empty");
    free(queue);
}

// Test: Mixed operations
TEST(test_tknsq_mixed_operations)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    // Enqueue some values
    for (CCB_terminal_t i = 0; i < 5; i++)
    {
        TokenQueue__enqueue(queue, i);
    }

    // Dequeue some values
    CCB_terminal_t value;
    TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(value, 0, "First dequeue should return 0");
    TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(value, 1, "Second dequeue should return 1");

    // Enqueue more values
    TokenQueue__enqueue(queue, 10);
    TokenQueue__enqueue(queue, 11);

    // Verify remaining values
    ASSERT_EQ(queue->numberOfNodes, 5, "Queue should have 5 elements");

    // Cleanup
    while (queue->numberOfNodes > 0)
    {
        TokenQueue__dequeue(queue, &value);
    }
    free(queue);
}

// Test: Large number of operations
TEST(test_tknsq_large_operations)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    size_t large_count = 100;

    // Enqueue many values
    for (size_t i = 0; i < large_count; i++)
    {
        int8_t result = TokenQueue__enqueue(queue, (CCB_terminal_t)i);
        ASSERT_EQ(result, CCB_SUCCESS, "Enqueue should succeed");
    }

    ASSERT_EQ(queue->numberOfNodes, large_count, "Queue size should be correct");

    // Dequeue all values
    for (size_t i = 0; i < large_count; i++)
    {
        CCB_terminal_t value;
        int8_t result = TokenQueue__dequeue(queue, &value);
        ASSERT_EQ(result, CCB_SUCCESS, "Dequeue should succeed");
        ASSERT_EQ(value, (CCB_terminal_t)i, "Value should match");
    }

    ASSERT_EQ(queue->numberOfNodes, 0, "Queue should be empty");
    free(queue);
}

// Test: Boundary values
TEST(test_tknsq_boundary_values)
{
    TokenQueue *queue = Queue__new();
    ASSERT_NOT_NULL(queue, "TokenQueue should not be NULL");

    // Test with CCB_EMPTY_STRING_TR
    TokenQueue__enqueue(queue, CCB_EMPTY_STRING_TR);
    CCB_terminal_t value;
    TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(value, CCB_EMPTY_STRING_TR, "Should handle CCB_EMPTY_STRING_TR");

    // Test with CCB_END_OF_TEXT_TR
    TokenQueue__enqueue(queue, CCB_END_OF_TEXT_TR);
    TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(value, CCB_END_OF_TEXT_TR, "Should handle CCB_END_OF_TEXT_TR");

    // Test with maximum uint8_t value
    CCB_terminal_t max_value = 255;
    TokenQueue__enqueue(queue, max_value);
    TokenQueue__dequeue(queue, &value);
    ASSERT_EQ(value, max_value, "Should handle maximum uint8_t value");

    free(queue);
}
