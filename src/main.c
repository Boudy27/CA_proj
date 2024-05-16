#include <stdio.h>
#include <string.h>
#include <stdlib.h> //for memory allocation (malloc, calloc, realloc, etc)
#include <stdint.h>

char *instructions[] = {
    "ADD",
    "SUB",
    "MUL",
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
char *instructionMemory[1024];

int8_t dataMemory[2048];

uint16_t pc = 0; //program counter
uint8_t statusRegister; //create a status register
int8_t registerFile [64]; //create a register file
uint8_t statusRegister = 0b00000000; 


#define MAX 1024
#define SCHAR_MAX 127// Maximum value for an unsigned 8-bit integer
#define SCHAR_MIN -128// Minimum value for a signed 8-bit integer
#define MASK 256 // Mask for the 9th bit (carry bit)
#define MASK2 0x80


///////////////////methods///////////////
char **split(char *str,char *sep,int *count);
char* getRegIndexInt(int num);
char* convertImmediateToBinary(int num);
char* intToBinary8(int num);
char* intToBinary4(int num);
int getReg(char **strings,int index);
int binaryToInt(char* binary);
int signedBinaryToInt(const char *binary_string);
// char* binaryToInt2sComplement(char* binary);


void encode(FILE *fptr, char buffer[MAX]);
void fetch();
void decode(char* instruction);
void execute(char* opcode, char* dReg, char* s2Reg, char* immediate);


void add(char* dReg, char* s2Reg);
void sub(char* dReg, char* s2Reg);
void mul(char* dReg, char* s2Reg);
void and(char* dReg, char* s1Reg, char* s2Reg);
void or(char* dReg, char* s2Reg);
void ldi(char* dReg, char* immediate);
void beqz(char* dReg, char* immediate);
void jr(char* dReg, char* s2Reg);
void slc(char* dReg,char* immediate);
void src(char* dReg,char* immediate);
void lb(char* dReg,char* immediate);
void sb(char* dReg, char* immediate);

int check_carry_overflow(int a, int b, char operation);
int NegativeSign(int num) ;
int SignFlag(int a , int b);
int ZeroFlag(int a);

////////////////////////////////////////

int main() {
    FILE *fptr;
    fptr = fopen("assembly.txt", "r");
    char buffer[MAX];

    if(fptr == NULL) 
        printf("Error opening file\n");

    else 
        encode(fptr, buffer);
     
    for(int i = 0; i<15; i++){
        printf("instructionMemory[%d]: %s\n",i,instructionMemory[i]);
    }

   for(int i = 0; i<1; i++){
        fetch();
    }
 
    // //print registerFile
    // for(int i = 0; i<10; i++){
    //     printf("registerFile[%d]: %d\n",i,registerFile[i]);
    // }

    // //print dataMemory
    // for(int i = 0; i<10; i++){
    //     printf("dataMemory[%d]: %d\n",i,dataMemory[i]);
    // }

    return 0;
}

//create encode function
void encode(FILE *fptr, char buffer[MAX]){

    for(int i = 0; i<2048; i++){
    dataMemory[i] = 0;
    };

    for(int i = 0; i<64; i++){
        if(i==0){
            registerFile[i] = 9;
            continue;
        }
        if(i==1){
            registerFile[i] = 9;
            continue;
        }
        
        registerFile[i] = 0;
    };

    int i = 0;
    while(fgets(buffer, MAX, fptr)) {
            int count;
            char **strings = split(buffer," ", &count);
    
            //get opcode
           int opcode;
           for(int i = 0; i<13; i++){
               if(strcmp(strings[0], instructions[i]) == 0){
                   opcode = i;
                   break;
               }
            }
            char *opcodeBinary = intToBinary4(opcode);


            //get format
            char format;
            if((opcode>=0 && opcode<=2) || (opcode>=5 && opcode<=7)){
                format = 'R';
            }
            else{
                format = 'I';
            }

            //get binary representation of registers and immediate values
            char* dReg = "";
            char* s1Reg = "";
            char* immediate = ""; 
            char* s2Reg = "";
            if(format=='I'){
                dReg = getRegIndexInt(getReg(strings,1));
                s1Reg = getRegIndexInt(getReg(strings,1));
                
                immediate = convertImmediateToBinary(atoi(strings[2]));
                printf("immediate: %s\n",immediate);

            }
            else{
                dReg = getRegIndexInt(getReg(strings,1));
                s1Reg = getRegIndexInt(getReg(strings,1));
                s2Reg = getRegIndexInt(getReg(strings,2));
            }
                    
            //concatenate binary strings to form instruction
            //allocate memory for instructionBinary
            char *instructionBinary = (char*)malloc(16 * sizeof(char));
            strcpy(instructionBinary, opcodeBinary); 
            strcat(instructionBinary, dReg);
            if(format=='R'){
                strcat(instructionBinary, s2Reg);
            }
            else{
                strcat(instructionBinary, immediate);
            }

            instructionMemory[i] = instructionBinary;
                i++;

            for(int i = 0; i<count; i++) 
                free(strings[i]);
            
            free(strings);
        }
        
        fclose(fptr); 
}


void fetch() {
                    
char *instruction = instructionMemory[pc];
decode(instruction); 
            pc++;
    printf("pc: %d\n",pc);
}

void decode(char* instruction){
    char opcode[5];
    char dReg[7];
    char s2Reg[7];
    char immediate[7];
    char format;
    char* temp;

    for(int i = 0; i<4; i++){
        opcode[i] = instruction[i];
    }
    opcode[4] = '\0';

    if(strcmp(opcode, "0000") == 0 || strcmp(opcode, "0001") == 0 || strcmp(opcode, "0010") == 0 || strcmp(opcode, "0101") == 0 || strcmp(opcode, "0110") == 0 || strcmp(opcode, "0111") == 0)
    {
        format = 'R';
    }
    else{
        format = 'I';
    }

    for(int i = 4; i<10; i++){
        dReg[i-4] = instruction[i];
    }
    dReg[6] = '\0';


    if(format == 'R'){
        for(int i = 10; i<16; i++){
            s2Reg[i-10] = instruction[i];
        }
            s2Reg[6] = '\0';
    }
    else{
        for(int i = 10; i<16; i++)
        {
            immediate[i-10] = instruction[i]; 
        }
        immediate[6] = '\0';
    }

    execute(opcode, dReg, s2Reg, immediate);
}

void execute(char* opcode, char* dReg, char* s2Reg, char* immediate){
        if(strcmp(opcode, "0000") == 0){
            add(dReg, s2Reg);
        }
        else if(strcmp(opcode, "0001") == 0){
            sub(dReg, s2Reg);
        }
        else if(strcmp(opcode, "0010") == 0){
            mul(dReg, s2Reg);
        }
        else if(strcmp(opcode, "0011") == 0){
            ldi(dReg, immediate);
        }
        else if(strcmp(opcode, "0100") == 0){
            beqz(dReg, immediate);
        }
        else if(strcmp(opcode, "0101") == 0){
            and(dReg,dReg, s2Reg);
        }
        else if(strcmp(opcode, "0110") == 0){
            or(dReg,s2Reg);
        }
        else if(strcmp(opcode, "0111") == 0){
            jr(dReg, s2Reg);
        }
        else if(strcmp(opcode, "1000") == 0){
            slc(dReg, immediate);
        }
        else if(strcmp(opcode, "1001") == 0){
            src(dReg, immediate);
        }
        else if(strcmp(opcode, "1010") == 0){
            lb(dReg, immediate);
        }
        else if(strcmp(opcode, "1011") == 0){
            sb(dReg,immediate);
        }
    }

void add(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);

    int carryOverflow = check_carry_overflow(registerFile[dRegInt], registerFile[s2RegInt], '+') ;
    int negative = NegativeSign(registerFile[dRegInt] + registerFile[s2RegInt]);
    int sign = SignFlag((carryOverflow & 0b10),negative);
    int zero = ZeroFlag(registerFile[dRegInt] + registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] + registerFile[s2RegInt];

    // Update status register
    statusRegister |= ((carryOverflow & 0b01) << 4);
    statusRegister |= ((carryOverflow & 0b10) << 2);
    statusRegister |= ((negative) << 2);
    statusRegister |= (sign << 1);
    statusRegister |= (zero << 0);
}

