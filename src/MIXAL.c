#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define MIXAL_DEFAULT_CAPACITY 256

static void
MIXAL_init(MIXAL *mixal)
{
    mixal->capacity = MIXAL_DEFAULT_CAPACITY;
    mixal->size = 0;
    mixal->instructions = malloc(mixal->capacity);

    if (!mixal->instructions) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }
}

static instruction *
MIXAL_next(MIXAL *mixal)
{
    if (mixal->size == mixal->capacity) {
	mixal->capacity *= 2;
	mixal->instructions =
		realloc(mixal->instructions,
			mixal->capacity * sizeof (instruction));
	if (!mixal->instructions) {
	    fprintf(stderr, "error : %s\n", strerror(errno));
	    exit(EXIT_FAILURE);
	}
    }

    return mixal->instructions + mixal->size++;
}

void
tac_to_MIXAL(three_address_code *tac, MIXAL *mixal)
{
    MIXAL_init(mixal);
    for (int i = 0; i < tac->size; ++i) {
	statement *s = tac->statements + i;

	switch (s->type) {
	    case SMOV:{
		break;
	    }
	    case SNOT:{
		break;
	    }
	    case SUMINUS:{
		break;
	    }
	    case SMOD:{
		break;
	    }
	    case SDIV:{
		break;
	    }
	    case SMUL:{
		break;
	    }
	    case SSUB:{
		break;
	    }
	    case SPLUS:{
		break;
	    }
	    case SBIGGER:{
		break;
	    }
	    case SBIGGEREQ:{
		break;
	    }
	    case SLESSEQ:{
		break;
	    }
	    case SLESS:{
		break;
	    }
	    case SNOTEQ:{
		break;
	    }
	    case SEQ:{
		break;
	    }
	    case SAND:{
		break;
	    }
	    case SOR:{
		break;
	    }
	    case SPRINT:{
		break;
	    }
	    case SJ:{
		break;
	    }
	    case SLABEL:{
		break;
	    }
	    case SJZ:{
		break;
	    }
	}
    }
}
