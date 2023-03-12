//#define AnaTypeLex	1
//#define AnaTypeSyn	1
#ifndef CONSTVAR_H
#define CONSTVAR_H

#define MAXTOKENLEN	256

typedef union {
	int number;
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
	{ 103, 103, 103, 103, 103,   4, 103, 103},
	{ 104, 104, 104, 104, 104,   4, 104, 104},
	{ 105, 105, 105, 105, 105,   5,   5, 105}};


//用于词法分析输出，及语法分析
#define ERR			-1
#define SYN_NUM		1		// int整数
#define SYN_ID		2		// id
#define SYN_LT		11		// <
#define SYN_GT		12		// >
#define SYN_LE		13		// <=
#define SYN_GE		14		// >=
#define SYN_EQ		15		// ==
#define SYN_NE		16		// !=
#define SYN_ADD		17		// +
#define SYN_SUB		18		// -
#define SYN_MUL		19		// *
#define SYN_DIV		20		// /
#define SYN_PAREN_L	21		// (
#define SYN_PAREN_R	22		// )
#define SYN_BRACE_L	23		// {
#define SYN_BRACE_R	24		// }
#define SYN_COMMA	25		// ,
#define SYN_SEMIC	26		// ;
#define SYN_SET		27		// =
#define SYN_AND		51		// &&
#define SYN_OR		52		// ||
#define SYN_NOT		53		// !
#define SYN_TRUE	54		// TRUE
#define SYN_FALSE	55		// FALSE
#define SYN_INT		56		// int
#define SYN_CHAR	57		// char
#define SYN_IF		58		// if
#define SYN_ELSE	59		// else
#define SYN_WHILE	60		// while
#define SYN_SHOW	61		// show

//用于符号表中类型
#define ID_FUN		1		// 函数类型
#define ID_INT		2		// int类型
#define ID_CHAR		3		// char类型


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

//extern AttributeNode *lookupAttribute(AttributeNode *attribute, enum AttributeType type);
//extern AttributeNode *installAttribute(AttributeNode *attribute, AttributeNode *head);

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