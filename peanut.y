%{
	#define PEANUT_DEBUG
	#define GLOBAL
	#include "globals.h"
	#undef GLOBAL

	#include <stdlib.h>
	#include <ctype.h>
	#include <string.h>
	#include <stdbool.h>
	#include "eval.h"
	#include "api.h"

	#define YYDEBUG 1
	#define YYERROR_VERBOSE 1
	
	extern FILE *yyin;
	//extern YYSTYPE yylval;
	//extern char* yytext;

	//extern char token_string[MAX_TOKEN_LENGTH];
	
	pn_node **tree_nodes;
	//int len_tree_nodes = 0;
	
	int yyerrstatus = 0;
	int yylex();
	int yyerror(char* msg);
	
	void lex_push_buffer_file();
	void lex_push_buffer(char *code);
	void lex_pop_buffer();
	
	static pn_node *new_node(NODE_TYPE node_type);
	static pn_node *new_expression(pn_node *object, char *operator, pn_node *other);
	static pn_node *new_property(pn_node *object, pn_node *property);
	static pn_node *add_to_last(pn_node *node, pn_node *next);
	static pn_node *new_if_stmt(pn_node *expression, pn_node *statement_list);
	static pn_node *add_if_stmt(pn_node *node, pn_node *next);
	static pn_node *new_while_stmt(pn_node *expression, pn_node *stmt_list);
	static pn_node *set_object_of_complex_var(pn_node *object, pn_node *chaining_list);
	static pn_node *new_list(pn_node *items);
	static pn_node *new_hash(pn_hash_item *items);
	static pn_hash_item *new_hash_item(pn_node *key, pn_node *value);
	static pn_hash_item *add_hash_item_to_last(pn_hash_item *item, pn_hash_item *next);
	static pn_node *new_for_stmt(pn_node *var, pn_node *expression, pn_node *stmt_list);
%}

%union {
	int token;
	pn_node *node;
	char *text;
	pn_hash_item *hash_item;
}

%token <token> CLASS DEF ELIF ELSE END ENDL FOR IF IMPORT IN RETURN WHILE PN_NULL
%token <text> PLUS MINUS MULT DIVIDE MOD NOT ASSIGN EQ NEQ AND OR LT LTEQ GT GTEQ
%token <text> ID STRING INTEGER REAL

%type <node> program codeblock_list codeblock declaration class_decl func_decl_list superclass
%type <node> empty_stmt function_decl import_decl func_decl simple_var_list simple_var statement_list statement expression_stmt expression
%type <node> simple_expr additive_expr term factor literal list hash
%type <node> complex_var chaining_list funcall parameter_list lambda_decl multiline_lambda_decl inline_lambda_decl
%type <node> return_stmt if_stmt elseif_stmt_list else_stmt for_stmt while_stmt

%type <hash_item> hash_item_list hash_item

%type <text> operator add_op rel_op mul_op func_id


%start program

%%

program				: codeblock_list			{ peanut_tree = $1; }
					;

codeblock_list		: codeblock_list codeblock	{ $$ = add_to_last($1, $2); }
					| codeblock					{ $$ = $1; }
					;

codeblock			: declaration				{ $$ = $1; }
					| statement					{ $$ = $1; }
					;

declaration			: import_decl				{ $$ = $1; }
					| class_decl				{ $$ = $1; }
					| function_decl				{ $$ = $1; }
					;

class_decl			: CLASS ID superclass ENDL func_decl_list END ENDL
						{
							$$ = new_node(NODE_DEF_CLASS);
							$$->def_class.name = $2;
							$$->def_class.super_list = $3;
							$$->def_class.func_list = $5;
						}
					| CLASS ID superclass ENDL END ENDL
						{
							$$ = new_node(NODE_DEF_CLASS);
							$$->def_class.name = $2;
							$$->def_class.super_list = $3;
							$$->def_class.func_list = NULL;
						}
					;

func_decl_list		: func_decl_list function_decl	{ $$ = add_to_last($1, $2); }
					| function_decl					{ $$ = $1; }
					;

superclass			: LT simple_var_list			{ $$ = $2; }
					| /* empty */					{ $$ = NULL; }
					;

empty_stmt			: ENDL							{ $$ = NULL; }
					;

function_decl		: func_decl ENDL				{ $$ = $1; }
					;

import_decl			: IMPORT STRING ENDL
						{
							// TODO: 이번 버전에서, import 는 하지 않기로 했음.
						}
					;

