#include <stdbool.h>
#include <stdint.h>
#include <ccabral/_grmmdata.h>
#include <ccabral/constants.h>

bool GrammarData__isEndOfText(GrammarData *self)
{
    return self->id == CCB_END_OF_TEXT_TR && self->type == CCB_TERMINAL_GT;
}

bool GrammarData__isEmptyString(GrammarData *self)
{
    return self->id == CCB_EMPTY_STRING_TR && self->type == CCB_TERMINAL_GT;
}
