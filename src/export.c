#include "types.h"

void
MIXAL_export(MIXAL *mixal, FILE *out)
{
    fprintf(out, "ORIG 2000\n");
    for (int i = 0; i < mixal->size; ++i) {
	instruction *inst = mixal->instructions + i;

	if (inst->label >= 0) {
	    fprintf(out, "l%d ", inst->label);
	}
	switch (inst->type) {
	    case IADD:{
		fprintf(out, "ADD %d\n", inst->address);
		break;
	    }
	    case ISUB:{
		fprintf(out, "SUB %d\n", inst->address);
		break;
	    }
	    case IMUL:{
		fprintf(out, "MUL %d\n", inst->address);
		break;
	    }
	    case IDIV:{
		fprintf(out, "DIV %d\n", inst->address);
		break;
	    }
	    case ISTA:{
		if (inst->address < 0) {
		    continue;
		}
		fprintf(out, "STA %d\n", inst->address);
		break;
	    }
	    case ILDA:{
		fprintf(out, "LDA %d\n", inst->address);
		break;
	    }
	    case IENNA:{
		fprintf(out, "ENNA %d\n", inst->address);
		break;
	    }
	    case IENTA:{
		fprintf(out, "ENTA %d\n", inst->address);
		break;
	    }
	    case IJAZ:{
		fprintf(out, "JAZ l%d\n", inst->address);
		break;
	    }
	    case IJSJ:{
		fprintf(out, "JSJ l%d\n", inst->address);
		break;
	    }
	    case IOUT:{
		fprintf(out, "OUT %d\n", inst->address);
		break;
	    }
	    case ILDAN:{
		fprintf(out, "LDAN %d\n", inst->address);
		break;
	    }
	    case ILDX:{
		fprintf(out, "LDX %d\n", inst->address);
		break;
	    }
	    case ISTX:{
		fprintf(out, "STX %d\n", inst->address);
		break;
	    }
	    case ICMPA:{
		fprintf(out, "CMPA %d\n", inst->address);
		break;
	    }
	    case IJG:{
		fprintf(out, "JG l%d\n", inst->address);
		break;
	    }
	    case IJLE:{
		fprintf(out, "JLE l%d\n", inst->address);
		break;
	    }
	    case IJGE:{
		fprintf(out, "JGE l%d\n", inst->address);
		break;
	    }
	    case IJL:{
		fprintf(out, "JL l%d\n", inst->address);
		break;
	    }
	    case IJNE:{
		fprintf(out, "JNE l%d\n", inst->address);
		break;
	    }
	    case IJE:{
		fprintf(out, "JE l%d\n", inst->address);
		break;
	    }
	    case IJANZ:{
		fprintf(out, "JANZ l%d\n", inst->address);
		break;
	    }
	}
    }

    fprintf(out, "HLT\nEND 2000");
}