func_decl			: DEF func_id '(' simple_var_list ')' ENDL statement_list END
						{
							pn_node *node = new_node(NODE_DEF_FUNC);
							node->def_func.func_id = $2;
							node->def_func.simple_var_list = $4;
							node->def_func.stmt_list = $7;
							$$ = node;
						}
					| DEF func_id '(' simple_var_list ')' ENDL END
						{
							pn_node *node = new_node(NODE_DEF_FUNC);
							node->def_func.func_id = $2;
							node->def_func.simple_var_list = $4;
							node->def_func.stmt_list = NULL;
							$$ = node;
						}
					;

func_id				: operator	{ $$ = strdup($1); }
					| ID		{ $$ = strdup($1); }
					;

operator			: PLUS		{ $$ = $1; }
					| MINUS		{ $$ = $1; }
					| MULT		{ $$ = $1; }
					| DIVIDE	{ $$ = $1; }
					| MOD		{ $$ = $1; }
					| NOT		{ $$ = $1; }
					| ASSIGN	{ $$ = $1; }
					| EQ		{ $$ = $1; }
					| NEQ		{ $$ = $1; }
					| AND		{ $$ = $1; }
					| OR		{ $$ = $1; }
					| LT		{ $$ = $1; }
					| LTEQ		{ $$ = $1; }
					| GT		{ $$ = $1; }
					| GTEQ		{ $$ = $1; }
					;

simple_var			: ID
						{
							pn_node *node = new_node(NODE_VAR_NAME);
							node->var_name = $1;
							$$ = node;
						}
					;

simple_var_list		: simple_var_list ',' simple_var
						{
							$$ = add_to_last($1, $3);
						}
					| simple_var
						{
							$$ = $1;
						}
					| /*empty*/
						{
							$$ = NULL;
						}
					;

statement_list		: statement_list statement		{ $$ = add_to_last($1, $2); }
					| statement						{ $$ = $1; }
					;

statement			: expression_stmt		{ $$ = $1; }
					| return_stmt			{ $$ = $1; }
					| if_stmt				{ $$ = $1; }
					| for_stmt				{ $$ = $1; }
					| while_stmt			{ $$ = $1; }
					| empty_stmt			{ $$ = $1; }
					;

/* expressions ******************************************************/

expression_stmt		: expression ENDL		{ $$ = $1; }
					;

expression			: complex_var ASSIGN simple_expr		{ $$ = new_expression($1, $2, $3); }
					| simple_expr							{ $$ = $1; }
					;

simple_expr			: additive_expr rel_op additive_expr	{ $$ = new_expression($1, $2, $3); }
					| additive_expr							{ $$ = $1; }
					;

rel_op				: LTEQ					{ $$ = $1; }
					| LT					{ $$ = $1; }
					| GT					{ $$ = $1; }
					| GTEQ					{ $$ = $1; }
					| EQ					{ $$ = $1; }
					| NEQ					{ $$ = $1; }
					;

additive_expr		: additive_expr add_op term				{ $$ = new_expression($1, $2, $3); }
					| term									{ $$ = $1; }
					;

add_op				: PLUS					{ $$ = $1; }
					| MINUS					{ $$ = $1; }
					;

term				: term mul_op factor					{ $$ = new_expression($1, $2, $3); }
					| factor								{ $$ = $1; }
					;

mul_op				: MULT					{ $$ = $1; }
					| DIVIDE				{ $$ = $1; }
					;

factor				: '(' expression ')'	{ $$ = $2; }
					| lambda_decl			{ $$ = $1; }
					| list					{ $$ = $1; }
					| hash					{ $$ = $1; }
					| complex_var			{ $$ = $1; }
					| literal				{ $$ = $1; }
					;

literal				: STRING
		   				{
							pn_node *node = new_node(NODE_LITERAL);
							node->value.type = TYPE_STRING;
							node->value.str_val = $1;
							$$ = node;
		   				}
					| REAL
		   				{
							pn_node *node = new_node(NODE_LITERAL);
							node->value.type = TYPE_REAL;
							node->value.real_val = atof($1);
							$$ = node;
							//free($1);
		   				}
					| INTEGER
		   				{
							pn_node *node = new_node(NODE_LITERAL);
							node->value.type = TYPE_INTEGER;
							node->value.int_val = atoi($1);
							$$ = node;
							//free($1);
		   				}
					| PN_NULL
						{
							pn_node *node = new_node(NODE_LITERAL);
							node->value.type = TYPE_NULL;
							$$ = node;
						}
					;

