#include	<stdlib.h>
#include	<stdio.h>
#include	"types.h"

extern FILE *yyin;
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

    if (yyparse() != 0) {
	fprintf(stderr, "Error parsing file.\n");
	exit(EXIT_FAILURE);
    }

    fclose(file);
    return 0;
}
