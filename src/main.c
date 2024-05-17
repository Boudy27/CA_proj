#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX 1024
#define MASK2 0x80
int INSTRUCTION_COUNT = 0;
int fetchCount = 0;
int decodeCount = 0;
int executeCount = 0;
bool BEQZ_FLAG = false; 
bool BEQZ_FLAG2 = false; 
bool JUMP_FLAG = false;
bool JUMP_FLAG2 = false;

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
uint16_t pc = 0; 

struct PipelineRegister {
    char* instruction;
    char opcode[5];
    char dReg[7];
    char s2Reg[7];
    char immediate[7];
    char format;
} IF_ID, ID_EX;

uint8_t statusRegister; 
int8_t registerFile [64]; 

///////////////////methods///////////////
char **split(char *str,char *sep,int *count);
char* getRegIndexInt(int num);
char* convertImmediateToBinary(int num);
char* intToBinary8(int num);
char* intToBinary4(int num);
int getReg(char **strings,int index);
int binaryToInt(char* binary);
int signedBinaryToInt(const char *binary_string);


void encode(FILE *fptr, char buffer[MAX]);

void fetch();
void decode();
void execute();

void decodeHelper(char* instruction, struct PipelineRegister* reg);
void executeHelper(char* opcode, char* dReg, char* s2Reg, char* immediate);


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
bool isBranch(char* opcode);
bool isJump(char* opcode);

////////////////////////////////////////

int main() {
    FILE *fptr;
    fptr = fopen("assembly.txt", "r");
    char buffer[MAX];

    if(fptr == NULL) 
        printf("Error opening file\n");

    else 
        encode(fptr, buffer);



    int cycles = 1;
    fetchCount = INSTRUCTION_COUNT;
    decodeCount = INSTRUCTION_COUNT;
    executeCount = INSTRUCTION_COUNT;

    while(fetchCount!=0 || decodeCount!=0 || executeCount!=0){
        
        if(cycles == 1){
            fetch();
        }
        else if(cycles==2){
            decode();
            if(fetchCount!=0){
                fetch();
            }
        }
        else{
            if(executeCount!=0){
                execute();
            }
            if(decodeCount!=0 ){
                decode();
            }
            if(fetchCount!=0){
                fetch();
            }
        }
        if(decodeCount == 0){
            IF_ID.instruction = NULL;
        }
        printf("---END OF CYCLE %d---\n",cycles);
        cycles++;
    }

    //print program has ended
    printf("------------Program has ended----------\n");
    //print cycles
    printf("Total number of cycles: %d\n",cycles-1);
    //print pc
    printf("PC: %d\n",pc);
    //print SREG
    printf("SREG: %s\n",intToBinary8(statusRegister));
    //print registerFile if the register is not 0
    for(int i = 0; i<64; i++){
        if(registerFile[i]!=0){
            printf("registerFile[%d]: %d\n",i,registerFile[i]);
        }
    }

    //print dataMemory if the dataMemory is not 0
    for(int i = 0; i<2048; i++){
        if(dataMemory[i]!=0){
            printf("dataMemory[%d]: %d\n",i,dataMemory[i]);
        }
    }

    //print instructions if not null
    for(int i = 0; i<INSTRUCTION_COUNT; i++){
        if(instructionMemory[i]!=NULL){
            printf("Instruction %d: %s\n",i,instructionMemory[i]);
        }
    }

    printf("NOTE: REGISTER/DATA MEMORY/INSTRUCTION MEMORY VALUES ARE ONLY PRINTED IF THEY ARE NOT 0/NULL\n");

    

    return 0;
}