list				: '[' parameter_list ',' ']'
						{
							$$ = new_list($2);
						}
					| '[' parameter_list ']'
						{
							$$ = new_list($2);
						}
					;

hash				: '{' hash_item_list ',' '}'
						{
							$$ = new_hash($2);
						}
					| '{' hash_item_list '}'
						{
							$$ = new_hash($2);
						}
					;

hash_item_list		: hash_item_list ',' hash_item
						{
							$$ = add_hash_item_to_last($1, $3);
						}
					| hash_item
						{
							$$ = $1;
						}
					| /*empty*/
						{
							$$ = NULL;
						}
					;

hash_item			: simple_expr ':' simple_expr
						{
							$$ = new_hash_item($1, $3);
						}
					;

complex_var			: list '.' chaining_list
						{
							$$ = set_object_of_complex_var($1, $3);
						}
					| hash '.' chaining_list
						{
							$$ = set_object_of_complex_var($1, $3);
						}
					| lambda_decl '.' chaining_list
						{
							$$ = set_object_of_complex_var($1, $3);
						}
					| literal '.' chaining_list
						{
							$$ = set_object_of_complex_var($1, $3);
						}
					| '(' expression ')' '.' chaining_list
						{
							$$ = set_object_of_complex_var($2, $5);
						}
					| chaining_list
						{
							$$ = $1;
						}
					;

chaining_list		: chaining_list '.' simple_var
						{
							$$ = new_property($1, $3);
						}
					| chaining_list '.' funcall
						{
							$3->expr.object = $1;
							$$ = $3;
						}
					| simple_var			{ $$ = $1; }
					| funcall				{ $$ = $1; }
					;

funcall				: func_id '(' parameter_list ')'	{ $$ = new_expression(NULL, $1, $3); }
					;

parameter_list		: parameter_list ',' expression
						{
							$$ = add_to_last($1, $3);
						}
					| expression
						{
							$$ = $1;
						}
					| /*empty*/
						{
							$$ = NULL;
						}
					;

lambda_decl			: multiline_lambda_decl		{ $$ = $1; }
					| inline_lambda_decl		{ $$ = $1; }
					;

multiline_lambda_decl : '{' '|' simple_var_list '|' statement_list '}'
						{
							pn_node *node = new_node(NODE_LAMBDA);
							node->lambda.simple_var_list = $3;
							node->lambda.stmt_list = $5;
							$$ = node;
						}
					;

inline_lambda_decl	: '{' '|' simple_var_list '|' expression '}'
						{
							pn_node *node = new_node(NODE_LAMBDA);
							node->lambda.simple_var_list = $3;
							node->lambda.stmt_list = $5;
							$$ = node;
						}
					;


/* statements ******************************************************/

return_stmt			: RETURN expression ENDL
						{
							pn_node *node = new_node(NODE_RETURN_STMT);
							node->return_stmt.expr = $2;
							$$ = node;
						}
					| RETURN ENDL
						{
							pn_node *node = new_node(NODE_RETURN_STMT);
							node->return_stmt.expr = NULL;
							$$ = node;
						}
					;

if_stmt				: IF expression ENDL elseif_stmt_list
						{
							pn_node *node = new_if_stmt($2, NULL);
							$$ = add_if_stmt(node, $4);
						}
					| IF expression ENDL statement_list elseif_stmt_list
						{
							pn_node *node = new_if_stmt($2, $4);
							$$ = add_if_stmt(node, $5);
						}
					;

elseif_stmt_list	: else_stmt
						{
							$$ = $1;
						}
					| ELIF expression ENDL elseif_stmt_list
						{
							pn_node *node = new_if_stmt($2, NULL);
							$$ = add_if_stmt(node, $4);
						}
					| ELIF expression ENDL statement_list elseif_stmt_list
						{
							pn_node *node = new_if_stmt($2, $4);
							$$ = add_if_stmt(node, $5);
						}
					;

else_stmt			: END ENDL
						{
							$$ = NULL;
						}
					| ELSE statement_list END ENDL
						{
							$$ = new_if_stmt(NULL, $2);
						}
					;

for_stmt			: FOR simple_var IN expression ENDL statement_list END ENDL
						{
							$$ = new_for_stmt($2, $4, $6);
						}
					| FOR simple_var IN expression ENDL END ENDL
						{
							$$ = new_for_stmt($2, $4, NULL);
						}
					;

