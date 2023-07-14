
/* All the defined \"instruction\" or \"expression\" types. */
typedef enum node_type {
    ELITERAL, ENOT, EUMINUS, EMOD, EDIV, EMUL, ESUB, EPLUS, EBIGGEREQ, EBIGGER,
    ELESSEQ, ELESS, ENOTEQ, EEQ, EAND, EOR, EMODASSIGN, EDIVASSIGN, EMULASSIGN,
    ESUBASSIGN, EPLUSASSIGN, EASSIGN, EID
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
    union {
	char *id;
	int i;
    };

    /* parameters, a dynamic array. */
    struct node *params;
    int size;
    int capacity;
} node;

/* That's our context, the data the parser needs to hold on to while parsing the
 * source file. It's rather small given the simple nature of the language we
 * are creating a compiler for.*/
typedef struct context {
    struct id {
	char size;
	char id[];
    } *ids;			  /* A set of all the variables, constant
				     after the DECLS point. */

    int size;			  /* Gotta know when to stop. */
} context;

char *context_find(const context *, const char *, char);