void encode(FILE *fptr, char buffer[MAX]){

    for(int i = 0; i<2048; i++){
    dataMemory[i] = 0;
    };

    for(int i = 0; i<64; i++){
        registerFile[i] = 0;
    };

    int i = 0;
    while(fgets(buffer, MAX, fptr)) {
            INSTRUCTION_COUNT ++;
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
            }
            else{
                dReg = getRegIndexInt(getReg(strings,1));
                s1Reg = getRegIndexInt(getReg(strings,1));
                s2Reg = getRegIndexInt(getReg(strings,2));
            }
                    
            //concatenate binary strings to form instruction
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
    if((isBranch(ID_EX.opcode) && BEQZ_FLAG) || (isJump(ID_EX.opcode) && JUMP_FLAG)){
         printf("Control Hazard: Stalling fetch\n");
         ID_EX.instruction = NULL;
         BEQZ_FLAG = false;
         JUMP_FLAG = false;
    }
   else{
    if(instructionMemory[pc] == NULL){
        return;
    }else{
    printf("Fetching: %s\n",instructionMemory[pc]);
    IF_ID.instruction = instructionMemory[pc];
    pc++;
    fetchCount--;
    }
   }
}

void decode() {
    if((isBranch(ID_EX.opcode) && BEQZ_FLAG) || (isJump(ID_EX.opcode) && JUMP_FLAG)){
        printf("Control Hazard: Stalling decode\n");
        ID_EX.instruction = NULL;
        IF_ID.instruction = instructionMemory[pc];
    }
    else if(BEQZ_FLAG2 || JUMP_FLAG2){
        printf("Control Hazard: Stalling decode\n");
        BEQZ_FLAG2 = false;
        JUMP_FLAG2 = false;
        return;
    }
    else
    if (IF_ID.instruction != NULL) {
    // Decode instruction and store relevant information
    // (opcode, registers, immediate value) in ID/EX register
    decodeHelper(IF_ID.instruction, &ID_EX);
    decodeCount--;

    printf("Decoding  %s\n",IF_ID.instruction);
  }
}

void execute() {
  // Check if there's an instruction in the ID/EX pipeline register
  if (ID_EX.instruction != NULL) {
    
    executeCount--;
    executeHelper(ID_EX.opcode, ID_EX.dReg, ID_EX.s2Reg, ID_EX.immediate);
    printf("Executing %s\n",ID_EX.instruction);

    // Clear ID/EX register for the next decode
    ID_EX.instruction = NULL;
  }
}

void decodeHelper(char* instruction, struct PipelineRegister* reg){
    for(int i = 0; i<4; i++){
        reg->opcode[i] = instruction[i];
    }
    reg->opcode[4] = '\0';

    if(strcmp(reg->opcode, "0000") == 0 || strcmp(reg->opcode, "0001") == 0 || strcmp(reg->opcode, "0010") == 0 || strcmp(reg->opcode, "0101") == 0 || strcmp(reg->opcode, "0110") == 0 || strcmp(reg->opcode, "0111") == 0)
    {
        reg->format = 'R';
    }
    else{
        reg->format = 'I';
    }

    for(int i = 4; i<10; i++){
        reg->dReg[i-4] = instruction[i];
    }
    reg->dReg[6] = '\0';


    if(reg->format == 'R'){
        for(int i = 10; i<16; i++){
            reg->s2Reg[i-10] = instruction[i];
        }
            reg->s2Reg[6] = '\0';
    }
    else{
        for(int i = 10; i<16; i++)
        {
            reg->immediate[i-10] = instruction[i]; 
        }
        reg->immediate[6] = '\0';
    }
    
    ID_EX.instruction = instruction;

}

