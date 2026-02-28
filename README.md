# ccabral

A C library for implementing parsers using LL(1) parsing techniques with predictive parsing tables.

## Overview

**ccabral** is a lightweight, efficient C library designed to facilitate the creation of LL(1) parsers. It provides a complete framework for building context-free grammar parsers with automatic computation of FIRST and FOLLOW sets, predictive parsing table generation, and tree-based parse result representation.

The library is built with C99 standards and focuses on simplicity, performance, and ease of integration into existing projects.

## Features

- **Automatic FIRST/FOLLOW Set Computation**: Automatically calculates FIRST and FOLLOW sets for your grammar
- **LL(k) Parsing Support**: Configurable lookahead distance (k) for more powerful parsing capabilities
- **Predictive Parsing Table Generation**: Generates LL(k) predictive parsing tables from production rules
- **Token Queue Management**: Built-in token queue for managing input streams
- **Parse Tree Construction**: Constructs abstract syntax trees during parsing
- **Custom Rule Actions**: Support for custom callbacks during rule execution
- **HashMap-Based Production Storage**: Efficient production rule management using hash maps
- **Static Library**: Lightweight static library with minimal dependencies
- **C99 Compatible**: Works with any C99-compliant compiler

## Requirements

- **CMake** 3.10 or higher
- **C99-compatible compiler** (GCC, Clang, MSVC, etc.)
- **cbarroso** library (included as external dependency)
- **clinschoten** library (included as external dependency for logging)

## Installation

### Building from Source

```bash
# Clone the repository
git clone <repository-url>
cd ccabral

# Create build directory and configure
mkdir build
cd build
cmake ..

# Build the library
make

# Install system-wide (optional)
sudo make install
```

### CMake Integration

After installation, you can use ccabral in your CMake projects:

```cmake
find_package(ccabral REQUIRED)
target_link_libraries(your_target PRIVATE ccabral::ccabral)
```

### Build Options

- `CCB_BUILD_EXAMPLES` - Build the example executable (default: OFF)
- `CCB_BUILD_TESTING` - Build the test suite (default: OFF)

Example with options:

```bash
cmake -DCCB_BUILD_EXAMPLES=ON -DCCB_BUILD_TESTING=ON ..
make
```

## Usage

### Basic Parser Setup

```c
#include <ccabral/parser.h>
#include <ccabral/tknsq.h>
#include <ccabral/prdcdata.h>
#include <ccabral/prdsmap.h>
#include <ccabral/constants.h>

// Define your terminals
#define PLUS_TR (CCB_terminal_t)2
#define MINUS_TR (CCB_terminal_t)3
#define NUMBER_TR (CCB_terminal_t)4

// Define your productions
#define EXPR_RULE_1 (CCB_production_t)0
#define EXPR_RULE_2 (CCB_production_t)1

// Create a grammar rule
ProductionData *createRule() {
    // Create production: EXPR -> PLUS
    ProductionData *production = ProductionData__new(
        EXPR_RULE_1,
        CCB_START_NT,
        PLUS_TR,
        CCB_TERMINAL_GT);
    
    return production;
}

// Define rule action callback
int8_t runRuleAction(TreeNode **parseStack, CCB_production_t production) {
    // Custom action when a rule is matched
    return CCB_SUCCESS;
}

int main() {
    // Create productions hash map
    ProductionsHashMap *productions = HashMap__new(4);
    
    // Create and add a rule
    ProductionData *rule = createRule();
    ProductionsHashMap__initializeTerminal(productions, CCB_START_NT, rule);
    
    // Add more rules for the same nonterminal
    // ProductionData *rule2 = createRule2();
    // ProductionsHashMap__insertProdForTerminal(productions, CCB_START_NT, rule2);
    
    // Create parser with k=1 lookahead
    Parser *parser = Parser__new(productions, runRuleAction, 1);
    
    // Create token queue and add tokens
    TokenQueue *tokens = Queue__new();
    TokenQueue__enqueue(tokens, PLUS_TR);
    TokenQueue__enqueue(tokens, CCB_END_OF_TEXT_TR);
    
    // Parse the input
    TreeNode *parseTree = Parser__parse(parser, tokens);
    
    // Clean up
    TreeNode__del(parseTree);
    Parser__del(parser);
    ProductionsHashMap__del(productions);
    Queue__del(tokens);
    
    return 0;
}
```

### Types and Constants

The library provides type definitions for grammar elements:

- `CCB_terminal_t` - Terminal symbol identifier
- `CCB_nonterminal_t` - Non-terminal symbol identifier
- `CCB_production_t` - Production rule identifier
- `CCB_grammar_t` - Generic grammar symbol identifier
- `CCB_grammartype_t` - Type flag for grammar symbols

Built-in constants:

- `CCB_SUCCESS` (0) - Operation succeeded
- `CCB_ERROR` (-1) - Operation failed
- `CCB_EMPTY_STRING_TR` - Epsilon/empty string terminal
- `CCB_END_OF_TEXT_TR` - End-of-input marker
- `CCB_START_NT` - Start symbol non-terminal

## Testing

The library includes a comprehensive test suite covering all major components.

### Building and Running Tests

```bash
mkdir build
cd build
cmake -DCCB_BUILD_TESTING=ON ..
make
./test_runner
```

### Test Coverage

- Token queue operations (enqueue, dequeue, FIFO behavior)
- Parser stack operations (push, pop, LIFO behavior)
- Auxiliary data structures (FIRST/FOLLOW set computation)
- Parser functionality (rule matching, parse tree construction)

See [tests/README.md](tests/README.md) for detailed test documentation.

## Example

A complete working example is available in the [example/main.c](example/main.c) file. To build and run:

```bash
mkdir build
cd build
cmake -DCCB_BUILD_EXAMPLES=ON ..
make
./ccabral_example
```

The example demonstrates:
- Defining terminals and productions
- Creating production rules using ProductionData API
- Building a ProductionsHashMap for efficient rule storage
- Setting up an LL(1) parser with k=1 lookahead
- Implementing custom rule actions for AST construction
- Parsing an input token stream
- Building and visualizing an abstract syntax tree

## Dependencies

- **cbarroso**: C data structures library providing linked lists, queues, stacks, trees, and hashmaps (included in `external/cbarroso`)
- **clinschoten**: Logging library for debugging and diagnostics (included in `external/clinschoten`)
- **ccauchy**: Testing framework (included in `external/ccauchy` for testing only)

## License

This project is licensed under the GNU General Public License v2.0. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please ensure that:

1. Code follows C99 standards
2. All tests pass before submitting
3. New features include appropriate tests
4. Documentation is updated accordingly

## Why "ccabral"

The name **ccabral** is a tribute to [Ana Suelly Câmara Cabral](https://pt.wikipedia.org/wiki/Ana_Suelly_C%C3%A2mara_Cabral).

## See Also

- [cbarroso](external/cbarroso/README.md) - Data structures library used by ccabral
