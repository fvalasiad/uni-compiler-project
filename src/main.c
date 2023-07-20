#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<errno.h>
#include	"types.h"

extern FILE *yyin;
context ctx;
extern context ctx;

extern int yyparse(void);

int
yywrap()
{
    return 1;
}

int
main(int argc, char *argv[])
{
    FILE *file = fopen(argv[1], "r");

    if (!file) {
	fprintf(stderr, "Error opening file.\n");
	exit(EXIT_FAILURE);
    }

    yyin = file;

    ctx.size = 0;
    ctx.capacity = 1024;
    ctx.ids = malloc(ctx.capacity);
    if (!ctx.ids) {
	fprintf(stderr, "error : %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    if (yyparse() != 0) {
	fprintf(stderr, "Error parsing file.\n");
	exit(EXIT_FAILURE);
    }

    fclose(file);

    /* Contect->tree now contains the Abstract syntax tree! */
    three_address_code tac;

    ast_to_tac(&tac);

    MIXAL mixal;

    tac_to_MIXAL(&tac, &mixal);
    return 0;
}