void sub(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);

    int carryOverflow = check_carry_overflow(registerFile[dRegInt], registerFile[s2RegInt], '-') ;
    int negative = NegativeSign(registerFile[dRegInt] - registerFile[s2RegInt]);
    int sign = SignFlag((carryOverflow & 0b10),negative);
    int zero = ZeroFlag(registerFile[dRegInt] - registerFile[s2RegInt]);

    
    registerFile[dRegInt] = registerFile[dRegInt] - registerFile[s2RegInt];

    statusRegister |= ((carryOverflow & 0b10) << 2);
    statusRegister |= ((negative) << 2);
    statusRegister |= (sign << 1);
    statusRegister |= (zero << 0);
}

void mul(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);

    int negative = NegativeSign(registerFile[dRegInt] * registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] * registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] * registerFile[s2RegInt];


    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void and(char* dReg, char* s1Reg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s1RegInt = binaryToInt(s1Reg);
    int s2RegInt = binaryToInt(s2Reg);
    int negative = NegativeSign(registerFile[s1RegInt] & registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] * registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[s1RegInt] & registerFile[s2RegInt];

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void or(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);
    int negative = NegativeSign(registerFile[dRegInt] | registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] | registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] | registerFile[s2RegInt];

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void ldi(char* dReg, char* immediate){
    int immediateInt = signedBinaryToInt(immediate);
    int dRegInt = binaryToInt(dReg);
    registerFile[dRegInt] = immediateInt;
}

