#include <stdio.h>
#include <string.h>
#include <stdlib.h> //for memory allocation (malloc, calloc, realloc, etc)
#include <stdint.h>

short int instructionMemory[1024];
int8_t dataMemory[2048];
int pc = 0;

#define MAX 15

// void fetch(pc){

    
// }

// void decode(short int instruction){

    
// }


char **split(char *str,char *sep,int *count);

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
                
            for(int i = 0; i<count; i++) 
                free(strings[i]);
            
            free(strings);
        }
         int opcode, operand1, operand2;
    while (fscanf(fptr, "%d %d %d", &opcode, &operand1, &operand2) == 4) {
        instructionMemory[pc++] = (opcode << 12) | (operand1 << 6) | operand2;
        printf("%d\n", instructionMemory[pc-1]);
    }

    

        fclose(fptr);
    
    


    return 0;
}
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






//compile and run command: gcc -o main src/main.c && ./main