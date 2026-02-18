# ccabral

A C library for implementing parsers using LL(1) parsing techniques with predictive parsing tables.

## Overview

**ccabral** is a lightweight, efficient C library designed to facilitate the creation of LL(1) parsers. It provides a complete framework for building context-free grammar parsers with automatic computation of FIRST and FOLLOW sets, predictive parsing table generation, and tree-based parse result representation.

The library is built with C99 standards and focuses on simplicity, performance, and ease of integration into existing projects.

## Features

- **Automatic FIRST/FOLLOW Set Computation**: Automatically calculates FIRST and FOLLOW sets for your grammar
- **Predictive Parsing Table Generation**: Generates LL(1) predictive parsing tables from production rules
- **Token Queue Management**: Built-in token queue for managing input streams
- **Parse Tree Construction**: Constructs abstract syntax trees during parsing
- **Custom Rule Actions**: Support for custom callbacks during rule execution
- **Static Library**: Lightweight static library with minimal dependencies
- **C99 Compatible**: Works with any C99-compliant compiler

## Requirements

- **CMake** 3.10 or higher
- **C99-compatible compiler** (GCC, Clang, MSVC, etc.)
- **cbarroso** library (included as external dependency)

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

- `CCB_BUILD_EXAMPLE` - Build the example executable (default: OFF)
- `CCB_BUILD_TESTING` - Build the test suite (default: OFF)

Example with options:

```bash
cmake -DCCB_BUILD_EXAMPLE=ON -DCCB_BUILD_TESTING=ON ..
make
```

## Usage

### Basic Parser Setup

```c
#include <ccabral/parser.h>
#include <ccabral/tknsq.h>
#include <ccabral/_auxds.h>
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
    ProductionData *production = malloc(sizeof(ProductionData));
    production->id = EXPR_RULE_1;
    production->leftHand = CCB_START_NT;
    
    // Build right-hand side with grammar symbols
    GrammarData *symbol = malloc(sizeof(GrammarData));
    symbol->id = PLUS_TR;
    symbol->type = CCB_TERMINAL_GT;
    production->rightHandHead = DoublyLinkedListNode__new(symbol, sizeof(GrammarData *));
    production->rightHandTail = production->rightHandHead;
    
    return production;
}

// Define rule action callback
int8_t runRuleAction(TreeNode **parseStack, CCB_production_t production) {
    // Custom action when a rule is matched
    return CCB_SUCCESS;
}

int main() {
    // Set up productions array
    ProductionData **productions = malloc(sizeof(ProductionData *) * NUM_PRODUCTIONS);
    productions[0] = createRule();
    // ... add more rules
    
    // Create parser
    Parser *parser = Parser__new(productions, runRuleAction);
    
    // Create token queue and add tokens
    TokenQueue *tokens = Queue__new(sizeof(CCB_terminal_t));
    CCB_terminal_t token = PLUS_TR;
    TokenQueue__enqueue(tokens, token);
    
    // Parse the input
    TreeNode *parseTree = Parser__parse(parser, tokens);
    
    // Clean up
    Parser__del(parser);
    Queue__del(tokens);
    
    return 0;
}
```

### Core Components

#### Parser

The main parser component that performs LL(1) predictive parsing:

```c
Parser *Parser__new(ProductionData **productions, RunRuleActionCallback runRuleAction);
TreeNode *Parser__parse(Parser *self, TokenQueue *input);
void Parser__del(Parser *self);
```

#### TokenQueue

A FIFO queue for managing input tokens:

```c
int8_t TokenQueue__enqueue(TokenQueue *self, CCB_terminal_t newValue);
int8_t TokenQueue__dequeue(TokenQueue *self, CCB_terminal_t *valueAddress);
```

#### Grammar Data Structures

- **GrammarData**: Represents a grammar symbol (terminal or non-terminal)
- **ProductionData**: Represents a production rule with left-hand side and right-hand side
- **FirstFollowEntry**: Stores FIRST and FOLLOW sets for non-terminals

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
cmake -DCCB_BUILD_EXAMPLE=ON ..
make
./ccabral_example
```

The example demonstrates:
- Defining terminals and productions
- Creating production rules with right-hand sides
- Setting up a parser with custom rule actions
- Parsing an input token stream
- Constructing a parse tree

## Dependencies

- **cbarroso**: C data structures library providing linked lists, queues, stacks, trees, and hashmaps (included in `external/cbarroso`)
- **ccauchy**: Testing framework (included in `external/ccauchy` for testing only)

## License

This project is licensed under the GNU General Public License v2.0. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please ensure that:

1. Code follows C99 standards
2. All tests pass before submitting
3. New features include appropriate tests
4. Documentation is updated accordingly

## Version

Current version: **0.1.0**

## See Also

- [cbarroso](external/cbarroso/README.md) - Data structures library used by ccabral
