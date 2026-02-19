#include <stdint.h>
#include <ccabral/_auxds.h>
#include <ccabral/constants.h>

uint8_t GrammarData__isEndOfText(GrammarData *self)
{
    return self->id == CCB_END_OF_TEXT_TR && self->type == CCB_TERMINAL_GT;
}

uint8_t GrammarData__isEmptyString(GrammarData *self)
{
    return self->id == CCB_EMPTY_STRING_TR && self->type == CCB_TERMINAL_GT;
}