while_stmt			: WHILE expression ENDL statement_list END ENDL
						{
							$$ = new_while_stmt($2, $4);
						}
					| WHILE expression ENDL END ENDL
						{
							$$ = new_while_stmt($2, NULL);
						}
					;

%%


void
usage()
{
	fprintf(stdout,
		"Usage: ./peanut [OPTION].. [FILE]\n" \
		"  -t : testmode\n" \
		"  -h : help\n" \
		"       With no FILE, read standard input.\n"
	);
}


static void
run_repl(pn_world *world)
{
	static unsigned int lineno = 0;
	char *total = (char *)pn_alloc(sizeof(char) * INPUT_STR_BUF * 30);
	char input[INPUT_STR_BUF];
	
	while (1)
	{
		total[0] = 0;
		
		while (1)
		{
			fprintf(stdout, "[%u]>> ", lineno);
			fgets(input, INPUT_STR_BUF, stdin);
			if (strcmp("\n", input) == 0)
				break;
			strcat(total, input);
		}
		lineno++;
		
		pn_object *ret = Peanut_EvalFromString(total, world, true);
		
		if (ret == NULL)
			fprintf(stderr, "# parse error.\n");
	}
	
	free(total);
}


void multi_test()
{
	pn_world* worlds[2];
	worlds[0] = Peanut_CreateWorld();
	//worlds[1] = Peanut_CreateWorld();
	
	pn_object* result = NULL;
	
	fprintf(stderr, "def function[%d, %p]\n", 0, worlds[0]);
	result = Peanut_EvalFromString("def strategy(id, params)\n\t9999\nend\n", worlds[0], true);
	PN_ASSERT(result != NULL);
	fprintf(stderr, "end def[%d]\n", 0);
	
	/*
	fprintf(stderr, "def function[%d, %p]\n", 1, worlds[1]);
	result = Peanut_EvalFromString("def strategy(id, params)\n\t100000\nend\n", worlds[1], true);
	PN_ASSERT(result != NULL);
	fprintf(stderr, "end def[%d]\n", 1);
	*/
	
	int i;
	for (i = 0; i < 1000; i++)
	{
		fprintf(stderr, "TEST[%d, %p] start\n", i, worlds[0]);
		result = Peanut_EvalFromString("1\n", worlds[0], true);
		//result = Peanut_EvalFromString("strategy(100, 300)\n", worlds[0], true);
		PN_ASSERT(result != NULL);
		fprintf(stderr, "TEST[%d, %p] end\n", i, worlds[0]);
		
		/*
		fprintf(stderr, "TEST[%d, %p] start\n", i, worlds[1]);
		result = Peanut_EvalFromString("strategy(123, 1234)\n", worlds[1], true);
		PN_ASSERT(result != NULL);
		fprintf(stderr, "TEST[%d, %p] end\n", i, worlds[1]);
		*/
	}
	
	Peanut_DestroyWorld(worlds[0]);
	//Peanut_DestroyWorld(worlds[1]);
}


#ifdef STANDALONE


static int readfile(const char* filename, char** result)
{
	int size = 0;
	FILE *f = fopen(filename, "r");
	if (f == NULL)
		return -1;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char*)pn_alloc(sizeof(char*) * size + 1);
	if (size != fread(*result, sizeof(char), size, f)) {
		free(*result);
		return -2;
	}
	fclose(f);
	(*result)[size] = 0;
	return size;
}

char* filenames[100];

/**
 * peanut 의 메인 함수
 */
int
main(int argc, char *argv[]) {
	bool trace = false;
	bool with_repl = false;
	bool eval_test = false;
	int filename_count = 0;
	
	int i;
	for (i = 1; i < argc; i++)
	{
		if (strcmp("-h", argv[i]) == 0)
		{
			usage();
			return 0;
		}
		else if (strcmp("-t", argv[i]) == 0)
		{
			trace = true;
		}
		else if (strcmp("-i", argv[i]) == 0)
		{
			with_repl = true;
		}
		else if (strcmp("--eval-test", argv[i]) == 0)
		{
			eval_test = true;
		}
		else if (strcmp("--multi-test", argv[i]) == 0)
		{
			multi_test();
			return 0;
		}
		else
		{
			filenames[filename_count++] = argv[i];
		}
	}
	
	pn_world *world = Peanut_CreateWorld();
	
	// 해당 파일을 실행하거나, 쉘모드로 동작한다.
	if (filename_count > 0)
	{
		for (i = 0; i < filename_count; i++)
		{
			if (eval_test)
			{
				char *code;
				int size = readfile(filenames[i], &code);
				if (size > 0)
					Peanut_EvalFromString(code, world, trace);
			}
			else
			{
				Peanut_EvalFromFile(filenames[i], world, trace);
			}
		}
		
		if (with_repl)
			run_repl(world);
	}
	else
	{
		run_repl(world);
	}
	
	Peanut_DestroyWorld(world);
	
	return 0;
}
#endif


