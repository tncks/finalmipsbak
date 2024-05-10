/* Student ID : 2022094839  */
/* Assembler code for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
#define MAXDATANUMBER 1000
#define NUMLABELS 1000
#define LABELSIZE 7

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

//returns the corresponding address line for the given label
int getaddress(char *, int, char [NUMLABELS][LABELSIZE], int label_addy[]);

//returns 1 if the character is uppercase
int isUpper(char *);

//returns 1 if the label is defined
int isDef(char *, int, char [NUMLABELS][LABELSIZE]);

//returns 1 if the reg range is valid (0-7 handling, dealing with range cases)
int judge_valid_reg(const char*);



int
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    //array of labels and their corresponding address
    char arrayoflabels[NUMLABELS][LABELSIZE];
    int label_addy[NUMLABELS];
    int num_labels = 0;

    int instruction;


    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    //number of instructions in the file(text, data, symbol, relocation)
    int t = 0;
    int d = 0;
    int s = 0;
    int r = 0;

    //number of defined globals
    int glob = 0;

    //global labels
    char defglobals[NUMLABELS][LABELSIZE];//defined globals
    char globals[NUMLABELS][LABELSIZE];//all globals
    char symbol[NUMLABELS][2];//for all globals, the associated code of "T", "D", or "U"

    int symbol_offset[NUMLABELS];//the line offset from the start of the T/D sections


    //array to hold the data values
    int data[MAXDATANUMBER];

	/* First: Phase-1 label calculation */
    //First pass: calculate the address for every symbolic label, error check the labels

    for(int addy = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++addy ){

      char code_type = '\0';
      //check for the opcode validity: check the case of unknown opcode
      {
          char *valid_opcodes[] = {"add", "nor", "lw", "sw", "beq", "jalr", "halt", "noop"};
          char *valid_additional_custom_instruction_code = ".fill";
          int c = -1;  
          if( strcmp(opcode, valid_additional_custom_instruction_code) != 0) {
              for(int i=0; i < sizeof(valid_opcodes) / sizeof(valid_opcodes[0]); i++) {
                  if( strcmp(opcode, valid_opcodes[i]) == 0){
                      c = 1;  // case-> match with opcode, so mark it valid
                      switch(i) {
                          case 0:  // add nor
                          case 1:
                              code_type='R';
                              break;
                          case 2:  // lw sw beq
                          case 3:
                          case 4:
                              code_type='I';
                              break;
                          case 5:  // jalr
                              code_type='J';
                              break;
                          case 6:  // halt noop
                          case 7: 
                              code_type='O';
                              break;
                          default:
                              exit(1);
                      }
                      break;
                   }
              }            
          }
          else {
              c = 0; // case -> match with .fill instruction code, so mark it valid
          }

          if (c <= -1) exit(1);
          
      }
      //check for the all arguments reg numeric value range validity
      {
          int r;

          if(code_type == 'R') {
              r = judge_valid_reg(arg0);
              if(r == 0) exit(1);
              r = judge_valid_reg(arg1);
              if(r == 0) exit(1);
              r = judge_valid_reg(arg2);
              if(r == 0) exit(1);
          }
          if(code_type == 'I' || code_type == 'J') {
              r = judge_valid_reg(arg0);
              if(r == 0) exit(1);
              r = judge_valid_reg(arg1);
              if(r == 0) exit(1);
          }



      }
    

      //only enters this part if the label exists
      if( strcmp(label, "")){
        //check for a duplicate label
        for(int i=0; i < (num_labels); ++i){
          if(!strcmp(label, arrayoflabels[i]))  exit(1);
        }

        //if uppercase and not in the .fill then "T"
        //if uppercase and in .fill then "D"
        if(isUpper(label) && (strcmp(opcode, ".fill")) != 0){
          strcpy(defglobals[glob], label);
          ++glob;
          strcpy(globals[s], label);
          strcpy(symbol[s], "T");

          symbol_offset[s] = addy;

          ++s;

        }
        else if(isUpper(label) && !strcmp(opcode, ".fill")){
          strcpy(defglobals[glob], label);
          ++glob;
          strcpy(globals[s], label);
          strcpy(symbol[s], "D");

          symbol_offset[s] = addy;

          ++s;
        }

        //else its local
        //store the label into the array
        // limit MAX size of characters upto 6.
        if(strlen(label) >= LABELSIZE)  exit(1);

        strcpy(arrayoflabels[num_labels], label);
        label_addy[num_labels++] = addy;
      }

    } // end of first pass

    //Rewind
    rewind(inFilePtr);

    //relocation table
    char rel_opcode[NUMLABELS][5];
    char rel_label[NUMLABELS][LABELSIZE];
    int rel_offset[NUMLABELS];

    //array to hold the instructions
    int text_table[1000];


	/* Second: Phase-2 generate machine codes to outfile */
    //Second Pass: generate a machine-language instruction for each line of assembly
    for(int PC = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++PC){
      //for add
      if(!strcmp(opcode, "add")){
        instruction = (0 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2));
      }

      //for nor
      else if(!strcmp(opcode, "nor")){
        instruction = (1 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | (atoi(arg2));
      }

      //for any of the i-type instructions
      else if(!strcmp(opcode, "lw") || !strcmp(opcode, "sw") || !strcmp(opcode, "beq")){
        //check if its a symbolic or numeric address, and determine the offset
        int offset;

        //for lw
        if(!strcmp(opcode, "lw")){
          //if its a label offset
          if(!isNumber(arg2)) {
            strcpy(rel_opcode[r], "lw");
            strcpy(rel_label[r], arg2);
            rel_offset[r] = PC;
            ++r;
            //if its a defined global, then save to the symbol table
            if(!isDef(arg2, glob, defglobals) && isUpper(arg2)){
              offset = 0;
              strcpy(globals[s], arg2);
              strcpy(symbol[s], "U");
              symbol_offset[s] = 0;
              ++s;
            }

            else{
              offset = getaddress(arg2, num_labels, arrayoflabels, label_addy);
            }
          }
          //if its a number offset
          else{
            offset = atoi(arg2);
            if( offset < -32768 || offset > 32767){
              exit(1);
            }
          }
          //mask in case of it being negative
          offset = offset & 0xFFFF;
          instruction = (2 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | offset;
        }

        //for sw
        else if(!strcmp(opcode, "sw")){
          //if its a symbolic address
          if(!isNumber(arg2)) {
            strcpy(rel_opcode[r], "sw");
            strcpy(rel_label[r], arg2);
            rel_offset[r] = PC;
            ++r;
            //if its not a defined global
            if(!isDef(arg2, glob, defglobals) && isUpper(arg2)){
              offset = 0;


              strcpy(globals[s], arg2);
              strcpy(symbol[s], "U");
              symbol_offset[s] = 0;
              ++s;
            }
            else{
              offset = getaddress(arg2, num_labels, arrayoflabels, label_addy);
            }
          }
          //if its a regular address
          else{
            offset = atoi(arg2);
            if( offset < -32768 || offset > 32767){
              exit(1);
            }
          }
          //mask
          offset = offset & 0xFFFF;
          instruction = (3 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | offset;
        }

        //for beq
        else{
          if(!isNumber(arg2)) {
            //beq cant branch to an undefined global
            if(!isDef(arg2, glob, defglobals) && isUpper(arg2)){
              exit(1);
            }

            else{
              offset = getaddress(arg2, num_labels, arrayoflabels, label_addy);
              offset = offset-PC-1;
            }
          }
          else{
            offset = atoi(arg2);
            if( offset < -32768 || offset > 32767){
              exit(1);
            }
          }

          //mask
          offset = offset & 0xFFFF;
          instruction = (4 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16) | offset;
        }
      }

      //for jalr
      else if(!strcmp(opcode, "jalr")){
        instruction = (5 << 22) | (atoi(arg0) << 19) | (atoi(arg1) << 16);
      }
      //for halt
      else if(!strcmp(opcode, "halt")){
        instruction = (6 << 22);
      }
      //for noop
      else if(!strcmp(opcode, "noop")){
        instruction = (7 << 22);
      }

      if(strcmp(opcode, ".fill")){
        text_table[t] = instruction;
        ++t;
      }

      //for fill
      if(!strcmp(opcode, ".fill")){
        int value;
        //value of data if its a label
        if(!isNumber(arg0)){
          strcpy(rel_opcode[r], ".fill");
          strcpy(rel_label[r], arg0);
          rel_offset[r] = PC;
          ++r;
          if(!isDef(arg0, glob, defglobals) && isUpper(arg0)){
            value = 0;
            strcpy(globals[s], arg2);
            strcpy(symbol[s], "U");
            ++s;
          }
          else{
            value = getaddress(arg0, num_labels, arrayoflabels, label_addy);
          }
        }
        //value of data if its a number
        else{
          value = atoi(arg0);
        }
        //pushback the data array
        data[d] = value;
        ++d;
      }

    } // end of second pass


    //PRINTING THE HEADER
    //printf("%d %d %d %d\n", t, d, s, r);

    //PRINTING THE TEXT SECTION
    for(int i=0; i < t; ++i){
      fprintf(outFilePtr, "%d\n", text_table[i]);
    }

    //PRINTING THE DATA SECTION
    for(int i=0; i < d; ++i){
      fprintf(outFilePtr, "%d\n", data[i]);
    }

    //PRINTING THE SYMBOL TABLE
    for(int i=0; i < s; ++i){
      fprintf(outFilePtr, "%s ", globals[i]);
      if(!strcmp(symbol[i], "T")){
        fprintf(outFilePtr, "%s %d\n", symbol[i], symbol_offset[i]);
      }
      else if(!strcmp(symbol[i], "D")){
        fprintf(outFilePtr, "%s %d\n", symbol[i], symbol_offset[i]-t);
      }
      else{
        fprintf(outFilePtr, "%s %d\n", symbol[i], 0);
      }
    }

    //PRINTING THE RELOCATION TABLE
    /* 
    for(int i=0; i < r; ++i){
      if(!strcmp(rel_opcode[i], ".fill")){
        printf("%d %s %s\n", rel_offset[i]-t, rel_opcode[i], rel_label[i]);
      }
      else{

        printf("%d %s %s\n", rel_offset[i], rel_opcode[i], rel_label[i]);
      }
    }
    */

    exit(0);

} // END of main

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int getaddress(char *label, int num_labels, char arrayoflabels[NUMLABELS][LABELSIZE], int label_addy[NUMLABELS]){
  int count;
  //index through the array until you come across the corresponding label
  for(count=0; (count<num_labels && strcmp(label, arrayoflabels[count])) ; ++count){}

    //if the index is the size or larger than the number of labels then the label is invalid
    if(count>=num_labels){
    /*  //if the undefined label is global then the address is 0
      if(isUpper(label[0])){
        return 0;
      }*/
      //else undefined local label: invalid and exit
      exit(1);
    }

    //else use same index in the label address to find the address
    return (label_addy[count]);

}

int isUpper(char *string){
  return(*string <= 'Z' && *string >= 'A');
}

int isDef(char *label, int num_labels, char arrayoflabels[NUMLABELS][LABELSIZE]){
  int count;
  for(count=0; (count<num_labels && strcmp(label, arrayoflabels[count])); ++count){}
  if(count>=num_labels){
    return 0;
  }
  return 1;
}

// case handling, dealing with some cases
int judge_valid_reg(const char* arg) {
    int reg = -1;
    reg = atoi(arg);
    if (reg < 0 || reg > 7) {
        return 0;
    }
    while (*arg != '\0') {
        if ((*arg) < '0' || (*arg) > '7') {
            return 0;
        }
        arg++;
    }
    return 1;
}


