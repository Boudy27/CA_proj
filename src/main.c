#include <stdio.h>
#include <string.h>
#include <stdlib.h> //for memory allocation (malloc, calloc, realloc, etc)
#include <stdint.h>

char *instructions[] = {
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "LDI",
    "BEQZ",
    "AND",
    "OR",
    "JR",
    "SLC",
    "SRC",
    "LB",
    "SB"
};
short int instructionMemory[1024];
int8_t dataMemory[2048];
int pc = 0;
int8_t registerFile [64];

#define MAX 15

// void fetch(){

    
// }

// void decode(short int instruction){

    
// }


char **split(char *str,char *sep,int *count);
char* intToBinary(int num);
int getReg(char **strings,int index);

int main() {

    FILE *fptr;

    fptr = fopen("assembly.txt", "r");

    char buffer[MAX];

    if(fptr == NULL) 
        printf("Error opening file\n");

    else {
        while(fgets(buffer, MAX, fptr)) {
            int count;
            char **strings = split(buffer," ", &count);
            //print strings

            for(int i = 0; i<count; i++) 
                printf("%s\n", strings[i]);

            
           int opcode;
           for(int i = 0; i<13; i++){
               if(strcmp(strings[0], instructions[i]) == 0){
                   opcode = i;
                   break;
               }
            }
            char format;
            if((opcode>=0 && opcode<=2) || (opcode>=5 && opcode<=7)){
                format = 'R';
            }
            else{
                format = 'I';
            }
            printf("format: %c\n",format);
            
          /*  if(format=='R'){
                char dRegTmp[2];
                
                if(strlen(strings[1])==2){
                    dRegTmp[0] = strings[1][1];
                    dRegTmp[1] = ' ';
                }
                else{
                    dRegTmp[0] = strings[1][1];
                    dRegTmp[1] = strings[1][2];
                }
                int dReg = atoi(dRegTmp);
                int s1Reg = atoi(dRegTmp);
            }*/
            
            if(format=='I'){
                int dReg = getReg(strings,1);
                int s1Reg = getReg(strings,1);
                int immediate = atoi(strings[2]);
            }
            else{
                int dReg = getReg(strings,1);
                int s1Reg = getReg(strings,1);
                int s2Reg = getReg(strings,2);
            }

            

            

            char *binary = intToBinary(opcode);
            printf("opcode: %s\n",binary);
            
            /////////////////////

                
            for(int i = 0; i<count; i++) 
                free(strings[i]);
            
            free(strings);
        }
        fclose(fptr);
    
    return 0;
}
}

int getReg(char **strings,int index){

    char RegTmp[2];
                
    if(strlen(strings[index])==2){
        RegTmp[0] = strings[index][1];
        RegTmp[1] = ' ';
    }
    else{
        RegTmp[0] = strings[index][1];
        RegTmp[1] = strings[index][2];
    }
    int Reg = atoi(RegTmp);
    return Reg;

}

char **split(char *string,char *separators,int *count){

    //get length of string
    int len = strlen(string);

    //counter for substrings
    *count = 0 ;

    int i=0;

    while(i<len)
    {
        while(i < len)
        {
            if(strchr(separators,string[i]) == NULL) //"To Do FINISHED"
                break;
            i++;
        }

        int old_i = i;
        while(i < len)
        {
            if(strchr(separators,string[i]) != NULL)
                break;
            i++;
        }

        if(i > old_i)
            (*count)++;
    }

    char **strings = malloc(sizeof(char *) * (*count));

    i = 0;
    char buffer[16384];
    int string_index =0;
    while(i<len)
    {
        while(i < len)
        {
            if(strchr(separators,string[i]) == NULL)
                break;
            i++;
        }

        int j = 0;
        while(i < len)
        {
            if(strchr(separators,string[i]) != NULL)
                break;
            
            buffer[j] = string[i];
            i++;
            j++;
        }

        if(j>0)
        {
            buffer[j] = '\0';
            int to_allocate = sizeof(char)*(strlen(buffer) + 1);
            strings[string_index] = malloc(to_allocate);
            strcpy(strings[string_index],buffer);
            string_index++;
        }

    }

    return strings;

}

char* intToBinary(int num) {
    char* binary = (char*)malloc(5 * sizeof(char)); // Allocate memory for the binary string
    if (binary == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    binary[4] = '\0'; // Null terminator
    
    for (int i = 3; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0'; // Masking least significant bit
        num >>= 1; // Right shift by one bit
    }
    
    return binary;
}




//compile and run command: gcc -o main src/main.c && ./main