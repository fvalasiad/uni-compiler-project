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
    FILE *out = fopen("ast.out", "w");

    if (out == NULL) {
	fprintf(stderr, "Error opening ast output file .\n");
	exit(EXIT_FAILURE);
    }

    ast_print(out);
    fclose(out);

    three_address_code tac;

    ast_to_tac(&tac);
    out = fopen("tac.out", "w");

    if (out == NULL) {
	fprintf(stderr, "Error opening tac output file.\n");
	exit(EXIT_FAILURE);
    }

    tac_print(&tac, out);
    fclose(out);

    MIXAL mixal;

    tac_to_MIXAL(&tac, &mixal);

    out = fopen("a.out", "w");

    if (out == NULL) {
	fprintf(stderr, "Error opening output file.\n");
	exit(EXIT_FAILURE);
    }

    MIXAL_export(&mixal, out);
    return 0;
}
