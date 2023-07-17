
/* ABSTRACT SYNTAX TREE OR STH PARSING-RELATED STUFF */

/* All the defined \"instruction\" or \"expression\" types. */
typedef enum node_type {
    ELITERAL, ENOT, EUMINUS, EMOD, EDIV, EMUL, ESUB, EPLUS, EBIGGEREQ, EBIGGER,
    ELESSEQ, ELESS, ENOTEQ, EEQ, EAND, EOR, EASSIGN, EID, EPRINT, ENOOP, EBREAK,
    ECONTINUE, ECOMMA, EDECL, EFOR, EWHILE, EIF
} node_type;

/* That's going to be the \"tree\" representation of the source file.
 * Essentially, each node is a type of \"instruction\" or
 * \"expression\" in the given context. Each node obviously has parameters
 * which need to be packaged with it.
 *
 * It's a linked structure basically.  */
typedef struct node {
    /* What type of node is this? */
    node_type type;

    /* in case the node represents a constant, or an id for that matter. */
    int i;

    /* parameters, a dynamic array. */
    struct node *params;
    int size;
    int capacity;
} node;

/* That's our context, the data the parser needs to hold on to while parsing the
 * source file. It's rather small given the simple nature of the language we
 * are creating a compiler for. */
typedef struct context {
    /* A set of all the variables, constant after the DECLS point. */
    struct id {
	char size;
	char id[];
    } *ids;

    int size;			  /* How many (bytes) of them are there? */
    int capacity;

    node tree;
} context;

int context_find(const context *, const char *, char);

void context_insert(context *, const char *, char);

/* THREE ADDRESS CODE IR OR STH IDK I came up with this on the fly
 * upon seeing some MIXAL docs. It reminds me of 8086 in some aspects
 * so I'll probably be biased.
 *
 * Gonna call them statements here for a change.
 *
 * Some key aspects: SSA! */

/* Once again, we need types, don't we? */
typedef enum {
    SMOV, SNOT, SUMINUS, SMOD, SDIV, SMUL, SSUB, SPLUS, SBIGGER, SBIGGEREQ,
    SLESSEQ, SLESS, SNOTEQ, SEQ, SAND, SOR, SPRINT, SJ, SNOOP, SJZ
} statement_type;

typedef struct {
    /* What instruction is this? */
    statement_type type;

    /* Any label present? */
    int label;

    /* That's enough, I think */
    int tx;
    int ty;
    int tz;
} statement;

typedef struct {
    int label_start;
    int label_end;
} loop;

typedef struct {
    /* A program is, if we think about it, just an array of those. */
    statement *statements;
    int size;
    int capacity;

    /* Let's carry some context while we are at it. */

    /* Basically vars[i] = temp, where temp is the temporary currently
       associated with the var. */
    int *vars;

    int label;

    loop *loops;
    int loops_size;
    int loops_capacity;
} three_address_code;

/* The context is a global, thanks POSIX yacc! */
void ast_to_tac(three_address_code *tac);
