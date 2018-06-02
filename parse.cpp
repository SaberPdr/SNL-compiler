#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"

FILE *fp_parse;
TokenNode *curr_list;
TokenType *curr_token;
TreeNode *tree_root;

char *tmp_name;

int tabnum = 0;
ParseStates parse_state = Normal;

char *NodeChar[NodeCount] = {
	"ProK", "PheadK", "DecK", "TypeK", "VarK",
	"ProcDecK", "StmLK", "StmtK", "ExpK", "ProcK"};

char *DecChar[DecCount] = {
	"ArrayK", "CharK", "IntegerK", "RecordK", "IdK"};

char *StmtChar[StmtCount] = {
	"If", "While", "Assign", "Read",
	"Write", "Call", "Return"};

char *ExpChar[ExpCount] = {"Op", "Const", "Vari", "ConstChar"};

char *VarChar[VarCount] = {"IdV", "ArrayMembV", "FieldMembV"};

char *ExpTypeChar[ExpTypeCount] = {"Void", "Integer", "Boolean"};

char *ParamChar[ParamCount] = {"val_param", "var_param"};

TreeNode *parse(TokenNode *token_list, char *parse_file)
{
	if ((fp_parse =  fopen(parse_file, "w")) == NULL) {
		fprintf(stderr, "打开或建立文件 %s 失败\n", parse_file);
		exit(-1);
	}

	if (token_list->nextToken != NULL) {
		curr_list = token_list->nextToken;
		curr_token = curr_list->token;
	} else {
		fprintf(stderr, "token list 为空\n");
		exit(-1);
	}

	TreeNode *tree_root = _Program();
	print_tree(tree_root);

	fclose(fp_parse);
	return tree_root;

}

TreeNode *new_TreeNode(NodeKind nodekind)
{
	TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
	memset(node, 0x00, sizeof(TreeNode));
	if (node == NULL) {
		fprintf(stderr, "内存不足，无法建立语法树!\n");
		exit(-1);
	}
	node->nodekind = nodekind;
	node->lineno = curr_token->line_show;

	return node;
}

void print_tree(TreeNode *tree_node)
{
	int i;
	if (tree_node != NULL) {
		fprintf(fp_parse, "line: %-3d ", tree_node->lineno);
		for (i = 0; i < tabnum; i++) {
			fprintf(fp_parse, "    ");
		}
		fprintf(fp_parse, "%s ", NodeChar[tree_node->nodekind]);
		tabnum++;
		switch (tree_node->nodekind)
		{
			case ProK:
			case TypeK:
			case VarK:
			case ProcDecK:
			case StmLK:
				parse_state = Normal;
				break;
			case PheadK:
				parse_state = Normal;
				fprintf(fp_parse, "%s ", tree_node->name[0]);
				break;
			case ProcK:
				parse_state = INProcDec;
				fprintf(fp_parse, "%s ", tree_node->name[0]);
				break;
			case DecK:
				if (parse_state == INProcDec) {
					fprintf(fp_parse, "%s ", ParamChar[tree_node->attr.ProcAttr.paramt]);
				} else if (parse_state == INRecordDec) {
					fprintf(fp_parse,"InRecord ");
				}
				switch (tree_node->kind.dec)
				{
					case ArrayK:
						fprintf(fp_parse, "%s ", DecChar[tree_node->kind.dec]);
						for (i = 0; i < tree_node->idnum; i++) {
							fprintf(fp_parse, "%s ", tree_node->name[i]);
						}
						fprintf(fp_parse, "low: %d up: %d childtype: %s ",\
								tree_node->attr.ArrayAttr.low, \
								tree_node->attr.ArrayAttr.up, \
								DecChar[tree_node->attr.ArrayAttr.childtype]);
						break;
					case CharK:
					case IntegerK:
						fprintf(fp_parse, "%s ", DecChar[tree_node->kind.dec]);
						for (i = 0; i < tree_node->idnum; i++) {
							fprintf(fp_parse, "%s ", tree_node->name[i]);
						}
						break;
					case RecordK:
						parse_state = INRecordDec;
						fprintf(fp_parse, "%s ", DecChar[tree_node->kind.dec]);
						for (i = 0; i < tree_node->idnum; i++) {
							fprintf(fp_parse, "%s ", tree_node->name[i]);
						}
						break;
					case IdK:
						fprintf(fp_parse, "%s ", tree_node->type_name);
						for (i = 0; i < tree_node->idnum; i++) {
							fprintf(fp_parse, "%s ", tree_node->name[i]);
						}
						break;
					default:
						fprintf(fp_parse, "Error DecKind! ");
				}
				break;
			case StmtK:
				fprintf(fp_parse, "%s ", StmtChar[tree_node->kind.stmt]);
				switch (tree_node->kind.stmt)
				{
					case IfK:
					case WhileK:
					case AssignK:
					case WriteK:
					case CallK:
					case ReturnK:
						break;
					case ReadK:
						fprintf(fp_parse, "%s ", tree_node->name[0]);
						break;
					default:
						fprintf(fp_parse, "Error StmtKind! ");
				}
				break;
			case ExpK:
				switch (tree_node->kind.exp)
				{
					case OpK:
						fprintf(fp_parse, "%s ", ExpChar[tree_node->kind.exp]);
						fprintf(fp_parse, "%s ", ReservedWord[tree_node->attr.ExpAttr.op]);
						break;
					case ConstK:
						fprintf(fp_parse, "%s ", ExpChar[tree_node->kind.exp]);
						fprintf(fp_parse, "%d ", tree_node->attr.ExpAttr.val);
						break;
					case ConstCharK:
						fprintf(fp_parse, "%s ", ExpChar[tree_node->kind.exp]);
						fprintf(fp_parse, "%c ", tree_node->attr.ExpAttr.chval);
						break;
					case VariK:
						fprintf(fp_parse, "%s ", tree_node->name[0]);
						fprintf(fp_parse, "%s ", VarChar[tree_node->attr.ExpAttr.varkind]);
						break;
					default:
						fprintf(fp_parse, "Error ExpKind! ");
				}
				break;
			default :
				fprintf(fp_parse, "Error NodeKind ");
		}
		fprintf(fp_parse, "\n");

		for (i = 0; i < MAXCHILDREN; i++) {
			if (tree_node->child[i] != NULL) {
				print_tree(tree_node->child[i]);
			}
		}
		tabnum--;
		if (tree_node->sibling != NULL) {
			print_tree(tree_node->sibling);
		}
	}
}