void executeHelper(char* opcode, char* dReg, char* s2Reg, char* immediate){
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
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    printf("registerFile[%d] before change: %d\n",s2RegInt,registerFile[s2RegInt]);

    int carryOverflow = check_carry_overflow(registerFile[dRegInt], registerFile[s2RegInt], '+') ;
    int negative = NegativeSign(registerFile[dRegInt] + registerFile[s2RegInt]);
    int sign = SignFlag((carryOverflow & 0b10),negative);
    int zero = ZeroFlag(registerFile[dRegInt] + registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] + registerFile[s2RegInt];
    printf("registerFile[%d] after ADDITION: %d\n",dRegInt,registerFile[dRegInt]);

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
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    printf("registerFile[%d] before change: %d\n",s2RegInt,registerFile[s2RegInt]);

    int carryOverflow = check_carry_overflow(registerFile[dRegInt], registerFile[s2RegInt], '-') ;
    int negative = NegativeSign(registerFile[dRegInt] - registerFile[s2RegInt]);
    int sign = SignFlag((carryOverflow & 0b10),negative);
    int zero = ZeroFlag(registerFile[dRegInt] - registerFile[s2RegInt]);

    
    registerFile[dRegInt] = registerFile[dRegInt] - registerFile[s2RegInt];
    printf("registerFile[%d] after SUBTRACTION: %d\n",dRegInt,registerFile[dRegInt]);

    statusRegister |= ((carryOverflow & 0b10) << 2);
    statusRegister |= ((negative) << 2);
    statusRegister |= (sign << 1);
    statusRegister |= (zero << 0);
}

void mul(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    printf("registerFile[%d] before change: %d\n",s2RegInt,registerFile[s2RegInt]);

    int negative = NegativeSign(registerFile[dRegInt] * registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] * registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] * registerFile[s2RegInt];
    printf("registerFile[%d] after MULTIPLICATION: %d\n",dRegInt,registerFile[dRegInt]);


    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void and(char* dReg, char* s1Reg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    printf("registerFile[%d] before change: %d\n",s2RegInt,registerFile[s2RegInt]);

    int negative = NegativeSign(registerFile[dRegInt] & registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] & registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] & registerFile[s2RegInt];
    printf("registerFile[%d] after BITWISE AND: %d\n",dRegInt,registerFile[dRegInt]);

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void or(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int s2RegInt = binaryToInt(s2Reg);
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    printf("registerFile[%d] before change: %d\n",s2RegInt,registerFile[s2RegInt]);

    int negative = NegativeSign(registerFile[dRegInt] | registerFile[s2RegInt]);
    int zero = ZeroFlag(registerFile[dRegInt] | registerFile[s2RegInt]);

    registerFile[dRegInt] = registerFile[dRegInt] | registerFile[s2RegInt];
    printf("registerFile[%d] after BITWISE OR: %d\n",dRegInt,registerFile[dRegInt]);

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void ldi(char* dReg, char* immediate){
    int immediateInt = signedBinaryToInt(immediate);
    int dRegInt = binaryToInt(dReg);
    printf("registerFile[%d] before change: %d\n",dRegInt,registerFile[dRegInt]);
    registerFile[dRegInt] = immediateInt;
    printf("registerFile[%d] after LOADING into it: %d\n",dRegInt,registerFile[dRegInt]);
}

void beqz(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = signedBinaryToInt(immediate);
    if(registerFile[dRegInt] == 0){
        printf("PC before change: %d\n",pc);
        pc = pc + immediateInt - 1;
        printf("PC after BRANCHING: %d\n",pc);
        BEQZ_FLAG = true;
        BEQZ_FLAG2 = true;

        fetchCount = INSTRUCTION_COUNT - pc;
        decodeCount = INSTRUCTION_COUNT - pc;
        executeCount = INSTRUCTION_COUNT - pc;

    }
}

void jr(char* dReg, char* s2Reg){
    int dRegInt = binaryToInt(dReg);
    int dregData = registerFile[dRegInt];
    int s2RegInt = binaryToInt(s2Reg);
    int s2RegData = registerFile[s2RegInt];
    printf("PC will JUMP to: %s\n",strcat(intToBinary8(dregData),intToBinary8(s2RegData)));
    printf("PC before JUMPING: %d\n",pc);
    pc = binaryToInt(strcat(intToBinary8(dregData),intToBinary8(s2RegData))); 
    printf("PC after JUMPING: %d\n",pc); 
    JUMP_FLAG = true;
    JUMP_FLAG2 = true;
    
    fetchCount = INSTRUCTION_COUNT - pc;
    decodeCount = INSTRUCTION_COUNT - pc;
    executeCount = INSTRUCTION_COUNT - pc;
}

void slc(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    printf("registerFile[%d] before CIRCULAR SHIFTING LEFT: %d\n",dRegInt,registerFile[dRegInt]);
    int immediateInt = binaryToInt(immediate); 
    if(immediateInt < 0){
       printf("Invalid shift amount\n");
       return;
    }
    int shiftAmount = immediateInt % 8; 
    int negative = NegativeSign((registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount)));

    int zero = ZeroFlag((registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount)));

    registerFile[dRegInt] = (registerFile[dRegInt] << shiftAmount) | (registerFile[dRegInt] >> (8 - shiftAmount));
    printf("registerFile[%d] after CIRCULAR SHIFTING LEFT: %d\n",dRegInt,registerFile[dRegInt]);

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void src(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    printf("registerFile[%d] before CIRCULAR SHIFTING RIGHT: %d\n",dRegInt,registerFile[dRegInt]);
    int immediateInt = binaryToInt(immediate); 
    if(immediateInt < 0){
       printf("Invalid shift amount\n");
       return;
    }
    int shiftAmount = immediateInt % 8; 
    int negative = NegativeSign((registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount)));
    int zero = ZeroFlag((registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount)));

    registerFile[dRegInt] = (registerFile[dRegInt] >> shiftAmount) | (registerFile[dRegInt] << (8 - shiftAmount));
    printf("registerFile[%d] after CIRCULAR SHIFTING RIGHT: %d\n",dRegInt,registerFile[dRegInt]);

    statusRegister |= ((negative) << 2);
    statusRegister |= (zero << 0);
}