pn_world *
Peanut_CreateWorld()
{
	return World_Create();
}

pn_object *
Peanut_EvalFromFile(char *filename, pn_world *world, bool trace)
{
	FILE* backup = NULL;
	
	if (filename != NULL)
	{
		backup = yyin;
		yyin = fopen(filename, "r");
		if (yyin == NULL)
		{
			fprintf(stderr, "file open error. %s\n", filename);
			return NULL;
		}
	}
	
	// 버퍼를 바꿨다가..
	lex_push_buffer_file();
	
	//len_tree_nodes = 0;
	//len_tree_nodes = world->len_tree_nodes;
	//tree_nodes = (pn_node**)pn_alloc(sizeof(pn_node*) * SIZE_TREE_NODES);
	int parsed = yyparse();
	
	// 버퍼를 원래대로..
	lex_pop_buffer();
	
	if (filename != NULL)
		fclose(yyin);
	
	// 파일 복구
	yyin = backup;
	
	if (parsed == 0)
	{
		//world->tree = peanut_tree;
		//world->tree_nodes = tree_nodes;
		//world->len_tree_nodes = len_tree_nodes;
		//return Eval_ExecuteTree(world, world->tree, trace);
		return Eval_ExecuteTree(world, peanut_tree, trace);
	}
	else
	{
		yyclearin;
		yyerrok;
		
		//free(tree_nodes);
		return NULL;
	}
}

pn_object *
Peanut_EvalFromString(char *code, pn_world *world, bool trace)
{
	// 버퍼를 바꿨다가..
	lex_push_buffer(code);
	
	//len_tree_nodes = 0;
	//tree_nodes = (pn_node**)pn_alloc(sizeof(pn_node*) * SIZE_TREE_NODES);
	//len_tree_nodes = world->len_tree_nodes;
	int parsed = yyparse();
	
	// 버퍼를 원래대로..
	lex_pop_buffer();
	
	if (parsed == 0)
	{
		//world->tree = peanut_tree;
		//world->tree_nodes = tree_nodes;
		//world->len_tree_nodes = len_tree_nodes;
		//return Eval_ExecuteTree(world, world->tree, trace);
		return Eval_ExecuteTree(world, peanut_tree, trace);
	}
	else
	{
		//free(tree_nodes);
		return NULL;
	}
}


void
Peanut_DestroyWorld(pn_world *world)
{
	//int i;
	//FIXME: -_-a
	//for (i = 0; i < world->len_tree_nodes; i++)
	//	free(world->tree_nodes[i]);
	
	//free(world->tree_nodes);
	World_Destroy(world);
}


/**
 * yacc 문법 에러 발생시에 실행되는 함수.
 */
int
yyerror(char *msg) {
	fprintf(stderr, "# Error: %s\n", msg);
	return 0;
}

/**
 * 새로운 노드를 생성한다.
 */
pn_node nodes[100000];
int node_index = 0;
static pn_node *
new_node(NODE_TYPE node_type)
{
	//pn_node *node = (pn_node *)pn_alloc(sizeof(pn_node));
	pn_node *node = &nodes[node_index++];
	
	// node 를 메모리 관리를 위해서 리스트에 넣어준다.

	//FIXME: -_-a
	//tree_nodes[len_tree_nodes++] = node;
	
	//ANDLOG("len_tree_nodes=%d\n", len_tree_nodes);
	
	PN_ASSERT(node != NULL);
	memset(node, 0, sizeof(node));
	node->node_type = node_type;
	//node->lineno = g_line_no;
	return node;
}

/**
 * expression 노드 생성
 */
static pn_node *
new_expression(pn_node *object, char *operator, pn_node *other)
{
	pn_node *node = new_node(NODE_EXPRESSION);
	PN_ASSERT(node != NULL);
	node->expr.object = object;
	node->expr.func_name = operator;
	node->expr.params = other;
	return node;
}

