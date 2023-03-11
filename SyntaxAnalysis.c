#include "stdio.h"
#include "stdlib.h"
#include "constvar.h"
#include "codeGeneration.h"

extern TERMINAL nextToken();
extern void renewLex();
static int match (int t);
static int strcompare(char *sstr, char *tstr);	//比较两个串
static IDTABLE* InstallID();		//在符号表中为curtoken_str建立一个条目
static IDTABLE* LookupID();			//在符号表中查找curtoken_str
static void FreeExit();
static int cast2int(EXPVAL exp);		//将exp的值转换为int类型
static char cast2char(EXPVAL exp);		//将exp的值转换为char类型
static int Prod_FUNC();
static int Prod_S();
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
static int pc = 0;			// initialize program counter

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
	Prod_S();
	match(SYN_BRACE_R);
	return(0);
}

static int Prod_S()
{
	long file_index;
	EXPVAL exp;
	int bval;
	if (lookahead.token==SYN_INT || lookahead.token==SYN_CHAR)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->D S\n");
		#endif
		Prod_D();
		Prod_S();
	}
	else if (lookahead.token==SYN_ID)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->A S\n");
		#endif
		Prod_A();
		Prod_S();
	}
	else if (lookahead.token==SYN_SHOW)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->show(E); S\n");
		#endif
		match(SYN_SHOW);
		match(SYN_PAREN_L);
		exp=Prod_E();
		match(SYN_PAREN_R);
		match(SYN_SEMIC);
		if (run_status==1)
			if (exp.type==ID_INT)
				printf("%d",exp.val.intval);
			else if (exp.type==ID_CHAR)
				printf("%c",exp.val.charval);
		Prod_S();
	}
	else if (lookahead.token==SYN_IF)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->if (B) {S} [else {S}] S");
		#endif
		match(SYN_IF);
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (lookahead.token==SYN_ELSE)
		{
			match(SYN_ELSE);
			if (run_status==1) run_status=2;
			else if (run_status==2) run_status=1;
			match(SYN_BRACE_L);
			Prod_S();
			match(SYN_BRACE_R);
			if (run_status==2) run_status=1;
		}
		Prod_S();
	}
	else if (lookahead.token==SYN_WHILE)
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S-->while(B) {S} S\n");
		#endif
		match(SYN_WHILE);
		file_index=ftell(sFile)-6;
		match(SYN_PAREN_L);
		bval=Prod_B();
		match(SYN_PAREN_R);
		if (run_status==1 && bval==0) run_status=2;
		match(SYN_BRACE_L);
		Prod_S();
		match(SYN_BRACE_R);
		if (run_status==1)
		{	fseek(sFile,file_index,SEEK_SET);
			renewLex();
		}
		else if (run_status==2)
			run_status=1;
		Prod_S();
	}
	else
	{
		#if defined(AnaTypeSyn)
		printf("SYN: S--> \n");
		#endif
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
		tmp->next = NULL;
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
			tmp->next = NULL;
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
	AttributeNode E_attr = Prod_E();

	AttributeNode *tmp = (AttributeNode*)malloc(sizeof(AttributeNode));
	tmp->type = ID_PTR;
	tmp->next = NULL;
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
		tmp->next = NULL;

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
		tmp->next = NULL;

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
		attr->next = NULL;
		
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
		attr->next = NULL;
		attr->val.imm.type = ID_INT;
		attr->val.imm.val.intval = 1;
	}
	else if (lookahead.token == SYN_FALSE)
	{
		// FB-->FALSE
		match(SYN_FALSE);
		attr->type = IMM_VAL;
		attr->next = NULL;
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
			attr->next = NULL;
			
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
			attr->next = NULL;
			
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
			attr->next = NULL;
			
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
			attr->next = NULL;
			
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
			attr->next = NULL;
			
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
			attr->next = NULL;
			
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
	AttributeNode *E1_attr = Prod_E1(TE_attr)
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
		tmp->next = NULL;

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
		tmp->next = NULL;

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
		tmp->next = NULL;

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
		tmp->next = NULL;

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
		attr->next = NULL;
	}
	else if (lookahead.token == SYN_ID)
	{
		// F-->id
		match(SYN_ID);
		IDTABLE *p = LookupID();
		attr = (AttributeNode*)malloc(sizeof(AttributeNode));
		attr->type = ID_PTR;
		attr->val.id_ptr = p;
		attr->next = NULL;
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
