#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static int PrintCode();				// print IR code
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S(int *S_next);
static int Prod_D();
static int Prod_L(int type);
static int Prod_T();
static int Prod_A();
static AttributeNode* Prod_B();
static AttributeNode* Prod_B1(AttributeNode* pre_attr);
static AttributeNode* Prod_TB();
static AttributeNode* Prod_TB1(AttributeNode* pre_attr);
static AttributeNode* Prod_FB();
static AttributeNode* Prod_E();
static AttributeNode* Prod_E1(AttributeNode *pre_attr);
static AttributeNode* Prod_TE();
static AttributeNode* Prod_TE1(AttributeNode *pre_attr);
static AttributeNode* Prod_F();

extern FILE *sFile;
static TERMINAL lookahead;
static int curtoken_num;
static char curtoken_str[MAXTOKENLEN];
static IDTABLE *IDTHead=NULL;
static int run_status=1;	//0；程序不执行；1:程序正常执行；2:跳过当前结构后继续执行

#define MAX_CODE 1024
Instr code[MAX_CODE];       // code stack
int pc = 0;                     // program counter

static char *binOpSet[] = {"+", "-", "*", "/", "&&", "||", ">", ">=", "<", "<=", "==", "!="};
static char *unaOpSet[] = {"-", "!"};

