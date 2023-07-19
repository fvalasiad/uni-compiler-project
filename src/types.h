
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
    SLESSEQ, SLESS, SNOTEQ, SEQ, SAND, SOR, SPRINT, SJ, SLABEL, SJZ
} statement_type;

typedef struct {
    /* What instruction is this? */
    statement_type type;

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

/* Finally, let's generate the MIXAL code.
 * This also means that we can finally move to instructions!
 * From expressions, to statements, to instructions! Awesome! */

/* Well, instructions also have types, duh. */
typedef enum {
    IADD, ISUB, IMUL, IDIV, ISTA, ILDA, IENNA, IENTA, IJAZ, IJSJ, IOUT,
    ILDAN, ILDX, ISTX, ICMPA, IJG, IJLE, IJGE, IJL, IJNE, IJE, IJANZ
} instruction_type;

/* In MIXAL, instructions are of the form:
 *  ------------------------------------------------
 * |   0   |   1   |   2   |   3   |   4   |   5    |
 * ------------------------------------------------
 * |        ADDRESS        | INDEX |  MOD  | OPCODE |
 * ------------------------------------------------
 *
 *  where each cell is a single byte, that being 6 bits for MIXAL.
 *  Cell 0 is sign.
 *
 *  A more abstract form:
 *  MNEMONIC  ADDRESS,INDEX(MOD)
 *
 *  where:
 *
 *  MNEMONIC: a mnemonic for a particular opcode
 *  ADDRESS: an ADDRESS to operate on from 0 to 3999
 *  INDEX: an INDEX to the ADDRESS.
 *  MOD: A field separator, effectively specifies which bytes are fetched.
 *  
 *  Given the extremely simple nature of our language, we can ignore
 *  INDEX and MOD and so effectively the instructions we are going to
 *  be dealing with are:
 *
 *  MNEMONIC  [ADDRESS]
 *
 *  Including the labels functionality that MIXAL supplies us with:
 *  [LABEL] MNEMONIC  [ADDRESS]
 * */
typedef struct {
    instruction_type type;	  /* The opcode */

    int label;			  /* The label */

    int address;		  /* The address at hand */
} instruction;

typedef struct {
    /* An array... who would have thought. */
    instruction *instructions;
    int size;
    int capacity;

    /* What temporary does the accumulator contain? */
    int ra;

    /* Next instruction should have that label! */
    int label;
} MIXAL;

void tac_to_MIXAL(three_address_code *tac, MIXAL *mixal);