void free_tree(TreeNode *tree_node)
{
	if (tree_node != NULL) {
		int i;
		for (i = 0; i < MAXCHILDREN; i++) {
			if (tree_node->child[i] != NULL) {
				free_tree(tree_node->child[i]);
			}
		}
		if (tree_node->sibling != NULL) {
			free_tree(tree_node->sibling);
		}
	}
}

void get_next_token()
{
	if (curr_list->nextToken != NULL) {
		curr_list = curr_list->nextToken;
		curr_token = curr_list->token;
	} else {
		fprintf(stderr, "token list已经结束!\n");
	}
}

void ParseError(TokenType *curr_token, LexType lex_type)
{
	fprintf(stderr, "第%d行: 错误: \"%s\", 在 \"%s\"之前\n\n", curr_token->line_show, ReservedWord[lex_type], curr_token->sem_info);
}

void UnexceptError()
{
	fprintf(stderr, "第%d行: 错误的token: \"%s\" \n", curr_token->line_show, curr_token->sem_info);
}

int match(LexType lex_type)
{
	int flag = 0;
	if (lex_type == curr_token->lex_type) {
		get_next_token();
		flag = 1;
	} else {
		flag = 0;
		UnexceptError();
		fprintf(stderr, "\t");
		ParseError(curr_token, lex_type);
	}
	return flag;
}

//1 _Program ::= _ProgramHead _DeclarePart _ProgramBody .
TreeNode *_Program()
{
	TreeNode *root = new_TreeNode(ProK);

	TreeNode *head = _ProgramHead();
	TreeNode *decl = _DeclarePart();
	TreeNode *body = _ProgramBody();

	int i = 0;
	if (head != NULL) {
		root->child[i] = head;
		i++;
	} else {
		fprintf(stderr, "Except \"_Program Head\" here!\n\n");
	}
	if (decl != NULL) {
		root->child[i] = decl;
		i++;
	}
	if (body != NULL) {
		root->child[i] = body;
	} else {
		fprintf(stderr, "Except \"_Program Body\" here!\n\n");
	}

	if (match(DOT)) {
		fprintf(stdout, "\n到达程序尾\n\n");
	} else {
		fprintf(stderr, "\n程序错误结束\n\n");
	}

	return root;
}

//2 _ProgramHead ::= PROGRAM _ProgramName
TreeNode *_ProgramHead()
{
	//程序头，root->child[0]
	TreeNode *head = NULL;
	match(PROGRAM);
	head = new_TreeNode(PheadK);
	_ProgramName(head);
	return head;
}

//3 _ProgramName ::= ID
void _ProgramName(TreeNode *tree_node)
{
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	return ;
}

//4 _DeclarePart ::= _TypeDecpart _VarDecpart _ProcDecpart
TreeNode *_DeclarePart()
{
	//声明部分，root->child[1]
	TreeNode *decl = NULL;

	//类型声明TypeK，相当于typedef
	TreeNode *type = new_TreeNode(TypeK);
	TreeNode *tmp_type = _TypeDecpart();
	type->child[0] = tmp_type;

	//变量声明VarK，上一步声明的类型(原本类型的别名)可以用来声明变量
	TreeNode *vard = new_TreeNode(VarK);
	TreeNode *tmp_vard = _VarDecpart();
	vard->child[0] = tmp_vard;

	//过程声明ProcK
	TreeNode *proc = new_TreeNode(ProcDecK);
	TreeNode *tmp_proc = _ProcDecpart();
	proc->child[0] = tmp_proc;

	//这三个是兄弟结点
	type->sibling = (tmp_vard != NULL) ? vard : proc;
	vard->sibling = (tmp_proc != NULL) ? proc : NULL;

	if (tmp_type != NULL) {
		decl = type;
	} else if (tmp_vard != NULL) {
		free(type);
		decl = vard;
	} else if (tmp_proc != NULL) {
		free(type);
		free(vard);
		decl = proc;
	} else {
		free(type);
		free(vard);
		free(proc);
	}

	return decl;
}