/**
 * 프로퍼티 노드 생성
 */
static pn_node *
new_property(pn_node *object, pn_node *property)
{
	pn_node *node = new_node(NODE_EXPRESSION);
	PN_ASSERT(node != NULL);
	PN_ASSERT(object != NULL);
	PN_ASSERT(property != NULL);
	PN_ASSERT(property->node_type == NODE_VAR_NAME);
	
	node->expr.object = object;
	node->expr.func_name = "@";
	node->expr.params = property;
	return node;
}

/**
 * node의 마지막에 next를 넣는다.
 */
static pn_node *
add_to_last(pn_node *node, pn_node *next)
{
	pn_node *root = NULL, *t = node;
	if (t != NULL)
	{
		while (t->sibling != NULL)
			t = t->sibling;
		t->sibling = next;
		root = node;
	}
	else
	{
		root = next;
	}
	return root;
}

/**
 * if_stmt 의 자식으로 next를 붙인다.
 */
static pn_node *
add_if_stmt(pn_node *node, pn_node *next)
{
	pn_node *root = NULL, *t = node;
	if (t != NULL)
	{
		while (t->if_stmt.next != NULL)
			t = t->if_stmt.next;
		t->if_stmt.next = next;
		root = node;
	}
	else
	{
		root = next;
	}
	return root;
}

/**
 * if_stmt 를 생성한다.
 */
static pn_node *
new_if_stmt(pn_node *expression, pn_node *statement_list)
{
	pn_node *node = new_node(NODE_IF_STMT);
	PN_ASSERT(node != NULL);
	node->if_stmt.expr = expression;
	node->if_stmt.stmt_list = statement_list;
	return node;
}

/**
 * while_stmt 를 생성한다.
 */
static pn_node *
new_while_stmt(pn_node *expression, pn_node *stmt_list)
{
	pn_node *node = new_node(NODE_WHILE_STMT);
	PN_ASSERT(node != NULL);
	node->while_stmt.expr = expression;
	node->while_stmt.stmt_list = stmt_list;
	return node;
}

/**
 * object 를 chaining_list 의 앞에다가 붙인다.
 */
static pn_node *
set_object_of_complex_var(pn_node *object, pn_node *chaining_list)
{
	PN_ASSERT(object != NULL);
	PN_ASSERT(chaining_list != NULL);
	
	pn_node *first = chaining_list;
	pn_node *term = first;
	
	while (first != NULL)
	{
		PN_ASSERT(first->node_type == NODE_EXPRESSION);
		term = first;
		first = first->expr.object;
	}
	
	term->expr.object = object;
	return chaining_list;
}

/**
 * 새 list 노드를 만든다.
 */
static pn_node *
new_list(pn_node *items)
{
	pn_node *node = new_node(NODE_LIST);
	PN_ASSERT(node != NULL);
	node->list_items = items;
	return node;
}

/**
 * 새 hash 노드를 만든다.
 */
static pn_node *
new_hash(pn_hash_item *items)
{
	pn_node *node = new_node(NODE_HASH);
	PN_ASSERT(node != NULL);
	node->hash_items = items;
	return node;
}

/**
 * 새 hash_item 노드를 만든다.
 */
static pn_hash_item *
new_hash_item(pn_node *key, pn_node* value)
{
	pn_hash_item *item = pn_alloc(sizeof(pn_hash_item));
	PN_ASSERT(item != NULL);
	item->key = key;
	item->value = value;
	return item;
}

/**
 * hash_item의 마지막에 next를 넣는다.
 */
static pn_hash_item *
add_hash_item_to_last(pn_hash_item *item, pn_hash_item *next)
{
	pn_hash_item *root = NULL, *t = item;
	if (t != NULL)
	{
		while (t->next_item != NULL)
			t = t->next_item;
		t->next_item = next;
		root = item;
	}
	else
	{
		root = next;
	}
	return root;
}

/**
 * 새 for문 노드 트리를 만든다.
 */
static pn_node *
new_for_stmt(pn_node *var, pn_node *expression, pn_node *stmt_list)
{
	pn_node *node = new_node(NODE_FOR_STMT);
	PN_ASSERT(node != NULL);
	node->for_stmt.var_name = var->var_name;
	node->for_stmt.expr = expression;
	node->for_stmt.stmt_list = stmt_list;
	
	//free(var);
	
	return node;
}