void lb(char* dReg, char* immediate){
    int dRegInt = binaryToInt(dReg);
    int immediateInt = binaryToInt(immediate);
    printf("registerFile[%d] before LOADING FROM MEMORY: %d\n",dRegInt,registerFile[dRegInt]);
    registerFile[dRegInt] = dataMemory[immediateInt];
    printf("registerFile[%d] after LOADING FROM MEMORY: %d\n",dRegInt,registerFile[dRegInt]);
}

void sb(char* dReg,char* immediate){
    int dRegInt = binaryToInt(dReg);
    printf("registerFile[%d]: %d\n",dRegInt,registerFile[dRegInt]);
    int immediateInt = binaryToInt(immediate);
    printf("dataMemory[%d] before STORING INTO MEMORY: %d\n",immediateInt,dataMemory[immediateInt]);
    dataMemory[immediateInt] = registerFile[dRegInt];  
    printf("dataMemory[%d] after STORING INTO MEMORY: %d\n",immediateInt,dataMemory[immediateInt]);
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

    int len = strlen(string);

    *count = 0 ;

    int i=0;

    while(i<len)
    {
        while(i < len)
        {
            if(strchr(separators,string[i]) == NULL) 
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
    char* binary = (char*)malloc(5 * sizeof(char)); 
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
    binary[8] = '\0';
    
    for (int i = 7; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0'; 
        num >>= 1;
    }
    
    return binary;
}


int binaryToInt(char* binary) {
    int result = 0;
    int len = strlen(binary);
    for (int i = 0; i < len; i++) {
        result <<= 1; 
        if (binary[i] == '1') {
            result |= 1; 
        }
    }
    return result;
}

int signedBinaryToInt(const char *binary_string) {
  
  if (strlen(binary_string) != 6) {
    return -1;
  }

  int decimal_value = 0;
  for (int i = 0; i < 6; ++i) {
    int bit_value = binary_string[i] - '0'; 
    decimal_value += bit_value << (5 - i);
  }

  if (binary_string[0] == '1') {
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
    return (a == 0) ? 1 : 0; 
}

bool isBranch(char* opcode) {
    return strcmp(opcode, "0100") == 0;
}

bool isJump(char* opcode) {
    return strcmp(opcode, "0111") == 0;
}