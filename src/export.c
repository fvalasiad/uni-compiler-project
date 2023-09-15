#include "types.h"

void
MIXAL_export(MIXAL *mixal, FILE *out)
{
    fprintf(out, "   ORIG 2000\n");
    for (int i = 0; i < mixal->size; ++i) {
	instruction *inst = mixal->instructions + i;

	if (inst->label >= 0) {
	    int count = fprintf(out, "l%d", inst->label);

	    fprintf(out, " ");
	    for (int i = 0; i < 3 - count; ++i) {
		fprintf(out, " ");
	    }
	} else {
	    fprintf(out, "    ");
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
		if (inst->address >= 0) {
		    fprintf(out, "JAZ l%d\n", inst->address);
		} else {
		    fprintf(out, "JAZ ldbze\n");
		}
		break;
	    }
	    case IJSJ:{
		fprintf(out, "JSJ l%d\n", inst->address);
		break;
	    }
	    case IOUT:{
		fprintf(out, "OUT %d(19)\n", inst->address);
		break;
	    }
	    case ICHAR:{
		fprintf(out, "CHAR\n");
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

    if (mixal->label >= 0) {
	int count = fprintf(out, "l%d", mixal->label);

	fprintf(out, " ");
	for (int i = 0; i < 3 - count; ++i) {
	    fprintf(out, " ");
	}
    } else {
	fprintf(out, "    ");
    }
    fprintf(out, "HLT\n");
    fprintf(out, "ldbze OUT ERR(19)\n HLT\n");
    fprintf(out,
	    "ERR ALF \"DIVIS\"\n ALF \"ION B\"\n ALF \"Y ZER\"\n ALF \"O    \"\n");
    fprintf(out, " END 2000\n");
}
