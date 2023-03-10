#include "constvar.h"

// data structure for attributes
union AttributeVal
{
    int true_lable;     // true lable pointer
    int false_lable;    // false lable pointer
    int begin_lable;    // begin lable pointer
    int next_lable;     // next lable pointer
    IDTABLE *id_ptr;    // id pointer
    EXPVAL *tmp_ptr;    // temporary variable pointer
    EXPVAL imm;         // immediate number value
};

enum AttributeType{TRUE_LABLE, FALSE_LABLE, BEGIN_LABLE, NEXT_LABLE, ID_PTR, TMP_PTR, IMM_VAL};

typedef struct AttributeNode
{
    enum AttributeType type;
    union AttributeVal val;
    struct AttributeNode *next;
}AttributeNode;

extern AttributeNode* lookupAttribute(AttributeNode *attribute, enum AttributeType type);
extern AttributeNode *installAttribute(AttributeNode *attribute, AttributeNode *head);

// data structure for Instruction
enum BinaryOperator{ADD, SUB, MUL, DIV, AND, OR, GT, GE, LT, LE, EQ};
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
    int lable;
}GotoInstr;

typedef struct ConditionalGotoInstruction
{
    AttributeNode *condition;
    int lable;
}ConGotoInstr;

union InstructionVal
{
    BinInstr binInstr;
    UnaInstr unaInstr;
    DupInstr dupInstr;
    GotoInstr gotoInstr;
    ConGotoInstr conGotoInstr;
};

enum InstructionType{BIN, UNA, DUP, GOTO, CGOTO};
typedef struct Instruction
{
    enum InstructionType type;
    union InstructionVal val;
}Instr;

#define MAX_CODE 1024
Instr code[MAX_CODE];
int pc;