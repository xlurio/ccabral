#ifndef CCABRAL__FIRST_FOLLOW_H
#define CCABRAL__FIRST_FOLLOW_H

#include <stdlib.h>
#include <cbarroso/sngllnkdlist.h>
#include "constants.h"
#include "prdcdata.h"
#include "prdsmap.h"
#include "types.h"

typedef SinglyLinkedListNode FirstFollowEntryNode;

/* Holds the head and tail of a singly linked list of `k`-sized arrays */
typedef struct FirstFollowEntry
{
    FirstFollowEntryNode *entriesHead;
    FirstFollowEntryNode *entriesTail;
} FirstFollowEntry;

/* An array of `FirstFollowEntry`s */
typedef FirstFollowEntry *FirstFollow;

void FirstFollow__del(FirstFollow *self);
FirstFollowEntry *FirstFollowEntry__new();

int8_t FirstFollowEntry__insert(
    FirstFollowEntry *self,
    CCB_terminal_t *kTerminals,
    size_t sizeOfKTerminals);

char *FirstFollowEntry__str(FirstFollowEntry *self, uint8_t k);

/* Creates the FIRST table: a table mapping each nonterminal to the first `k`
terminals each of its rules derive to */
FirstFollow *First__new(ProductionsHashMap *productions, uint8_t k);

/* Creates the FOLLOW table: a table mapping each nonterminal to `k`
terminals. It looks into all productions that nonterminal appears in the right hand
side. If there are up to `k` terminals after or that the nonterminals after it derive
to, they will be mapped to it */
FirstFollow *Follow__new(
    ProductionsHashMap *productions,
    FirstFollow *first,
    uint8_t k);

char *FirstFollow__str(FirstFollow *self, uint8_t k);

#endif
