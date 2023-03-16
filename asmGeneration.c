#include "constvar.h"
#include "stdio.h"
#include "malloc.h"

AttributeNode *reg[3] = {NULL, NULL, NULL};   // content of regisiters x5,x6,x7
IdPos *idPosTable = NULL;
int asm_pc = 0;
int esp = 0;        // virtual esp for id position caculation, assuming ebp = 0

/// @brief add up imm and rs1, rd holds the result
/// @param rd dest register
/// @param rs1 src register
/// @param imm immidate number
void addi(int rd, int rs1, int imm){
    asmCode[asm_pc].type = ADDI;
    asmCode[asm_pc].operand1.type = 0;
    asmCode[asm_pc].operand1.val = rd;
    asmCode[asm_pc].operand2.type = 0;
    asmCode[asm_pc].operand2.val = rs1;
    asmCode[asm_pc].operand3.type = 1;
    asmCode[asm_pc].operand3.val = imm;
    asm_pc++;
}

/// @brief load a word from stack at offset(rs1), rd holds the result
/// @param rd dest register
/// @param offset offset from rs1
/// @param rs1 src register
void lw(int rd, int offset, int rs1){
    asmCode[asm_pc].type = LW;
    asmCode[asm_pc].operand1.type = 0;
    asmCode[asm_pc].operand1.val = rd;
    asmCode[asm_pc].operand2.type = 1;
    asmCode[asm_pc].operand2.val = offset;
    asmCode[asm_pc].operand3.type = 0;
    asmCode[asm_pc].operand3.val = rs1;
    asm_pc++;
}

/// @brief load immediate number into rd
/// @param rd dest register
/// @param imm immediate number
void li(int rd, int imm){
    asmCode[asm_pc].type = LI;
    asmCode[asm_pc].operand1.type = 0;
    asmCode[asm_pc].operand1.val = rd;
    asmCode[asm_pc].operand2.type = 1;
    asmCode[asm_pc].operand2.val = imm;
    asm_pc++;
}

/// @brief look up id position in stack
/// @param id id pointer in id table
/// @return id offset from base pointer in stack
int lookupIdPos(IDTABLE *id){
    IdPos *p = idPosTable;
    IdPos *q = NULL;
    while (p != NULL && p->id != id)
    {
        q = p;
        p = p->next;
    }
    if (p == NULL)
    {
        // id isn't in stack, allocate space for id
        addi(2, 2, -4);
        esp -= 4;
        // link new idPos node
        p = (IdPos*)malloc(sizeof(IdPos));
        p->id = id;
        p->next = NULL;
        p->offset = esp;        
        // empty id position table
        if (q == NULL)
            idPosTable = p;
        else
            q->next = p;
        return esp;
    }
    // id is in stack already
    return p->offset;
}

/// @brief find an available temporary register
/// @return available register, or -1 when no register available
int availableRegister(){
    for (int i = 0; i < 3; i++)
    {
        if (reg[i] == NULL)
        {
            return i+5;
        }
    }
    return -1;
}

int binInstr2asm(Instr tac_code){
    switch (tac_code.val.binInstr.y->type)
    {
    case ID_PTR:
        // load id value into register
        int rd = availableRegister();
        if (rd == -1) return -1;
        lw(availableRegister(), lookupIdPos(tac_code.val.binInstr.y->val.id_ptr), 1);
        reg[rd] = tac_code.val.binInstr.y;
        break;
    case IMM_VAL:
        // load immediate value into register
        li(availableRegister(), tac_code.val.binInstr.y->val.imm.val.intval);
        break;
    }
    
}