void beqz(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    if(registerFile[dRegInt] == 0){
        pc = pc + immediateInt;
    }
}

void jr(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int dregData = registerFile[dRegInt];
    int s2RegInt = binaryToInt(s2Reg);
    int s2RegData = registerFile[s2RegInt];
 
    pc = binaryToInt(strcat(intToBinary8(dregData),intToBinary8(s2RegData)));  
}

void slc(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    int shiftAmount = immediateInt % 8; 
    int negative = NegativeSign((registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount)));

    int zero = ZeroFlag((registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount)));

    registerFile[dRegInt] = (registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount));

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void src(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    int shiftAmount = immediateInt % 8; 
    int negative = NegativeSign((registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount)));
    int zero = ZeroFlag((registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount)));

    registerFile[dRegInt] = (registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount));

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void lb(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    registerFile[dRegInt] = dataMemory[immediateInt];
}

void sb(char* dReg,char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    dataMemory[immediateInt] = registerFile[dRegInt];  
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
    char buffer[100];
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

//FOR OPCODE
char* intToBinary4(int num) {
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

char* convertImmediateToBinary(int num) {
  char* binary = (char*)malloc(7 * sizeof(char)); 
  if (binary == NULL) {
    printf("Memory allocation failed.\n");
    exit(1);
  }

  binary[6] = '\0'; // Null terminator

  // Handle negative numbers
  
    // Convert to positive using two's complement (add 2^n, where n is number of bits)
    num = (num & 0x3F) + (1 << 6); // Mask to 6 bits and add 2^6
  

  // Convert to binary string
  for (int i = 5; i >= 0; i--) {
    binary[i] = (num & 1) ? '1' : '0';
    num >>= 1;  // Right shift by one bit
  }

  return binary;
}


char* getRegIndexInt(int num) {
    char* binary = (char*)malloc(7 * sizeof(char));    if (binary == NULL) {
    printf("Memory allocation failed.\n");
    exit(1);
}
    binary[6] = '\0'; // Null terminator
    if(num < 0){
        //num is in 2's complement
        num = num & 0x3F; // Mask the number to 6 bits
        for (int i = 5; i >= 0; i--) {
            binary[i] = (num & 1) ? '1' : '0'; // Masking least significant bit
            num >>= 1; // Right shift by one bit
        }
    }
    else{
        num = num & 0x3F; // Mask the number to 6 bits
        for (int i = 5; i >= 0; i--) {
            binary[i] = (num & 1) ? '1' : '0'; // Masking least significant bit
            num >>= 1; // Right shift by one bit
        }
    }
    return binary;
}
//FOR REG DATA
char* intToBinary8(int num) {
    char* binary = (char*)malloc(9 * sizeof(char)); // Allocate memory for the binary string
    if (binary == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    binary[8] = '\0'; // Null terminator
    
    for (int i = 7; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0'; // Masking least significant bit
        num >>= 1; // Right shift by one bit
    }
    
    return binary;
}


// char* convertRegDataToBinary(int num) {
//   char* binary = (char*)malloc(7 * sizeof(char)); 
//   if (binary == NULL) {
//     printf("Memory allocation failed.\n");
//     exit(1);
//   }

//   binary[8] = '\0'; // Null terminator

//   // Handle negative numbers
  
//     num = (num & 0xFF) + (1 << 8); // Mask to 8 bits and add 2^8
  

//   // Convert to binary string
//   for (int i = 7; i >= 0; i--) {
//     binary[i] = (num & 1) ? '1' : '0';
//     num >>= 1;  // Right shift by one bit
//   }

//   return binary;
// }

int binaryToInt(char* binary) {
    int result = 0;
    int len = strlen(binary);
    for (int i = 0; i < len; i++) {
        result <<= 1; // Left shift by one bit
        if (binary[i] == '1') {
            result |= 1; // Set the least significant bit to 1
        }
    }
    return result;
}

int signedBinaryToInt(const char *binary_string) {
  
  if (strlen(binary_string) != 6) {
    // Handle error: Input string must be a 6-bit binary string
    return -1;
  }

  // Convert the binary string to an unsigned integer
  int decimal_value = 0;
  for (int i = 0; i < 6; ++i) {
    int bit_value = binary_string[i] - '0'; // Convert char to int (0 or 1)
    decimal_value += bit_value << (5 - i); // Apply bit shift based on position
  }

  // Check for negative numbers (MSB is 1)
  if (binary_string[0] == '1') {
    // Calculate the two's complement by subtracting from 2^n (where n is the number of bits)
    decimal_value = - (1 << 6) + decimal_value;
  }

  if(decimal_value > 31){
    return -1;
  }

  return decimal_value;
}


int check_carry_overflow(int a, int b, char operation) {
  int carry = 0;
  int overflow = 0;

  // Convert to signed char for arithmetic operations
  signed char sa = (signed char) a;
  signed char sb = (signed char) b;

  int result =0;

  if (operation == '+') {
    result = sa + sb;
  }else{
    result = sa - sb;
  }
    
    char arr[9] = "00000000";
    arr[9] = '\0';

    int temp = 0;
    arr[7] = (a & 0b00000001) && (b & 0b00000001) ? '1' : '0';
    
    for (int i = 1; i < 8; i++) {
        int index = strlen(arr) - i;
       
        temp = (((a >> i) & 1) & ((b >> i) & 1)) || (((a >> i) & 1) & (arr[index])) || (((b >> i) & 1) & (arr[index]));
        
        arr[index-1] = temp==1 ? '1' : '0';
    }

    signed char carry_prev = (result & MASK2) >> 7;
    
    overflow = (arr[0] ^ arr[1]);

    carry = arr[0] == '0'? 0 : 1;
    

  return (carry << 0) | (overflow << 1);
}

int NegativeSign(int num) {
    if(num < 0){
        return 1;
    }
    else{
        return 0;
    } 
}

int SignFlag(int a , int b) {
    return (a ^ b) ;  
}

int ZeroFlag(int a) {
    return (a == 0) ? 1 : 0; // Check if the number is zero
}

