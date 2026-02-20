#include <ccabral/_frstfllw.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/_prdcdata.h>
#include <ccabral/_prdcprsntble.h>
#include <ccabral/prdcdata.h>
#include <ccabral/prdsmap.h>
#include <ccauchy.h>

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

    // Create grammar data for right-hand side
    GrammarData *grammarData = malloc(sizeof(GrammarData));
    if (grammarData == NULL)
    {
        free(prod);
        return NULL;
    }
    grammarData->id = rightSymbol;
    grammarData->type = rightType;

    // Create doubly linked list node
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
                        sizeof(CCB_nonterminal_t), (void **)&entry);
        
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

// Test: Create GrammarData structure
TEST(test_auxds_grammar_data)
{
    GrammarData data;
    data.id = 5;
    data.type = CCB_TERMINAL_GT;

    ASSERT_EQ(data.id, 5, "Grammar id should be 5");
    ASSERT_EQ(data.type, CCB_TERMINAL_GT, "Type should be CCB_TERMINAL_GT");
}

// Test: Create ProductionData structure
TEST(test_auxds_production_data)
{
    ProductionData *prod = createTestProduction(1, CCB_START_NT,
                                                CCB_EMPTY_STRING_TR, CCB_TERMINAL_GT);
    ASSERT_NOT_NULL(prod, "ProductionData should not be NULL");
    ASSERT_EQ(prod->id, 1, "Production id should be 1");
    ASSERT_EQ(prod->leftHand, CCB_START_NT, "Left hand should be CCB_START_NT");
    ASSERT_NOT_NULL(prod->rightHandHead, "Right hand head should not be NULL");
    ASSERT_NOT_NULL(prod->rightHandTail, "Right hand tail should not be NULL");

    GrammarData *gd = (GrammarData *)prod->rightHandHead->value;
    ASSERT_EQ(gd->id, CCB_EMPTY_STRING_TR, "Right hand symbol should be CCB_EMPTY_STRING_TR");
    ASSERT_EQ(gd->type, CCB_TERMINAL_GT, "Right hand type should be CCB_TERMINAL_GT");

    freeTestProduction(prod);
}

// Test: FirstFollowEntry initialization
TEST(test_auxds_first_follow_entry)
{
    FirstFollowEntry *entry = malloc(sizeof(FirstFollowEntry));
    ASSERT_NOT_NULL(entry, "FirstFollowEntry should not be NULL");

    entry->entriesHead = NULL;
    entry->entriesTail = NULL;

    ASSERT_NULL(entry->entriesHead, "Entries head should be NULL");
    ASSERT_NULL(entry->entriesTail, "Entries tail should be NULL");

    free(entry);
}

// Test: Add terminal to FirstFollowEntry
TEST(test_auxds_add_terminal_to_entry)
{
    FirstFollowEntry *entry = malloc(sizeof(FirstFollowEntry));
    ASSERT_NOT_NULL(entry, "FirstFollowEntry should not be NULL");

    entry->entriesHead = NULL;
    entry->entriesTail = NULL;

    // Add first terminal
    CCB_terminal_t *terminal1 = malloc(sizeof(CCB_terminal_t));
    *terminal1 = 5;
    entry->entriesHead = SinglyLinkedListNode__new(terminal1, sizeof(CCB_terminal_t));
    entry->entriesTail = entry->entriesHead;

    ASSERT_NOT_NULL(entry->entriesHead, "Entries head should not be NULL");
    ASSERT_EQ(*(CCB_terminal_t *)entry->entriesHead->value, 5, "Terminal value should be 5");

    // Add second terminal
    CCB_terminal_t *terminal2 = malloc(sizeof(CCB_terminal_t));
    *terminal2 = 10;
    entry->entriesTail->next = SinglyLinkedListNode__new(terminal2, sizeof(CCB_terminal_t));
    entry->entriesTail = entry->entriesTail->next;

    ASSERT_NOT_NULL(entry->entriesHead->next, "Second entry should exist");
    ASSERT_EQ(*(CCB_terminal_t *)entry->entriesHead->next->value, 10, "Second terminal should be 10");

    // Cleanup
    SinglyLinkedListNode__del(entry->entriesHead);
    free(entry);
}

