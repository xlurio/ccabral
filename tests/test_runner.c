#include <ccauchy.h>

// Forward declarations for TokenQueue tests
void test_tknsq_new(void);
void test_tknsq_enqueue_single(void);
void test_tknsq_enqueue_multiple(void);
void test_tknsq_dequeue_single(void);
void test_tknsq_dequeue_empty(void);
void test_tknsq_fifo_order(void);
void test_tknsq_mixed_operations(void);
void test_tknsq_large_operations(void);
void test_tknsq_boundary_values(void);

// Forward declarations for ParserStack tests
void test_prsrstck_new(void);
void test_prsrstck_push_single(void);
void test_prsrstck_push_multiple(void);
void test_prsrstck_pop_single(void);
void test_prsrstck_pop_empty(void);
void test_prsrstck_lifo_order(void);
void test_prsrstck_push_nonterminals(void);
void test_prsrstck_push_terminals(void);
void test_prsrstck_mixed_operations(void);
void test_prsrstck_large_operations(void);
void test_prsrstck_grammar_data(void);

// Forward declarations for auxiliary data structure tests
void test_auxds_grammar_data(void);
void test_auxds_production_data(void);
void test_auxds_first_follow_entry(void);
void test_auxds_add_terminal_to_entry(void);
void test_auxds_build_first_simple(void);
void test_auxds_build_first_allocation(void);
void test_auxds_build_follow_simple(void);
void test_auxds_build_parse_table(void);
void test_auxds_destroy_first_follow(void);
void test_auxds_grammar_types(void);
void test_auxds_production_multiple_symbols(void);

// Forward declarations for Parser tests
void test_parser_new(void);
void test_parser_new_with_null_callback(void);
void test_parser_del(void);
void test_parser_parse_empty_input(void);
void test_parser_parse_single_eot_token(void);
void test_parser_with_callback(void);
void test_parser_stores_productions(void);
void test_parser_builds_parse_table(void);
void test_parser_multiple_instances(void);

int main(void)
{
    printf("===========================================\n");
    printf("         ccabral Test Suite\n");
    printf("===========================================\n\n");

    // TokenQueue Tests
    printf("--- TokenQueue Tests ---\n");
    RUN_TEST(test_tknsq_new);
    RUN_TEST(test_tknsq_enqueue_single);
    RUN_TEST(test_tknsq_enqueue_multiple);
    RUN_TEST(test_tknsq_dequeue_single);
    RUN_TEST(test_tknsq_dequeue_empty);
    RUN_TEST(test_tknsq_fifo_order);
    RUN_TEST(test_tknsq_mixed_operations);
    RUN_TEST(test_tknsq_large_operations);
    RUN_TEST(test_tknsq_boundary_values);
    printf("\n");

    // ParserStack Tests
    printf("--- ParserStack Tests ---\n");
    RUN_TEST(test_prsrstck_new);
    RUN_TEST(test_prsrstck_push_single);
    RUN_TEST(test_prsrstck_push_multiple);
    RUN_TEST(test_prsrstck_pop_single);
    RUN_TEST(test_prsrstck_pop_empty);
    RUN_TEST(test_prsrstck_lifo_order);
    RUN_TEST(test_prsrstck_push_nonterminals);
    RUN_TEST(test_prsrstck_push_terminals);
    RUN_TEST(test_prsrstck_mixed_operations);
    RUN_TEST(test_prsrstck_large_operations);
    RUN_TEST(test_prsrstck_grammar_data);
    printf("\n");

    // Auxiliary Data Structure Tests
    printf("--- Auxiliary Data Structure Tests ---\n");
    RUN_TEST(test_auxds_grammar_data);
    RUN_TEST(test_auxds_production_data);
    RUN_TEST(test_auxds_first_follow_entry);
    RUN_TEST(test_auxds_add_terminal_to_entry);
    RUN_TEST(test_auxds_build_first_simple);
    RUN_TEST(test_auxds_build_first_allocation);
    RUN_TEST(test_auxds_build_follow_simple);
    RUN_TEST(test_auxds_build_parse_table);
    RUN_TEST(test_auxds_destroy_first_follow);
    RUN_TEST(test_auxds_grammar_types);
    RUN_TEST(test_auxds_production_multiple_symbols);
    printf("\n");

    // Parser Tests
    printf("--- Parser Tests ---\n");
    RUN_TEST(test_parser_new);
    RUN_TEST(test_parser_new_with_null_callback);
    RUN_TEST(test_parser_del);
    RUN_TEST(test_parser_parse_empty_input);
    RUN_TEST(test_parser_parse_single_eot_token);
    RUN_TEST(test_parser_with_callback);
    RUN_TEST(test_parser_stores_productions);
    RUN_TEST(test_parser_builds_parse_table);
    RUN_TEST(test_parser_multiple_instances);
    printf("\n");

    // Summary
    printf("===========================================\n");
    printf("Test Results: %d/%d tests passed\n", tests_passed, tests_run);
    printf("===========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
