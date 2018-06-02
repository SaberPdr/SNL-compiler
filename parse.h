#ifndef PARSE_H
#define PARSE_H

#include <stack>
#include "lex.h"
#include "lex_type.h"
using namespace std;

typedef enum {INProcDec, INRecordDec, INArrayDec, INType, Normal} ParseStates;

typedef enum {ProK, PheadK, DecK, TypeK, VarK, ProcDecK, StmLK, StmtK, ExpK, ProcK} NodeKind;
#define NodeCount 10
extern char *NodeChar[NodeCount];

typedef enum {ArrayK, CharK, IntegerK, RecordK, IdK} DecKind;
#define DecCount 5
extern char *DecChar[DecCount];

typedef enum {IfK, WhileK, AssignK, ReadK, WriteK, CallK, ReturnK} StmtKind;
#define StmtCount 7
extern char *StmtChar[StmtCount];

typedef enum {OpK, ConstK, VariK, ConstCharK} ExpKind;
#define ExpCount 4
extern char *ExpChar[ExpCount];

typedef enum {IdV, ArrayMembV, FieldMembV} VarKind;
#define VarCount 3
extern char *VarChar[VarCount];

typedef enum {Void, Integer, Boolean} ExpType;
#define ExpTypeCount 3
extern char *ExpTypeChar[ExpTypeCount];

typedef enum {valparamType, varparamType} ParamType;
#define ParamCount 2
extern char *ParamChar[ParamCount];

#define MAXCHILDREN 3
#define SYMBTABLE_SIZE 10 // 节点中标识符的最多个数

/********** 语法树节点TreeNode类型 *********/
typedef struct TreeNode {
	struct TreeNode *child[MAXCHILDREN];	/* 子节点指针	*/
	struct TreeNode *sibling;		/* 兄弟节点指针	*/

	int lineno;			/* 源代码行号	*/
	NodeKind nodekind;		    /* 节点类型	*/

	union {
		DecKind  dec;//声明
		StmtKind stmt;//语句
		ExpKind  exp;//表达式
	} kind;                       /* 具体类型 */

	int idnum;                    /* 相同类型的变量个数 */

	char name[SYMBTABLE_SIZE][MAX_INFO_SIZE];            /* 标识符的名称 , MAX_INFO_SIZE 标识符最大长度*/

	// lex_type
	char type_name[MAX_INFO_SIZE];

	struct {
		struct {
			int low;		/* 数组下界     */
			int up;			/* 数组上界     */
			DecKind   childtype;	/* 数组的子类型 */
		} ArrayAttr;			/* 数组属性     */

		struct {
			ParamType  paramt;	/* 过程的参数类型 */
		} ProcAttr;			/* 过程属性 */

		struct {
			LexType op;           /* 表达式的操作符 */
			int val;	      /* 表达式的值 */
			char chval;
			VarKind  varkind;     /* 变量的类别 */
			ExpType type;         /* 用于类型检查 */
		} ExpAttr;	              /* 表达式属性 */

//		char type_name[10];             /* 类型名是标识符 */
	} attr;					/* 属性 */
} TreeNode;

void get_next_token();
void ParseError(TokenType *curr_token, LexType lex_type);
void UnexceptError();
int match(LexType lex_type);

TreeNode *new_TreeNode();
void print_tree(TreeNode *tree_node);
void free_tree(TreeNode *tree_node);

TreeNode *parse(TokenNode *token_list, char *parse_file);
TreeNode *_Program();
TreeNode *_ProgramHead();
void _ProgramName(TreeNode *tree_node);

// Type Declare
TreeNode *_DeclarePart();
TreeNode *_TypeDecpart();
TreeNode *_TypeDec();
TreeNode *_TypeDecList();
TreeNode *_TypeDecMore();
void _TypeId(TreeNode *tree_node);
void _TypeDef(TreeNode *tree_node);
void _BaseType(TreeNode *tree_node);

// Structure type declare
void _StructureType(TreeNode *tree_node);
void _ArrayType(TreeNode *tree_node);
void _low(TreeNode *tree_node);
void _top(TreeNode *tree_node);
void _RecType(TreeNode *tree_node);
TreeNode *_FieldDecList();
TreeNode *_FieldDecMore();
void _IdList(TreeNode *tree_node);
void _IdMore(TreeNode *tree_node);

// Var Declare
TreeNode *_VarDecpart();
TreeNode *_VarDec();
TreeNode *_VarDecList();
TreeNode *_VarDecMore();
void _VarIdList(TreeNode *tree_node);
void _VarIdMore(TreeNode *tree_node);

// Proc Declare
TreeNode *_ProcDecpart();
TreeNode *_ProcDec();
TreeNode *_ProcDecMore();
void _ProcName(TreeNode *tree_node);
TreeNode *_ParamList();
TreeNode *_ParamDecList();
TreeNode *_ParamMore();
TreeNode *_Param();
void _FormList(TreeNode *tree_node);
void _FidMore(TreeNode *tree_node);
TreeNode *_ProcDecPart();
TreeNode *_ProcBody();

TreeNode *_ProgramBody();

TreeNode *_StmList();
TreeNode *_StmMore();
TreeNode *_Stm();

TreeNode *_AssCall();
TreeNode *_AssignmentRest();
TreeNode *_ConditionalStm();
TreeNode *_LoopStm();
TreeNode *_InputStm();
void _Invar(TreeNode *tree_node);
TreeNode *_OutputStm();
TreeNode *_ReturnStm();
TreeNode *_CallStmRest();
TreeNode *_ActParamList();
TreeNode *_ActParamMore();

// Exp
TreeNode *_RelExp();
void _OtherRelE(TreeNode *tree_node);
TreeNode *_Exp();
TreeNode *_OtherTerm();
TreeNode *_Term();
TreeNode *_OtherFactor();
TreeNode *_Factor();
TreeNode *_Variable();
void _VariMore(TreeNode *tree_node);
TreeNode *_FieldVar();
void _FieldVarMore(TreeNode *tree_node);
void _CmpOp(TreeNode *tree_node);
void _AddOp(TreeNode *tree_node);
void _MultOp(TreeNode *tree_node);

#endif
