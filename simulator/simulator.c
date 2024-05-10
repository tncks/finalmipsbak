/* LC-2K Instruction-level simulator */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

enum Operator {
    ADD,
    NOR,
    LW,
    SW,
    BEQ,
    JALR,
    HALT,
    NOOP,
    FILL,
    UNKNOWN
};

typedef struct OperatorAndArgs {
    enum Operator op;
    int32_t arg0, arg1, arg2;
} OperatorAndArgsType;

void printState(stateType *);
int convertNum(int num);
OperatorAndArgsType n2o(int32_t n);

const char operatorNames[9][6] = {"add", "nor", "lw", "sw", "beq", "jalr", "halt", "noop", "fill"};

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    
    // Begin here

    // initialize all reg to 0

    for (uint8_t i = 0; i < 8; i++)
        state.reg[i] = 0;

    int8_t isHalt = false;
    int32_t cnt = 0;
    while (1) {
        int8_t needAddOne = 1;
        printState(&state);
        cnt++;
        if (cnt > 10000) {
            printf("Force stop\n");
            return 1;
        }
        OperatorAndArgsType ops = n2o(state.mem[state.pc]);
        switch (ops.op) {
            case ADD:
                state.reg[ops.arg2] = state.reg[ops.arg0] + state.reg[ops.arg1];
                break;
            case NOR:
                state.reg[ops.arg2] = ~(state.reg[ops.arg0] | state.reg[ops.arg1]);
                break;
            case LW:
                if (state.reg[ops.arg0] + ops.arg2 < 0 || state.reg[ops.arg0] + ops.arg2 > 65535)
                    return 1;
                state.reg[ops.arg1] = state.mem[state.reg[ops.arg0] + ops.arg2];
                break;
            case SW:
                if (state.reg[ops.arg0] + ops.arg2 < 0 || state.reg[ops.arg0] + ops.arg2 > 65535)
                    return 1;
                state.mem[state.reg[ops.arg0] + ops.arg2] = state.reg[ops.arg1];
                break;
            case BEQ:
                if (state.reg[ops.arg0] == state.reg[ops.arg1])
                    state.pc += ops.arg2;
                break;
            case JALR:
                state.reg[ops.arg1] = state.pc + 1;
                state.pc = state.reg[ops.arg0];
                needAddOne = 0;
                break;
            case HALT:
                isHalt = true;
                break;
            case NOOP:
                break;
            default:
                break;
        }
        state.pc += needAddOne;
        if (isHalt)
            break;
    }
    printf("machine halted\ntotal of %d instructions executed\nfinal state of machine:\n", cnt);
    printState(&state);

    // End


    return(0);
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

int convertNum(int num)
{
	/* convert a 16-bit number into a 32-bit Linux integer */
	if (num & (1 << 15)) {
		num -= (1 << 16);
	}
	return (num);
}

OperatorAndArgsType n2o(int32_t n) {
    OperatorAndArgsType res;
    res.op = (n & ((1 << 25) - (1 << 22))) >> 22;
    res.arg0 = (n & ((1 << 22) - (1 << 19))) >> 19;
    res.arg0 = convertNum(res.arg0);
    res.arg1 = (n & ((1 << 19) - (1 << 16))) >> 16;
    res.arg1 = convertNum(res.arg1);
    res.arg2 = n & ((1 << 16) - 1);
    res.arg2 = convertNum(res.arg2);
    return res;
}
