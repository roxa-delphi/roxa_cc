#include <ctype.h>
#include "stdio.h"
#include <stdlib.h>
#include <string.h>


enum {
	TK_NUM = 256,
	TK_EOF,
};

typedef struct {
	int	ty;
	int	val;
	char	*input;
} Token;

enum {
	ND_NUM = 256,
};

typedef struct Node {
	int		ty;
	struct Node	*lhs;
	struct Node 	*rhs;
	int		val;
} Node;

Node	*term();
Node	*mul();
Node	*expr();


Node *new_node(int op, Node *lhs, Node *rhs) {
	Node	*node = malloc(sizeof(Node));
	node->ty  = op;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node	*node = malloc(sizeof(Node));
	node->ty  = ND_NUM;
	node->val = val;
	return node;
}

Token	tokens[100];
int	pos = 0;


//void error(int i) {
//	fprintf(stderr, "予期せぬトークンです: %s\n", tokens[i].input);
//	exit(1);
//}
void error(char *msg, char *p) {
	fprintf(stderr, msg, p);
	exit(1);
}

Node *term() {
	//printf("term() : token = %c,%d\n", tokens[pos].ty, tokens[pos].val);
	if (tokens[pos].ty == TK_NUM)
		return new_node_num(tokens[pos++].val);
	if (tokens[pos].ty == '(') {
		pos++;
		Node	*node = expr();
		if (tokens[pos].ty != ')')
			error("開きカッコに対応する閉じカッコがありません: %s\n", tokens[pos].input);
		pos++;
		return node;
	}
	error("数値でも開きカッコでもないトークンです: %s\n", tokens[pos].input);
}

Node *mul() {
	//printf("mul() : token = %c,%d\n", tokens[pos].ty, tokens[pos].val);
	Node	*lhs = term();
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == ')')
		return lhs;
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs, mul());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs, mul());
	}
	if (tokens[pos].ty == '*') {
		pos++;
		return new_node('*', lhs, mul());
	}
	if (tokens[pos].ty == '/') {
		pos++;
		return new_node('/', lhs, mul());
	}
	error("想定しないトークンです(1): %s\n", tokens[pos].input);
}

Node *expr() {
	//printf("expr(0) : token = %c,%d\n", tokens[pos].ty, tokens[pos].val);
	Node	*lhs = mul();
	//printf("expr(1) : token = %c,%d\n", tokens[pos].ty, tokens[pos].val);
	if (tokens[pos].ty == TK_EOF)
		return lhs;
	if (tokens[pos].ty == ')')
		return lhs;
	if (tokens[pos].ty == '+') {
		pos++;
		return new_node('+', lhs, expr());
	}
	if (tokens[pos].ty == '-') {
		pos++;
		return new_node('-', lhs, expr());
	}
	error("想定しないトークンです(2): %s\n", tokens[pos].input);
}

void gen(Node *node) {
	if (node->ty == ND_NUM) {
		printf("  push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("  pop rdi\n");
	printf("  pop rax\n");

	switch (node->ty) {
	case '+' :
		printf("  add rax, rdi\n");
		break;
	case '-' :
		printf("  sub rax, rdi\n");
		break;
	case '*' :
		printf("  mul rdi\n");
		break;
	case '/' :
		printf("  mov rdx, 0\n");
		printf("  div rdi\n");
	}

	printf("  push rax\n");
}


void tokenize(char *p) {
	int	i = 0;
	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
			tokens[i].ty = *p;
			tokens[i].input = p;
			i++;
			p++;
			continue;
		}

		if (isdigit(*p)) {
			tokens[i].ty = TK_NUM;
			tokens[i].input = p;
			tokens[i].val = strtol(p, &p, 10);
			i++;
			continue;
		}

		fprintf(stderr, "トークナイズできません : %s\n", p);
		exit(1);
	}

	tokens[i].ty = TK_EOF;
	tokens[i].input = p;
}

int debug_tokens() {
	int	i = 0;
	while(tokens[i].ty != TK_EOF) {
		if (tokens[i].ty == TK_NUM)
			printf("tokens[%d].ty = %d\n", i, tokens[i].val);
		else
			printf("tokens[%d].ty = %c\n", i, tokens[i].ty);

		i++;
	}
	printf("tokens[%d].ty = %d\n", i, tokens[i].ty);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "illegal number of argments\n");
		return 1;
	}

	tokenize(argv[1]);
	//debug_tokens();

	Node	*node = expr();


	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	gen(node);

	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}