//5 _TypeDecpart ::= ε | _TypeDec
TreeNode *_TypeDecpart()
{
	//类型声明，root->child[1]三兄弟的第一个
	TreeNode *type = NULL;

	switch (curr_token->lex_type)
	{
		case VAR:
		case PROCEDURE:
		case BEGIN:
			break;
		case TYPE:
			//有TYPE关键字
			type = _TypeDec();
			break;
		default :
			UnexceptError();
			get_next_token();
			break;
	}

	return type;
}

//7 _TypeDec ::= TYPE _TypeDecList
TreeNode *_TypeDec()
{
	match(TYPE);
	//类型声明的儿子链表
	TreeNode *type_dec = _TypeDecList();

	return type_dec;
}

//8 _TypeDecList ::= _TypeId=_TypeDef;_TypeDecMore
TreeNode *_TypeDecList()
{
	//类型声明部分的分析，返回这个部分声明的链表头
	TreeNode *type_list = new_TreeNode(DecK);
	_TypeId(type_list);		//匹配等号之前的ID
	match(EQ);				//匹配等号
	_TypeDef(type_list);	//匹配等号之后的数据类型
	match(SEMI);			//匹配；
	TreeNode *type_more = _TypeDecMore();//不止一条声明则调用more
	type_list->sibling = type_more;

	return type_list;
}

