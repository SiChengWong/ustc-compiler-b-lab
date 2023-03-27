//#define AnaTypeLex	1
//#define AnaTypeSyn	1
#ifndef CONSTVAR_H
#define CONSTVAR_H

#define MAXTOKENLEN	256

typedef union {
	int number;
    char character;
	char *str;
} TOKENVAL;

typedef struct {
	int token;
	TOKENVAL tokenVal;
} TERMINAL;

typedef union {
	int intval;
	char charval;
} EXPVALUE;

typedef struct expValue{
	int type;
	EXPVALUE val;
} EXPVAL;

typedef struct idNode{
	char name[MAXTOKENLEN];
	int type;
	EXPVALUE val;
	struct idNode *next;
} IDTABLE;

//词法分析DFA转换表
static int LexTable[6][8]=
   {{   1, 201, 204,   2,   3,   4,   5, 205},
	{ 101, 101, 101, 101, 101, 101, 101, 101},
	{ 102, 102, 202, 203, 102, 102, 102, 102},
	{ 103, 103, 103, 103, 103, 103, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};


//用于词法分析输出，及语法分析
enum SynType{
    ERR = -1, 
    SYN_IMM_INT, SYN_IMM_CHAR, SYN_ID, 
    SYN_LT, SYN_GT, SYN_LE, SYN_GE, SYN_EQ, SYN_NE, 
    SYN_ADD, SYN_SUB, SYN_MUL, SYN_DIV,
    SYN_PAREN_L, SYN_PAREN_R, SYN_BRACE_L, SYN_BRACE_R,
    SYN_COMMA, SYN_SEMIC, 
    SYN_SET, 
    SYN_AND, SYN_OR, SYN_NOT, SYN_TRUE, SYN_FALSE, 
    SYN_INT, SYN_CHAR, 
    SYN_IF, SYN_ELSE, SYN_WHILE, SYN_SHOW
    };

//用于符号表中类型
enum IdType{ID_FUN = 1, ID_INT, ID_CHAR};

// data structure for attributes
union AttributeVal
{
    IDTABLE *id_ptr;    // id pointer
    EXPVAL *tmp_ptr;    // temporary variable pointer
    EXPVAL imm;         // immediate number value
};

enum AttributeType{ID_PTR, TMP_PTR, IMM_VAL};

typedef struct AttributeNode
{
    enum AttributeType type;
    union AttributeVal val;
}AttributeNode;

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

#endif