// print program code
int PrintCode(){
	for (int i = 0; i < pc; i++)
	{
		printf("%d\t", i);
		switch (code[i].type)
		{
		case BIN:
			// print x
			switch (code[i].val.binInstr.x->type)
			{
			case ID_PTR:
				printf("%s\t=\t", code[i].val.binInstr.x->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\t=\t", code[i].val.binInstr.x->val.tmp_ptr);
				break;
			}
			// print y
			switch (code[i].val.binInstr.y->type)
			{
			case ID_PTR:
				printf("%s\t", code[i].val.binInstr.y->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\t", code[i].val.binInstr.y->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("%d\t", code[i].val.binInstr.y->val.imm.val.intval);
				break;
			}
			// print op
			printf("%s\t", binOpSet[code[i].val.binInstr.op]);
			// print z
			switch (code[i].val.binInstr.z->type)
			{
			case ID_PTR:
				printf("%s\n", code[i].val.binInstr.z->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\n", code[i].val.binInstr.z->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("%d\n", code[i].val.binInstr.z->val.imm.val.intval);
				break;
			}
			break;
		case UNA:
			// print x
			switch (code[i].val.unaInstr.x->type)
			{
			case ID_PTR:
				printf("%s\t=\t", code[i].val.unaInstr.x->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\t=\t", code[i].val.unaInstr.x->val.tmp_ptr);
				break;
			}
			// print op
			printf("%s\t", unaOpSet[code[i].val.unaInstr.op]);
			// print y
			switch (code[i].val.unaInstr.y->type)
			{
			case ID_PTR:
				printf("%s\n", code[i].val.unaInstr.y->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\n", code[i].val.unaInstr.y->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("%d\n", code[i].val.unaInstr.y->val.imm.val.intval);
				break;
			}
			break;
		case DUP:
			// print x
			switch (code[i].val.dupInstr.x->type)
			{
			case ID_PTR:
				printf("%s\t=\t", code[i].val.dupInstr.x->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\t=\t", code[i].val.dupInstr.x->val.tmp_ptr);
				break;
			}
			// print y
			switch (code[i].val.dupInstr.y->type)
			{
			case ID_PTR:
				printf("%s\n", code[i].val.dupInstr.y->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("&%x\n", code[i].val.dupInstr.y->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("%d\n", code[i].val.dupInstr.y->val.imm.val.intval);
				break;
			}
			break;
		case GOTO:
			printf("goto\tL%d\n", *code[i].val.gotoInstr.label);
			break;
		case CGOTO:
			// print condition
			switch (code[i].val.conGotoInstr.condition->type)
			{
			case ID_PTR:
				printf("if\t%s\t", code[i].val.conGotoInstr.condition->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("if\t&%x\t", code[i].val.conGotoInstr.condition->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("if\t%d\t", code[i].val.conGotoInstr.condition->val.imm.val.intval);
				break;
			}
			// print goto dest
			printf("goto\tL%d\n", *code[i].val.conGotoInstr.label);
			break;
		case PRINT:
			switch (code[i].val.printInstr.expression->type)
			{
			case ID_PTR:
				printf("SHOW\t%s\n", code[i].val.printInstr.expression->val.id_ptr->name);
				break;
			case TMP_PTR:
				printf("SHOW\t&%x\n", code[i].val.printInstr.expression->val.tmp_ptr);
				break;
			case IMM_VAL:
				printf("SHOW\t%d\n", code[i].val.printInstr.expression->val.imm.val.intval);
				break;
			}
			break;
		}
	}
	return 0;
}

// execute program code
int Execute(){
	return 0;
}

void SyntaxAnalysis()
{
#if defined(AnaTypeLex)
//testing lexical analysis
	TERMINAL token;
	token=nextToken();
	while (token.token!=ERR)
	{	if (token.token==SYN_NUM)
			printf("LEX: %d,%d\n",token.token,token.tokenVal.number);
		else if (token.token==SYN_ID)
			printf("LEX: %d,%s\n",token.token,token.tokenVal.str);
		else
			printf("LEX: %d\n",token.token);
		token=nextToken();
	}
#else
//syntax analysis
	lookahead=nextToken();
	if (Prod_FUNC()==ERR)
		printf("PROGRAM HALT!\n");
	PrintCode();
	FreeExit();

#endif
}

static int match (int t)
{
	char *p,*q;
	if (lookahead.token == t)
	{	if (t==SYN_NUM)
			curtoken_num=lookahead.tokenVal.number;
		else if (t==SYN_ID)
			for (p=lookahead.tokenVal.str,q=curtoken_str;(*q=*p)!='\0';p++,q++);
		lookahead = nextToken( );
	}
	else
		FreeExit();
	return(0);
}

static int strcompare(char *sstr, char *tstr)
{
	while (*sstr==*tstr && *sstr!='\0') { sstr++; tstr++; }
	if (*sstr=='\0' && *tstr=='\0')	return(0);
	else return(ERR);
}

static IDTABLE* InstallID()
{
	IDTABLE *p,*q;
	char *a,*b;
	p=IDTHead; q=NULL;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
	{
		q=p;
		p=p->next;
	}
	if (p!=NULL)
		return(NULL);
	else
	{
		p=(IDTABLE*)malloc(sizeof(IDTABLE));
		if (q==NULL)
			IDTHead=p;
		else
			q->next=p;
		p->next=NULL;
		for (a=curtoken_str,b=p->name;(*b=*a)!='\0';a++,b++);
		return(p);
	}
}

static IDTABLE* LookupID()
{
	IDTABLE *p;
	p=IDTHead;
	while (p!=NULL && strcompare(curtoken_str,p->name)==ERR)
		p=p->next;
	if (p==NULL)
		return(NULL);
	else
		return(p);
}

static void FreeExit()
{
	IDTABLE *p,*q;
	//释放链表空间
	p=IDTHead;
	while ((q=p)!=NULL)
	{	p=p->next;
		#if defined(AnaTypeSyn)
		printf("NAME:%s, TYPE:%d, ",q->name,q->type);
		if (q->type==ID_INT)
			printf("VALUE:%d\n",q->val.intval);
		else if (q->type==ID_CHAR)
			printf("VALUE:%c\n",q->val.charval);
		else
			printf("\n");
		#endif
		free(q);
	}
	exit(0);
}

static int cast2int(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return(exp.val.intval);
	else if (exp.type==ID_CHAR)
		return((int)(exp.val.charval));
}

static char cast2char(EXPVAL exp)
{
	if (exp.type==ID_INT)
		return((char)(exp.val.intval));
	else if (exp.type==ID_CHAR)
		return(exp.val.charval);
}

static int Prod_FUNC()
{
	IDTABLE *p;
	match(SYN_ID);
	if (strcompare(curtoken_str,"main")==ERR) FreeExit();
	p=InstallID();
	p->type=ID_FUN;
	#if defined(AnaTypeSyn)
	printf("SYN: FUNC-->main() {S}\n");
	#endif
	match(SYN_PAREN_L);
	match(SYN_PAREN_R);
	match(SYN_BRACE_L);

	int *S_next = (int*)malloc(sizeof(int));
	Prod_S(S_next);
	*S_next = pc;

	match(SYN_BRACE_R);
	return(0);
}

static int Prod_S(int *S_next)
{
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{
		// S-->D S
		Prod_D();
		Prod_S(S_next);
	}
	else if (lookahead.token==SYN_ID)
	{
		// S-->A S
		Prod_A();
		Prod_S(S_next);
	}
	else if (lookahead.token==SYN_SHOW)
	{
		// S-->show(E); S
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		AttributeNode *E_attr=Prod_E();
		match(SYN_PAREN_R);
		match(SYN_SEMIC);

		code[pc].type = PRINT;
		code[pc].val.printInstr.expression = E_attr;
		pc++;

		Prod_S(S_next);
	}
	else if (lookahead.token==SYN_IF)
	{
		// S-->if (B) {S} [else {S}] S
		match(SYN_IF);
		match(SYN_PAREN_L);
		AttributeNode *B_attr = Prod_B();
		match(SYN_PAREN_R);

		// emit conditional goto instruction
		int *B_true = (int*)malloc(sizeof(int));
		int *B_false = (int*)malloc(sizeof(int));
		// generate true branch
		code[pc].type = CGOTO;
		code[pc].val.conGotoInstr.condition = B_attr;
		code[pc].val.conGotoInstr.label = B_true;	// B.true label
		pc++;
		// generate false branch
		code[pc].type = GOTO;
		code[pc].val.gotoInstr.label = B_false;
		pc++;
		// backpatch B.true
		*B_true = pc;
		
		match(SYN_BRACE_L);
		int *S_1_next = (int*)malloc(sizeof(int));
		Prod_S(S_1_next);
		match(SYN_BRACE_R);

		// emit unconditional goto instruction
		code[pc].type = GOTO;
		code[pc].val.gotoInstr.label = S_1_next;
		pc++;

		if (lookahead.token == SYN_ELSE)
		{
			// S-->if (B) {S} else {S} S
			// backpatch B.false
			*B_false = pc;

			match(SYN_ELSE);
			match(SYN_BRACE_L);
			int *S_2_next = S_1_next;
			Prod_S(S_2_next);
			match(SYN_BRACE_R);

			// backpatch S1.next, S2.next
			*S_1_next = *S_2_next = pc;
		}
		else
		{
			// S-->if (B) {S} S
			// backpatch B.false
			*B_false = pc;
		}
		Prod_S(S_next);
	}
	else if (lookahead.token==SYN_WHILE)
	{
		// S-->while(B) {S} S
		// generate label S.begin
		int *S_begin = (int*)malloc(sizeof(int));
		*S_begin = pc;

		match(SYN_WHILE);
		match(SYN_PAREN_L);
		AttributeNode *B_attr = Prod_B();
		match(SYN_PAREN_R);

		// emit conditional goto instruction
		int *B_true = (int*)malloc(sizeof(int));
		int *B_false = (int*)malloc(sizeof(int));
		// generate true branch
		code[pc].type = CGOTO;
		code[pc].val.conGotoInstr.condition = B_attr;
		code[pc].val.conGotoInstr.label = B_true;
		pc++;
		// generate false branch
		code[pc].type = GOTO;
		code[pc].val.gotoInstr.label = B_false;
		pc++;
		// backpatch B.true
		*B_true = pc;

		match(SYN_BRACE_L);
		Prod_S(S_begin);
		match(SYN_BRACE_R);

		code[pc].type = GOTO;
		code[pc].val.gotoInstr.label = S_begin;
		pc++;

		*B_false = pc;
		Prod_S(S_next);
	}
	else
	{
		// S--> 
	}
	return(0);
}

static int Prod_D()
{
	int type;
	// D-->T id [=E] L;
	type=Prod_T();
	match(SYN_ID);
	IDTABLE *p=InstallID();
	p->type=type;
	if (lookahead.token==SYN_SET)
	{
		match(SYN_SET);
		AttributeNode *E_attr = Prod_E();

		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = ID_PTR;
		tmp->val.id_ptr = p;

		code[pc].type = DUP;
		code[pc].val.dupInstr.x = tmp;
		code[pc].val.dupInstr.y = E_attr;
		pc++;
	}
	Prod_L(type);
	match(SYN_SEMIC);
	return(0);
}

static int Prod_L(int type)
{
	if (lookahead.token==SYN_COMMA)
	{
		// L-->, id [=E] L
		match(SYN_COMMA);
		match(SYN_ID);
		IDTABLE *p=InstallID();
		p->type=type;
		if (lookahead.token==SYN_SET)
		{
			match(SYN_SET);
			AttributeNode *E_attr = Prod_E();

			AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
			tmp->type = ID_PTR;
			tmp->val.id_ptr = p;

			code[pc].type = DUP;
			code[pc].val.dupInstr.x = tmp;
			code[pc].val.dupInstr.y = E_attr;
			pc++;
		}
		Prod_L(type);
	}
	// L-->
	return(0);
}

static int Prod_T()
{
	if (lookahead.token==SYN_INT)
	{
		// T-->int
		match(SYN_INT);
		return ID_INT;
	}
	else if (lookahead.token==SYN_CHAR)
	{
		// T-->char
		match(SYN_CHAR);
		return ID_CHAR;
	}
	else
		FreeExit();
	return(0);
}

static int Prod_A()
{
	// A-->id=E;
	match(SYN_ID);
	IDTABLE *p = LookupID();
	match(SYN_SET);
	AttributeNode *E_attr = Prod_E();
	match(SYN_SEMIC);

	AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
	tmp->type = ID_PTR;
	tmp->val.id_ptr = p;
	
	code[pc].type = DUP;
	code[pc].val.dupInstr.x = tmp;
	code[pc].val.dupInstr.y = E_attr;
	pc++;

	return 0;
}

static AttributeNode* Prod_B()
{
	// B-->TB B1
	AttributeNode *TB_attr = Prod_TB();
	AttributeNode *B1_attr = Prod_B1(TB_attr);
	return B1_attr;
}

static AttributeNode* Prod_B1(AttributeNode *pre_attr)
{
	AttributeNode *attr;
	if (lookahead.token == SYN_OR)
	{
		// B1-->|| TB B1
		match(SYN_OR);
		AttributeNode *TB_attr = Prod_TB();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = TB_attr;
		code[pc].val.binInstr.op = OR;
		pc++;

		attr = Prod_B1(tmp);
	}
	else
	{
		// B1-->
		attr = pre_attr;
	}
	return attr;
}

static AttributeNode* Prod_TB()
{
	// TB-->FB TB1
	AttributeNode *FB_attr = Prod_FB();
	AttributeNode *TB1_attr = Prod_TB1(FB_attr);
	return TB1_attr;
}

static AttributeNode* Prod_TB1(AttributeNode *pre_attr)
{
	AttributeNode *attr;
	if (lookahead.token == SYN_AND)
	{
		// TB1-->&& FB TB1
		match(SYN_ADD);
		AttributeNode *FB_attr = Prod_FB();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = FB_attr;
		code[pc].val.binInstr.op = AND;
		pc++;

		attr = Prod_TB1(tmp);
	}
	else
	{
		// TB1-->
		attr = pre_attr;
	}
	return attr;
}

static AttributeNode* Prod_FB()
{
	AttributeNode *attr;
	if (lookahead.token == SYN_NOT)
	{
		// FB-->!B
		match(SYN_NOT);
		AttributeNode *B_attr = Prod_B();
		attr = (AttributeNode*)malloc(sizeof(AttributeNode));
		attr->type = TMP_PTR;
		
		code[pc].type = UNA;
		code[pc].val.unaInstr.x = attr;
		code[pc].val.unaInstr.y = B_attr;
		code[pc].val.unaInstr.op = NEG;
		pc++;
	}
	else if (lookahead.token == SYN_TRUE)
	{
		// FB-->TRUE
		match(SYN_TRUE);
		attr = (AttributeNode*)malloc(sizeof(AttributeNode));
		attr->type = IMM_VAL;
		attr->val.imm.type = ID_INT;
		attr->val.imm.val.intval = 1;
	}
	else if (lookahead.token == SYN_FALSE)
	{
		// FB-->FALSE
		match(SYN_FALSE);
		attr->type = IMM_VAL;
		attr->val.imm.type = ID_INT;
		attr->val.imm.val.intval = 0;
	}
	else if (lookahead.token == SYN_ID || lookahead.token == SYN_NUM || lookahead.token == SYN_PAREN_L)
	{
		AttributeNode *E_attr = Prod_E();
		if (lookahead.token == SYN_LT)
		{
			// FB-->E<E
			match(SYN_LT);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = LT;
			pc++;
		}
		else if (lookahead.token == SYN_LE)
		{
			// FB-->E<=E
			match(SYN_LE);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = LE;
			pc++;
		}
		else if (lookahead.token == SYN_GT)
		{
			// FB-->E>E
			match(SYN_GT);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = GT;
			pc++;
		}
		else if (lookahead.token == SYN_GE)
		{
			// FB-->E>=E
			match(SYN_GE);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = GE;
			pc++;
		}
		else if (lookahead.token == SYN_EQ)
		{
			// FB-->E==E
			match(SYN_EQ);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = EQ;
			pc++;
		}
		else if (lookahead.token == SYN_NE)
		{
			// FB-->E!=E
			match(SYN_NE);
			AttributeNode *E_1_attr = E_attr;
			AttributeNode *E_2_attr = Prod_E();
			attr = (AttributeNode*)malloc(sizeof(AttributeNode));
			attr->type = TMP_PTR;
			
			code[pc].type = BIN;
			code[pc].val.binInstr.x = attr;
			code[pc].val.binInstr.y = E_1_attr;
			code[pc].val.binInstr.z = E_2_attr;
			code[pc].val.binInstr.op = NE;
			pc++;
		}
		else
		{
			// FB-->E
			attr = E_attr;
		}
		return attr;
	}
	else
	{
		FreeExit();
		return 0;
	}
	
}

static AttributeNode* Prod_E()
{
	// E-->TE E1
	AttributeNode *TE_attr = Prod_TE();
	AttributeNode *E1_attr = Prod_E1(TE_attr);
	return E1_attr;
}

static AttributeNode* Prod_E1(AttributeNode *pre_attr)
{
	AttributeNode *attr;
	if (lookahead.token == SYN_ADD)
	{
		// E1-->+TE E1
		match(SYN_ADD);
		AttributeNode *TE_attr = Prod_TE();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = TE_attr;
		code[pc].val.binInstr.op = ADD;
		pc++;

		attr = Prod_E1(tmp);
	}
	else if (lookahead.token == SYN_SUB)
	{
		// E1-->-TE E1
		match(SYN_SUB);
		AttributeNode *TE_attr = Prod_TE();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = TE_attr;
		code[pc].val.binInstr.op = SUB;
		pc++;

		attr = Prod_E1(tmp);
	}
	else
	{
		// E1-->
		attr = pre_attr;
	}
	return attr;
}

static AttributeNode* Prod_TE()
{
	// TE-->F TE1
	AttributeNode *F_attr = Prod_F();
	AttributeNode *TE1_attr = Prod_TE1(F_attr);
	return TE1_attr;
}

static AttributeNode* Prod_TE1(AttributeNode *pre_attr)
{
	AttributeNode *attr;
	if (lookahead.token == SYN_MUL)
	{
		// TE1-->*F TE1
		match(SYN_MUL);
		AttributeNode *F_attr = Prod_F();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = F_attr;
		code[pc].val.binInstr.op = MUL;
		pc++;

		attr = Prod_TE1(tmp);
	}
	else if (lookahead.token == SYN_DIV)
	{
		// TE1-->/F TE1
		match(SYN_DIV);
		AttributeNode *F_attr = Prod_F();
		AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
		tmp->type = TMP_PTR;

		code[pc].type = BIN;
		code[pc].val.binInstr.x = tmp;
		code[pc].val.binInstr.y = pre_attr;
		code[pc].val.binInstr.z = F_attr;
		code[pc].val.binInstr.op = DIV;
		pc++;

		attr = Prod_TE1(tmp);
	}
	else
	{
		// TE1-->
		attr = pre_attr;
	}
	return attr;
}

static AttributeNode* Prod_F()
{
	AttributeNode *attr;
	if (lookahead.token == SYN_NUM)
	{
		// F-->num
		match(SYN_NUM);
		attr = (AttributeNode*)malloc(sizeof(AttributeNode));
		attr->type = IMM_VAL;
		attr->val.imm.type = ID_INT;
		attr->val.imm.val.intval = curtoken_num;
		
	}
	else if (lookahead.token == SYN_ID)
	{
		// F-->id
		match(SYN_ID);
		IDTABLE *p = LookupID();
		attr = (AttributeNode*)malloc(sizeof(AttributeNode));
		attr->type = ID_PTR;
		attr->val.id_ptr = p;
		
	}
	else if (lookahead.token == SYN_PAREN_L)
	{
		// F-->(E)
		match(SYN_PAREN_L);
		attr = Prod_E();
		match(SYN_PAREN_R);
	}
	else
		FreeExit();
	return attr;
}