//9 _TypeDecMore ::= ε | _TypeDecList
TreeNode *_TypeDecMore()
{
	//第二条类型声明及之后的分析
	TreeNode *type_more = NULL;
	switch (curr_token->lex_type)
	{
		case VAR:
		case PROCEDURE:
		case BEGIN:
			break;
		case ID:
			type_more = _TypeDecList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return type_more;
}

//11 _TypeId ::= ID
void _TypeId(TreeNode *tree_node)
{
	//类型声明中，“=”之前的是ID
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	return ;
}

//12 _TypeDef ::= _BaseType | _StructureType | ID
void _TypeDef(TreeNode *tree_node)
{
	//匹配(等号之后的)数据类型
	switch (curr_token->lex_type)
	{
		case INTEGER:
		case CHAR:
			//基本类型
			_BaseType(tree_node);
			break;
		case ARRAY:
		case RECORD:
			//数组和record，结构类型
			_StructureType(tree_node);
			break;
		case ID:
			tree_node->kind.dec = IdK;
			strcpy(tree_node->type_name, curr_token->sem_info);
			match(ID);
			break;
		default :
			UnexceptError();
			get_next_token();
			break;
	}
	return ;
}

//15 _BaseType ::= INTEGER | CHAR
void _BaseType(TreeNode *tree_node)
{
	//int和char，基本类型
	switch (curr_token->lex_type)
	{
		case INTEGER:
			tree_node->kind.dec = IntegerK;
			match(INTEGER);
			break;
		case CHAR:
			tree_node->kind.dec = CharK;
			match(CHAR);
			break;
		default :
			UnexceptError();
			get_next_token();
			break;
	}
	return ;
}

//17 _StructureType ::= _ArrayType | _RecType
void _StructureType(TreeNode *tree_node)
{
	//数组和记录类型也就是结构体，结构类型
	switch (curr_token->lex_type)
	{
		case ARRAY:
			_ArrayType(tree_node);
			break;
		case RECORD:
			_RecType(tree_node);
			break;
		default :
			UnexceptError();
			get_next_token();
			break;
	}
	return ;
}

//19 _ArrayType ::= ARRAY [_low.._top] OF _BaseType
void _ArrayType(TreeNode *tree_node)
{
	//匹配数组类型，记录数组上下限和子类型
	match(ARRAY);
	match(LMIDPAREN);
	_low(tree_node);
	match(UNDERANGE);
	_top(tree_node);
	match(RMIDPAREN);
	match(OF);
	_BaseType(tree_node);
	tree_node->attr.ArrayAttr.childtype = tree_node->kind.dec;
	tree_node->kind.dec = ArrayK;
	return ;
}

//20 _low ::= INTC
void _low(TreeNode *tree_node)
{
	if (curr_token->lex_type == INTC) {
		tree_node->attr.ArrayAttr.low = atoi(curr_token->sem_info);
	}
	match(INTC);
	return ;
}

//21 _top ::= INTC
void _top(TreeNode *tree_node)
{
	if (curr_token->lex_type == INTC) {
		tree_node->attr.ArrayAttr.up = atoi(curr_token->sem_info);
	}
	match(INTC);
	return ;
}

//22 _RecType ::= RECORD _FieldDecList END
void _RecType(TreeNode *tree_node)
{
	//结构体分析
	/*
	w2=record
          integer  x;
          integer  y;
       end;

	DecK RecordK w2
	DecK InRecord IntegerK x
	DecK InRecord IntegerK y
	*/
	tree_node->kind.dec =  RecordK;
	TreeNode *rec_type = NULL;
	match(RECORD);
	//结构体的子变量属于结构体变量的子节点，子节点之间是兄弟节点关系
	rec_type = _FieldDecList();
	tree_node->child[0] = rec_type;
	match(END);
	return ;
}

//23 _FieldDecList ::= _BaseType _IdList; _FieldDecMore | _ArrayType _IdList; _FieldDecMore
TreeNode *_FieldDecList()
{
	//结构体子域分析
	TreeNode *rec_type = new_TreeNode(DecK);
	TreeNode *field_more = NULL;
	switch (curr_token->lex_type)
	{
		case INTEGER:
		case CHAR:
			_BaseType(rec_type);
			break;
		case ARRAY:
			_ArrayType(rec_type);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	_IdList(rec_type);
	match(SEMI);
	field_more = _FieldDecMore();
	rec_type->sibling = field_more;
	return rec_type;
}

//25 _FieldDecMore ::=   | _FieldDecList
TreeNode *_FieldDecMore()
{
	//多于一个子域时，调用more
	TreeNode *field_more = NULL;
	switch (curr_token->lex_type)
	{
		case END:
			break;
		case INTEGER:
		case CHAR:
		case ARRAY:
			field_more = _FieldDecList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return field_more;
}

//27 _IdList ::= ID _IdMore
void _IdList(TreeNode *tree_node)
{
	//结构体内部的变量名列表
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	_IdMore(tree_node);
	return ;
}

//28 _IdMore ::=   | , _IdList
void _IdMore(TreeNode *tree_node)
{
	switch (curr_token->lex_type)
	{
		case SEMI:
			break;
		case COMMA:
			//逗号，idlist还要加
			match(COMMA);
			_IdList(tree_node);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return ;
}

//30 _VarDecpart ::=  | _VarDec
TreeNode *_VarDecpart()
{
	//变量声明
	TreeNode *vard = NULL;
	switch (curr_list->token->lex_type)
	{
		case PROCEDURE:
		case BEGIN:
			break;
		case VAR:
			//遇到var开始变量声明部分
			vard = _VarDec();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return vard;
}

//32 _VarDec ::= VAR _VarDecList
TreeNode *_VarDec()
{
	match(VAR);
	TreeNode *vard = NULL;
	vard = _VarDecList();
	return vard;
}

//33 _VarDecList ::= _TypeDef _VarIdList; _VarDecMore
TreeNode *_VarDecList()
{
	//变量声明分析，返回一个变量声明的链表头
	TreeNode *vard_list = new_TreeNode(DecK);
	TreeNode *vard_more = NULL;
	_TypeDef(vard_list);		//匹配数据类型
	_VarIdList(vard_list);		//变量名
	match(SEMI);
	vard_more = _VarDecMore();
	vard_list->sibling = vard_more;
	return vard_list;
}

//34 _VarDecMore ::=  | _VarDecList
TreeNode *_VarDecMore()
{
	//多于一个变量声明时调用more
	TreeNode *vard_more = NULL;
	switch (curr_token->lex_type)
	{
		case PROCEDURE:
		case BEGIN:
			break;
		case INTEGER:
		case CHAR:
		case ARRAY:
		case RECORD:
		case ID:
			vard_more = _VarDecList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return vard_more;
}

//36 _VarIdList ::= ID _VarIdMore
void _VarIdList(TreeNode *tree_node)
{
	//一行声明中的列表(因为可能有逗号)
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	_VarIdMore(tree_node);
	return ;
}

//37 _VarIdMore ::=  | , _VarIdList
void _VarIdMore(TreeNode *tree_node)
{
	//同上36
	switch (curr_token->lex_type)
	{
		case SEMI:
			break;
		case COMMA:
			match(COMMA);
			_VarIdList(tree_node);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}

//39 _ProcDecpart ::=  | _ProcDec
TreeNode *_ProcDecpart()
{
	//过程声明部分
	TreeNode *proc = NULL;
	switch (curr_token->lex_type)
	{
		case BEGIN:
			break;
		case PROCEDURE:
			//procedure那行，包含函数名，形参表
			proc = _ProcDec();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return proc;
}

//41 _ProcDec ::= PROCEDURE _ProcName (_ParamList); _ProcDecPart _ProcBody _ProcDecMore
TreeNode *_ProcDec()
{
	//procedure那行，包含函数名，形参表
	TreeNode *proc = new_TreeNode(ProcK);
	TreeNode *proc_para = NULL;
	TreeNode *proc_decp = NULL;
	TreeNode *proc_body = NULL;
	TreeNode *proc_more = NULL;
	match(PROCEDURE);
	_ProcName(proc);
	match(LPAREN);	//左括号,左右括号之间是形参
	proc_para = _ParamList();
	int i = 0;
	// 必定有一个 child[0] 节点给proc_para
	proc->child[i] = proc_para;
	i++;
	match(RPAREN);	//右括号
	match(SEMI);
	proc_decp = _ProcDecPart();
	if (proc_decp != NULL) {
		proc->child[i] = proc_decp;
		i++;
	} else {
	}
	proc_body = _ProcBody();
	if (proc_body != NULL) {
		proc->child[i] = proc_body;
	} else {
		fprintf(stderr, "\"Procedure Body!\"\n");
	}
	proc_more = _ProcDecMore();
	proc->sibling = proc_more;
	return proc;
}

//42 _ProcDecMore ::= | _ProcDec
TreeNode *_ProcDecMore()
{
	TreeNode *proc_more = NULL;
	switch (curr_token->lex_type)
	{
		case BEGIN:
			break;
		case PROCEDURE:
			proc_more = _ProcDec();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return proc_more;
}

//44 _ProcName ::= ID
void _ProcName(TreeNode *tree_node)
{
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	return ;
}

//45 _ParamList ::=  | _ParamDecList
TreeNode *_ParamList()
{
	//形参列表
	TreeNode *param_list = NULL;
	switch (curr_token->lex_type)
	{
		case RPAREN:
			break;
		case INTEGER:
		case CHAR:
		case ARRAY:
		case RECORD:
		case ID:
		case VAR:
			//自定义类型和传址变量
			param_list = _ParamDecList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return param_list;
}

//47 _ParamDecList ::= _Param _ParamMore
TreeNode *_ParamDecList()
{
	//返回形参链表头
	TreeNode *param_list = NULL;
	TreeNode *param_more = NULL;
	param_list = _Param();
	param_more = _ParamMore();
	param_list->sibling = param_more;
	return param_list;
}

//48 _ParamMore ::=  | ; _ParamDecList
TreeNode *_ParamMore()
{
	//多于一个形参时调用
	TreeNode *param_more = NULL;
	switch (curr_token->lex_type)
	{
		case RPAREN:
			break;
		case SEMI:
			match(SEMI);
			param_more = _ParamDecList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return param_more;
}

//50 _Param ::= _TypeDef _FormList | VAR _TypeDef _FormList
TreeNode *_Param()
{
	//匹配形参
	TreeNode *param = new_TreeNode(DecK);
	switch (curr_token->lex_type)
	{
		case INTEGER:
		case CHAR:
		case ARRAY:
		case RECORD:
		case ID:
			param->attr.ProcAttr.paramt = valparamType;
			_TypeDef(param);//12数据类型匹配
			_FormList(param);//52变量列表
			break;
		case VAR:
			match(VAR);
			param->attr.ProcAttr.paramt = varparamType;
			_TypeDef(param);
			_FormList(param);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return param;
}

//52 _FormList ::= ID _FidMore
void _FormList(TreeNode *tree_node)
{
	//变量列表
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	_FidMore(tree_node);
	return ;
}

//53 _FidMore ::=  | , _FormList
void _FidMore(TreeNode *tree_node)
{
	//一行之内的形参变量声明，到右括号结束
	switch (curr_token->lex_type)
	{
		case SEMI:
		case RPAREN:
			break;
		case COMMA:
			match(COMMA);
			_FormList(tree_node);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}

//55 _ProcDecPart ::= _DeclarePart
TreeNode *_ProcDecPart()
{
	//函数的内部声明部分
	TreeNode *proc_part = NULL;
	proc_part = _DeclarePart();	//12，调用声明部分函数
	return proc_part;
}

//56 _ProcBody ::= _ProgramBody
TreeNode *_ProcBody()
{
	//返回过程声明的过程体
	TreeNode *proc_body = NULL;
	proc_body = _ProgramBody();
	return proc_body;
}

//57 _ProgramBody ::= BEGIN _StmList END
TreeNode *_ProgramBody()
{
	//返回root->child[2]，程序体StmLK
	TreeNode *stml = new_TreeNode(StmLK);
	TreeNode *stmlist = NULL;
	match(BEGIN);
	stmlist = _StmList();//58
	stml->child[0] = stmlist;
	match(END);
	return stml;
}

//58 _StmList ::= _Stm _StmMore
TreeNode *_StmList()
{
	//语句序列函数，返回(程序体的子链表，)即语句链表
	TreeNode *stmlist = NULL;
	TreeNode *stml_more = NULL;
	stmlist = _Stm();//61
	stml_more = _StmMore();//59
	stmlist->sibling = stml_more;
	return stmlist;
}

//59 _StmMore ::=  | ; _StmList
TreeNode *_StmMore()
{
	TreeNode *stml_more = NULL;
	switch (curr_token->lex_type)
	{
		case ELSE:
		case FI:
		case END:
		case ENDWH:
			break;
		case SEMI:
			match(SEMI);
			stml_more = _StmList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return stml_more;
}

//61 _Stm ::= | _ConditionalStm | _LoopStm | _InputStm | _OutputStm | _ReturnStm | ID _AssCall
TreeNode *_Stm()
{
	//语句类型识别，并跳转到对应类型的处理函数
	TreeNode *stm = NULL;
	switch (curr_token->lex_type)
	{
		case IF:
			stm = _ConditionalStm();//70
			break;
		case WHILE:
			stm = _LoopStm();//71
			break;
		case READ:
			stm = _InputStm();//72
			break;
		case WRITE:
			stm = _OutputStm();//74
			break;
		case RETURN:
			stm = _ReturnStm();//75
			break;
		case ID:
			tmp_name = curr_token->sem_info;
			match(ID);
			stm = _AssCall();//67，ID后接的可能是赋值语句或者子函数调用语句
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return stm;
}

//67 _AssCall ::= _AssignmentRest | _CallStmRest
TreeNode *_AssCall()
{
	TreeNode *ass_call = NULL;
	switch (curr_token->lex_type)
	{
		case ASSIGN:
		case LMIDPAREN:
		case DOT:
			//ID后接"=","[","."是赋值语句
			ass_call = _AssignmentRest();
			break;
		case LPAREN:
			//ID后接"(",是函数调用
			ass_call = _CallStmRest();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return ass_call;
}

//69 _AssignmentRest ::= _VariMore := _Exp
TreeNode *_AssignmentRest()
{
	//创建新的赋值语句类型语法树节点
	TreeNode *ass_rest = new_TreeNode(StmtK);
	ass_rest->kind.stmt = AssignK;
	TreeNode *ass_var = new_TreeNode(ExpK);
	ass_var->kind.exp = VariK;
	strcpy(ass_var->name[ass_var->idnum], tmp_name);
	(ass_var->idnum)++;
	TreeNode *exp = NULL;
	_VariMore(ass_var);
	ass_rest->child[0] = ass_var;
	match(ASSIGN);
	exp = _Exp();//83
	ass_rest->child[1] = exp;
	return ass_rest;
}

//70 _ConditionalStm ::= IF _RelExp THEN _StmList ELSE _StmList FI
TreeNode *_ConditionalStm()
{
	//IF语句
	TreeNode *cond = new_TreeNode(StmtK);
	cond->kind.stmt = IfK;
	TreeNode *rel_exp = NULL;
	TreeNode *then_stm = NULL;
	TreeNode *else_stm = NULL;
	//匹配if，调用表达式函数exp()
	match(IF);
	rel_exp = _RelExp();
	int i = 0;
	if (rel_exp != NULL) {
		cond->child[i] = rel_exp;
		i++;
	} else {
		fprintf(stderr, "RelExp 必须存在\n");
	}
	//匹配then，调用语句序列函数
	match(THEN);
	then_stm = _StmList();
	if (then_stm != NULL) {
		cond->child[i] = then_stm;
		i++;
	} else {
	}
	if (match(ELSE)) {
		else_stm = _StmList();
		if (else_stm != NULL) {
			cond->child[i] = else_stm;
		} else {
		}
	}
	match(FI);
	return cond;
}

//71 _LoopStm ::= WHILE _RelExp DO _StmList ENDWH
TreeNode *_LoopStm()
{
	//循环语句，返回循环语句的树节点
	TreeNode *loop_stm = new_TreeNode(StmtK);
	loop_stm->kind.stmt = WhileK;
	TreeNode *rel_exp = NULL;
	TreeNode *do_stm = NULL;
	//匹配while，调用exp()
	match(WHILE);
	rel_exp = _RelExp();
	int i = 0;
	if (rel_exp != NULL) {
		loop_stm->child[i++] = rel_exp;
	} else {
	}
	//匹配do，调用语句序列函数
	match(DO);
	do_stm = _StmList();
	if (do_stm != NULL) {
		loop_stm->child[i++] = do_stm;
	} else {
	}
	match(ENDWH);
	return loop_stm;
}

//72 _InputStm ::= READ (_Invar)
TreeNode *_InputStm()
{
	//输入语句
	TreeNode *input_stm = new_TreeNode(StmtK);
	input_stm->kind.stmt = ReadK;
	match(READ);
	match(LPAREN);
	//记录标识符名称
	_Invar(input_stm);
	match(RPAREN);
	return input_stm;
}

//73 _Invar ::= ID
void _Invar(TreeNode *tree_node)
{
	if (curr_token->lex_type == ID) {
		strcpy(tree_node->name[tree_node->idnum], curr_token->sem_info);
		(tree_node->idnum)++;
	}
	match(ID);
	return ;
}


//74 _OutputStm ::= WRITE(_Exp)
TreeNode *_OutputStm()
{
	//输出语句处理
	TreeNode *output_stm = new_TreeNode(StmtK);
	output_stm->kind.stmt = WriteK;
	TreeNode *exp = NULL;
	match(WRITE);
	match(LPAREN);
	exp = _Exp();
	output_stm->child[0] = exp;
	match(RPAREN);
	return output_stm;
}

//75 _ReturnStm ::= RETURN
TreeNode *_ReturnStm()
{

	TreeNode *return_stm = new_TreeNode(StmtK);
	return_stm->kind.stmt = ReturnK;
	match(RETURN);
	return return_stm;
}

//76 _CallStmRest ::= (_ActParamList)
TreeNode *_CallStmRest()
{
	//函数调用语句分析
	TreeNode *call_stm = new_TreeNode(StmtK);
	call_stm->kind.stmt = CallK;
	TreeNode *call_var = new_TreeNode(ExpK);
	call_var->kind.exp = VariK;
	strcpy(call_var->name[call_var->idnum], tmp_name);
	(call_var->idnum)++;//同名函数偏移
	call_var->attr.ExpAttr.varkind = IdV;
	call_stm->child[0] = call_var;
	TreeNode *act_param = NULL;
	match(LPAREN);
	act_param = _ActParamList();
	call_stm->child[1] = act_param;
	match(RPAREN);
	return call_stm;
}

//77 _ActParamList ::=  | _Exp _ActParamMore
TreeNode *_ActParamList()
{
	//实参处理
	TreeNode *act_param = NULL;// new_TreeNode(ExpK);
	TreeNode *act_param_more = NULL;
	switch (curr_token->lex_type)
	{
		case RPAREN:
			break;
		case LPAREN://左括号后接id或者intc，调用exp()处理表达式
		case INTC:
		case ID:
			act_param = _Exp();
			act_param_more = _ActParamMore();
			act_param->sibling = act_param_more;
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return act_param;
}

//79 _ActParamMore ::=  | , _ActParamList
TreeNode *_ActParamMore()
{
	TreeNode *act_param_more = NULL;
	switch (curr_token->lex_type)
	{
		case RPAREN:
			break;
		case COMMA:
			match(COMMA);
			act_param_more = _ActParamList();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return act_param_more;
}

//81 _RelExp ::= _Exp _OtherRelE
TreeNode *_RelExp()
{
	//条件表达式,exp otherexp
	TreeNode *rel_exp = new_TreeNode(ExpK);
	rel_exp->kind.exp = OpK;
	TreeNode *exp = NULL;
	exp = _Exp();
	rel_exp->child[0] = exp;
	_OtherRelE(rel_exp);
	return rel_exp;
}

//82 _OtherRelE ::= _CmpOp _Exp
void _OtherRelE(TreeNode *tree_node)
{
	//cmpop exp
	TreeNode *other_exp = NULL;
	_CmpOp(tree_node);
	other_exp = _Exp();
	tree_node->child[1] = other_exp;
	return ;
}

//83 _Exp ::= _Term _OtherTerm
TreeNode *_Exp()
{
	TreeNode *exp = NULL;
	TreeNode *term = NULL;
	TreeNode *other_term = NULL;
	term = _Term();//86
	other_term = _OtherTerm();
	if (other_term == NULL) {
		exp = term;
	} else {
		exp = other_term;
		exp->child[0] = term;
	}
	return exp;
}

//84 _OtherTerm ::=  | _AddOp _Exp
TreeNode *_OtherTerm()
{
	TreeNode *exp = NULL;
	TreeNode *other_term = NULL;
	switch (curr_token->lex_type)
	{
		case LT:
		case EQ:
		case RMIDPAREN:
		case THEN:
		case ELSE:
		case FI:
		case DO:
		case ENDWH:
		case RPAREN:
		case END:
		case SEMI:
		case COMMA:
			break;
		case PLUS:
		case MINUS:
			exp = new_TreeNode(ExpK);
			exp->kind.exp = OpK;
			_AddOp(exp);
			other_term = _Exp();
			exp->child[1] = other_term;
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return exp;
}

//86 _Term ::= _Factor _OtherFactor
TreeNode *_Term()
{
	//项递归，times或over
	TreeNode *term = NULL;
	TreeNode *factor = NULL;
	TreeNode *other_factor = NULL;
	factor = _Factor();
	other_factor = _OtherFactor();
	if (other_factor == NULL) {
		term = factor;
	} else {
		term = other_factor;
		term->child[0] = factor;
	}
	return term;
}

//87 _OtherFactor ::=  | _MultOp _Term
TreeNode *_OtherFactor()
{
	TreeNode *term = NULL;
	TreeNode *other_factor = NULL;
	switch (curr_token->lex_type)
	{
		case LT:
		case EQ:
		case RMIDPAREN:
		case THEN:
		case ELSE:
		case FI:
		case DO:
		case ENDWH:
		case RPAREN:
		case END:
		case SEMI:
		case COMMA:
		case PLUS:
		case MINUS:
			break;
		case TIMES:
		case OVER:
			term = new_TreeNode(ExpK);
			term->kind.exp = OpK;
			_MultOp(term);
			other_factor = _Term();
			term->child[1] = other_factor;
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return term;
}

//89 _Factor ::= (_Exp) | INTC | CHARC | _Variable
TreeNode *_Factor()
{
	//变量处理
	TreeNode *factor = NULL;
	switch (curr_token->lex_type)
	{
		case LPAREN:
			match(LPAREN);
			factor = _Exp();
			match(RPAREN);
			break;
		case INTC:
			factor = new_TreeNode(ExpK);
			factor->kind.exp = ConstK;
			factor->attr.ExpAttr.val = atoi(curr_token->sem_info);
			match(INTC);
			break;
		case CHARC:
			factor = new_TreeNode(ExpK);
			factor->kind.exp = ConstCharK;
			factor->attr.ExpAttr.chval = *curr_token->sem_info;
			match(CHARC);
			break;
		case ID:
			factor = _Variable();
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return factor;
}

//92 _Variable ::= ID _VariMore
TreeNode *_Variable()
{
	TreeNode *variable = new_TreeNode(ExpK);
	variable->kind.exp = VariK;
	if (curr_token->lex_type == ID) {
		strcpy(variable->name[variable->idnum], curr_token->sem_info);
		(variable->idnum)++;
	}
	match(ID);
	_VariMore(variable);
	return variable;
}

//93 _VariMore ::=  | [_Exp] | ._FieldVar
void _VariMore(TreeNode *tree_node)
{
	TreeNode *exp = NULL;
	switch (curr_token->lex_type)
	{
		case ASSIGN:
		case TIMES:
		case OVER:
		case LT:
		case EQ:
		case RMIDPAREN:
		case THEN:
		case ELSE:
		case FI:
		case DO:
		case ENDWH:
		case RPAREN:
		case END:
		case SEMI:
		case COMMA:
		case PLUS:
		case MINUS:
			tree_node->attr.ExpAttr.varkind = IdV;
			break;
		case LMIDPAREN:
			match(LMIDPAREN);
			exp = _Exp();
			exp->attr.ExpAttr.varkind = IdV;
			tree_node->child[0] = exp;
			tree_node->attr.ExpAttr.varkind = ArrayMembV;
			match(RMIDPAREN);
			break;
		case DOT:
			match(DOT);
			exp = _FieldVar();
			exp->attr.ExpAttr.varkind = IdV;
			tree_node->child[0] = exp;
			tree_node->attr.ExpAttr.varkind = FieldMembV;
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return ;
}

//96 _FieldVar ::= ID _FieldVarMore
TreeNode *_FieldVar()
{
	TreeNode *field_var = new_TreeNode(ExpK);
	field_var->kind.exp = VariK;
	if (curr_token->lex_type == ID) {
		strcpy(field_var->name[field_var->idnum], curr_token->sem_info);
		(field_var->idnum)++;
	}
	match(ID);
	_FieldVarMore(field_var);
	return field_var;
}

//97 _FieldVarMore ::=  | [_Exp]
void _FieldVarMore(TreeNode *tree_node)
{
	TreeNode *field_more = NULL;
	switch (curr_token->lex_type)
	{
		case ASSIGN:
		case TIMES:
		case OVER:
		case LT:
		case EQ:
		case RMIDPAREN:
		case THEN:
		case ELSE:
		case FI:
		case DO:
		case ENDWH:
		case RPAREN:
		case END:
		case SEMI:
		case COMMA:
		case PLUS:
		case MINUS:
			tree_node->attr.ExpAttr.varkind = IdV;
			break;
		case LMIDPAREN:
			match(LMIDPAREN);
			field_more = _Exp();
			field_more->attr.ExpAttr.varkind = IdV;
			tree_node->child[0] = field_more;
			tree_node->attr.ExpAttr.varkind = ArrayMembV;
			match(RMIDPAREN);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}

//99 _CmpOp ::= < | =
void _CmpOp(TreeNode *tree_node)
{
	switch (curr_token->lex_type)
	{
		case LT:
			tree_node->attr.ExpAttr.op = LT;
			match(LT);
			break;
		case EQ:
			tree_node->attr.ExpAttr.op = EQ;
			match(EQ);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}

//101 _AddOp ::= + | -
void _AddOp(TreeNode *tree_node)
{
	switch (curr_token->lex_type)
	{
		case PLUS:
			tree_node->attr.ExpAttr.op = PLUS;
			match(PLUS);
			break;
		case MINUS:
			tree_node->attr.ExpAttr.op = MINUS;
			match(MINUS);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}

//103 _MultOp ::= * | /
void _MultOp(TreeNode *tree_node)
{
	switch (curr_token->lex_type)
	{
		case TIMES:
			tree_node->attr.ExpAttr.op = TIMES;
			match(TIMES);
			break;
		case OVER:
			tree_node->attr.ExpAttr.op = OVER;
			match(OVER);
			break;
		default:
			UnexceptError();
			get_next_token();
			break;
	}
	return;
}
