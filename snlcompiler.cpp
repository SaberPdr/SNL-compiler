#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lex_type.h"
#include "lex.h"
#include "parse.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "缺少参数:\n%s: [argument]\nargument是一个文件名!!\n\n", argv[0]);
		exit(-1);
	}

	fprintf(stdout, "\n读入文件%s......\n开始词法分析...\n", argv[1]);
	//fprintf(stdout,"开始对c1.snl进行语法分析。。");
	TokenNode *token_list = NULL;
	//token_list = lex("c1.snl", "lex.txt");
	token_list = lex(argv[1], "lex.txt");
	fprintf(stdout, "%s的词法分析完成! \
			\nToken输出到 \"lex.txt\".\n", argv[1]);

	fprintf(stdout, "\n开始语法分析......\n");
	TreeNode *tree_node = NULL;
	tree_node = parse(token_list, "parse.txt");
	fprintf(stdout, "语法分析完成，输出到parse.txt\n");

	fprintf(stdout, "全部完成。\n\n");

	free_token(token_list);
	free_tree(tree_node);

	return 0;
}
