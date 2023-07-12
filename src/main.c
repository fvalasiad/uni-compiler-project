#include	<stdio.h>
#include	<stdarg.h>

void
yyerror(const char *format, ...)
{
    fprintf(stderr, "error: ");

    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
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