// Test: First__new with simple productions
TEST(test_auxds_build_first_simple)
{
    // This test verifies that First__new can handle basic productions
    // Note: Requires CCB_NUM_OF_PRODUCTIONS and CCB_NUM_OF_NONTERMINALS to be properly defined

    if (CCB_NUM_OF_PRODUCTIONS == 0)
    {
        printf("  (Skipped - no productions defined)\n");
        return;
    }

    ProductionData **productions = malloc(sizeof(ProductionData *) * CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(productions, "Productions array should not be NULL");

    // Create a simple production: START -> terminal
    for (uint8_t i = 0; i < CCB_NUM_OF_PRODUCTIONS; i++)
    {
        productions[i] = createTestProduction(i, CCB_START_NT,
                                              CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
        ASSERT_NOT_NULL(productions[i], "Production should be created");
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    FirstFollowEntry **first = First__new(map, 1);
    ASSERT_NOT_NULL(first, "First set should not be NULL");

    // Cleanup
    FirstFollow__del(first);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: First__new allocates entries for all nonterminals
TEST(test_auxds_build_first_allocation)
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
                                              CCB_EMPTY_STRING_TR, CCB_TERMINAL_GT);
    }

    ProductionsHashMap *map = createProductionsHashMap(productions, CCB_NUM_OF_PRODUCTIONS);
    ASSERT_NOT_NULL(map, "ProductionsHashMap should not be NULL");

    FirstFollowEntry **first = First__new(map, 1);
    ASSERT_NOT_NULL(first, "First set should not be NULL");

    // Verify all nonterminal entries are allocated
    for (uint8_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        ASSERT_NOT_NULL(first[i], "First entry for nonterminal should exist");
    }

    // Cleanup
    FirstFollow__del(first);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: Follow__new with simple productions
TEST(test_auxds_build_follow_simple)
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

    FirstFollowEntry **first = First__new(map, 1);
    ASSERT_NOT_NULL(first, "First set should not be NULL");

    FirstFollowEntry **follow = Follow__new(map, first, 1);
    ASSERT_NOT_NULL(follow, "Follow set should not be NULL");

    // Cleanup
    FirstFollow__del(follow);
    FirstFollow__del(first);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: buildPrdcPrsnTbl creates parse table
TEST(test_auxds_build_parse_table)
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

    PrdcPrsnTble *parseTable = PrdcPrsnTble__new(map, 1);
    ASSERT_NOT_NULL(parseTable, "Parse table should not be NULL");

    // Cleanup
    PrdcPrsnTble__del(parseTable);
    ProductionsHashMap__del(map);
    free(productions);
}

// Test: FirstFollow__del function
TEST(test_auxds_destroy_first_follow)
{
    // Allocate a first/follow set
    FirstFollowEntry **entries = malloc(sizeof(FirstFollowEntry *) * CCB_NUM_OF_NONTERMINALS);
    ASSERT_NOT_NULL(entries, "Entries array should not be NULL");

    for (uint8_t i = 0; i < CCB_NUM_OF_NONTERMINALS; i++)
    {
        entries[i] = malloc(sizeof(FirstFollowEntry));
        ASSERT_NOT_NULL(entries[i], "Entry should not be NULL");
        entries[i]->entriesHead = NULL;
        entries[i]->entriesTail = NULL;
    }

    // This should not crash
    FirstFollow__del(entries);
}

// Test: GrammarData with different types
TEST(test_auxds_grammar_types)
{
    GrammarData terminal;
    terminal.id = 10;
    terminal.type = CCB_TERMINAL_GT;
    ASSERT_EQ(terminal.type, CCB_TERMINAL_GT, "Should be terminal type");

    GrammarData nonterminal;
    nonterminal.id = 20;
    nonterminal.type = CCB_NONTERMINAL_GT;
    ASSERT_EQ(nonterminal.type, CCB_NONTERMINAL_GT, "Should be nonterminal type");
}

// Test: Production with multiple right-hand symbols
TEST(test_auxds_production_multiple_symbols)
{
    ProductionData *prod = createTestProduction(1, CCB_START_NT,
                                                CCB_END_OF_TEXT_TR, CCB_TERMINAL_GT);
    ASSERT_NOT_NULL(prod, "Production should not be NULL");

    // Add another symbol to right-hand side
    GrammarData *gd2 = malloc(sizeof(GrammarData));
    gd2->id = CCB_EMPTY_STRING_TR;
    gd2->type = CCB_TERMINAL_GT;

    DoublyLinkedListNode *newNode = DoublyLinkedListNode__new(gd2, sizeof(GrammarData));
    prod->rightHandTail->next = newNode;
    newNode->prev = prod->rightHandTail;
    prod->rightHandTail = newNode;

    ASSERT_NOT_NULL(prod->rightHandHead->next, "Should have second symbol");
    GrammarData *secondSymbol = (GrammarData *)prod->rightHandHead->next->value;
    ASSERT_EQ(secondSymbol->id, CCB_EMPTY_STRING_TR, "Second symbol should be empty string");

    freeTestProduction(prod);
}
