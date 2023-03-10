#include "constvar.h"

// data structure for attributes
union AttributeVal
{
    int true_lable;     // true lable pointer
    int false_lable;    // false lable pointer
    int begin_lable;    // begin lable pointer
    int next_lable;     // next lable pointer
    IDTABLE *exp_ptr;    // id pointer, or temporary variable(including immediate number)
};

enum AttributeType{TRUE_LABLE, FALSE_LABLE, BEGIN_LABLE, NEXT_LABLE, EXP_PTR};

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
    IDTABLE *x, *y, *z;
    union BinaryOperator op;
}BinInstr;

enum UnaryOperator{NEG, NOT};
typedef struct UnaryOperationInstruction
{
    IDTABLE *x, *y;
    union UnaryOperator op;
}UnaInstr;

typedef struct DuplicationInstruction
{
    IDTABLE *x, *y;
}DupInstr;

typedef struct GotoInstruction
{
    int lable;
}GotoInstr;

typedef struct ConditionalGotoInstruction
{
    IDTABLE *condition;
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

enum InstructionType{BIN, UNA, DUP, GOTO, CGOTO}
typedef struct Instruction
{
    InstructionType type;
    union InstructionVal;
}Instr;
