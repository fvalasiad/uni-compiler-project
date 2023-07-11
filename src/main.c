#include	<stdio.h>

void
yyerror(const char *s)
{
    fprintf(stderr, "error: %s", s);
}

int
yywrap()
{
    return 1;
}

int
main()
{
    printf("Hello, World!\n");
}
