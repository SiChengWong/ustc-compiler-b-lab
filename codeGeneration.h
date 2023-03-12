#include "constvar.h"

// data structure for attributes
union AttributeVal
{
    int *true_label;     // true label pointer
    int *false_label;    // false label pointer
    int *begin_label;    // begin label pointer
    int *next_label;     // next label pointer
    IDTABLE *id_ptr;    // id pointer
    EXPVAL *tmp_ptr;    // temporary variable pointer
    EXPVAL imm;         // immediate number value
};

enum AttributeType{TRUE_label, FALSE_label, BEGIN_label, NEXT_label, ID_PTR, TMP_PTR, IMM_VAL};

typedef struct AttributeNode
{
    enum AttributeType type;
    union AttributeVal val;
    struct AttributeNode *next;
}AttributeNode;

extern AttributeNode* lookupAttribute(AttributeNode *attribute, enum AttributeType type);
extern AttributeNode *installAttribute(AttributeNode *attribute, AttributeNode *head);

// data structure for Instruction
enum BinaryOperator{ADD, SUB, MUL, DIV, AND, OR, GT, GE, LT, LE, EQ, NE};
typedef struct BinaryOperationInstruction
{
    AttributeNode *x, *y, *z;
    enum BinaryOperator op;
}BinInstr;

enum UnaryOperator{NEG, NOT};
typedef struct UnaryOperationInstruction
{
    AttributeNode *x, *y;
    enum UnaryOperator op;
}UnaInstr;

typedef struct DuplicationInstruction
{
    AttributeNode *x, *y;
}DupInstr;

typedef struct GotoInstruction
{
    int *label;
}GotoInstr;

typedef struct ConditionalGotoInstruction
{
    AttributeNode *condition;
    int *label;
}ConGotoInstr;

typedef struct PrintInstruction
{
    AttributeNode *expression;
}PrintInstr;


union InstructionVal
{
    BinInstr binInstr;
    UnaInstr unaInstr;
    DupInstr dupInstr;
    GotoInstr gotoInstr;
    ConGotoInstr conGotoInstr;
    PrintInstr printInstr;
};

enum InstructionType{BIN, UNA, DUP, GOTO, CGOTO, PRINT};
typedef struct Instruction
{
    enum InstructionType type;
    union InstructionVal val;
}Instr;

#define MAX_CODE 1024
Instr code[MAX_CODE];       // code stack
int pc;                     // program counter