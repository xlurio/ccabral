#ifndef CCABRAL_CONSTANTS_H
#define CCABRAL_CONSTANTS_H

#include "_types.h"

// Return codes
#define CCB_SUCCESS 0
#define CCB_ERROR -1

// Grammar types
#define CCB_NONTERMINAL_GT (CCB_grammartype_t)0
#define CCB_TERMINAL_GT (CCB_grammartype_t)1

// Nonterminals
#define CCB_START_NT (CCB_nonterminal_t)0

#ifndef CCB_NUM_OF_NONTERMINALS
#define CCB_NUM_OF_NONTERMINALS 1
#endif

// Terminals
#define CCB_EMPTY_STRING_TR (CCB_terminal_t)0
#define CCB_END_OF_TEXT_TR (CCB_terminal_t)1

#ifndef CCB_NUM_OF_TERMINALS
#define CCB_NUM_OF_TERMINALS 2
#endif

// Productions
#define CCB_ERROR_PR (CCB_production_t) - 1

#ifndef CCB_NUM_OF_PRODUCTIONS
#define CCB_NUM_OF_PRODUCTIONS 0
#endif

#endif
