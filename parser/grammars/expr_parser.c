/**
 * simple LL(1) expression parser -- C version
 *
 * @author Tobias Weber
 * @date 14-mar-2020, 8-may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * References:
 *	- https://www.cs.uaf.edu/~cs331/notes/FirstFollow.pdf
 *	- https://de.wikipedia.org/wiki/LL(k)-Grammatik
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <math.h>


// ----------------------------------------------------------------------------
// definitions
// ----------------------------------------------------------------------------
#define MAX_IDENT 256


#define USE_INTEGER
#ifdef USE_INTEGER
	typedef int t_value;
#else
	typedef double t_value;
#endif


enum Token
{
	TOK_VALUE   = 1000,
	TOK_IDENT   = 1001,
	TOK_END     = 1002,

	TOK_INVALID = 10000,
};


static int g_lookahead = TOK_INVALID;
static t_value g_lookahead_val = 0;
static char g_lookahead_text[MAX_IDENT];


static t_value plus_term();
static t_value plus_term_rest(t_value arg);
static t_value mul_term();
static t_value mul_term_rest(t_value arg);
static t_value pow_term();
static t_value pow_term_rest(t_value arg);
static t_value factor();
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// helper functions
// ----------------------------------------------------------------------------

static void strncat_char(char* str, char c, int max_len)
{
	int len = strlen(str);
	if(len+1 < max_len)
	{
		str[len] = c;
		str[len+1] = 0;
	}
}

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// symbol table
// ----------------------------------------------------------------------------

struct Symbol
{
	char name[MAX_IDENT];
	t_value value;

	struct Symbol* next;
};


static struct Symbol g_symboltable;


struct Symbol* create_symbol(const char* name, t_value value)
{
	struct Symbol *sym = (struct Symbol*)malloc(sizeof(struct Symbol));
	strncpy(sym->name, name, MAX_IDENT);
	sym->value = value;
	sym->next = 0;

	return sym;
}


void init_symbols()
{
	strncpy(g_symboltable.name, "", MAX_IDENT);
	g_symboltable.value = 0;

	struct Symbol *sym = create_symbol("pi", M_PI);
	g_symboltable.next = sym;
}


void deinit_symbols()
{
	struct Symbol *sym = g_symboltable.next;

	while(sym)
	{
		struct Symbol *symnext = sym->next;
		free(sym);
		sym = symnext;
	}

	g_symboltable.next = 0;
}


struct Symbol* find_symbol(const char* name)
{
	struct Symbol* sym = &g_symboltable;

	while(sym)
	{
		if(strncmp(sym->name, name, MAX_IDENT) == 0)
			return sym;

		sym = sym->next;
	}

	return sym;
}


struct Symbol* assign_or_insert_symbol(const char* name, t_value value)
{
	struct Symbol* sym = find_symbol(name);

	if(sym)
	{
		sym->value = value;
	}
	else
	{
		sym = create_symbol(name, value);

		struct Symbol *table = &g_symboltable;
		while(1)
		{
			if(table->next == 0)
			{
				table->next = sym;
				break;
			}

			table = table->next;
		}
	}

	return sym;
}


void print_symbols()
{
	const struct Symbol* sym = g_symboltable.next;

	while(sym)
	{
#ifdef USE_INTEGER
		printf("%s = %d\n", sym->name, sym->value);
#else
		printf("%s = %f\n", sym->name, sym->value);
#endif
		sym = sym->next;
	}
}
// ------------------------------------------------------------------------



// ------------------------------------------------------------------------
// lexer
// ------------------------------------------------------------------------


static int match_int(const char* tok)
{
	int len = strlen(tok);
	for(int i=0; i<len; ++i)
	{
		if(!isdigit(tok[i]))
			return 0;
	}
	return 1;
}


static int match_real(const char* tok)
{
	// TODO
	return match_int(tok);
}


static int match_ident(const char* tok)
{
	int len = strlen(tok);
	if(len == 0)
		return 0;
	if(!isalpha(tok[0]))
		return 0;

	for(int i=1; i<len; ++i)
	{
		if(!isdigit(tok[i]) || !isalpha(tok[i]))
			return 0;
	}

	return 1;
}


/**
 * find all matching tokens for input string
 */
static int get_matching_token(const char* str, t_value* value)
{
#ifdef USE_INTEGER
	{	// int
		if(match_int(str))
		{
			t_value val = 0;
			val = atoi(str);

			*value = val;
			return TOK_VALUE;
		}
	}
#else
	{	// real
		if(match_real(str))
		{
			t_value val = 0.;
			val = atof(str);

			*value = val;
			return TOK_VALUE;
		}
	}
#endif

	{	// ident
		if(match_ident(str))
		{
			*value = 0;
			return TOK_IDENT;
		}
	}

	{	// tokens represented by themselves
		if(strcmp(str, "+")==0 || strcmp(str, "-")==0 || strcmp(str, "*")==0 ||
			strcmp(str, "/")==0 || strcmp(str, "%")==0 || strcmp(str, "^")==0 ||
			strcmp(str, "(")==0 || strcmp(str, ")")==0 || strcmp(str, ",")==0 ||
			strcmp(str, "=")==0)
		{
			*value = 0;
			return (int)str[0];
		}
	}

	*value = 0;
	return TOK_INVALID;
}


static int g_input_idx = 0;
static int g_input_len = 0;
static const char* g_input = 0;


static void set_input(const char* input)
{
	g_input = input;
	g_input_len = strlen(g_input);
	g_input_idx = 0;
}


static int input_get()
{
	if(g_input_idx >= g_input_len)
		return EOF;

	return g_input[g_input_idx++];
}


static int input_peek()
{
	if(g_input_idx+1 >= g_input_len)
		return EOF;

	return g_input[g_input_idx+1];
}


static void input_putback(/*char c*/)
{
	if(g_input_idx > 0)
		--g_input_idx;
}


static int input_eof()
{
	return g_input_idx >= g_input_len;
}


/**
 * @return token, yylval, yytext
 */
static int lex(t_value* lval, char* text)
{
	char input[MAX_IDENT];
	char longest_input[MAX_IDENT];
	input[0] = 0;
	longest_input[0] = 0;

	int longest_matching_token = TOK_INVALID;
	t_value longest_matching_value = 0;

	while(1)
	{
		int c = input_get();
		if(c == EOF)
			break;

		// if outside any other match...
		if(longest_matching_token == TOK_INVALID)
		{
			// ...ignore white spaces
			if(c==' ' || c=='\t')
				continue;
			// ...end on new line
			if(c=='\n')
			{
				*lval = 0;
				strncpy(text, longest_input, MAX_IDENT);
				return TOK_END;
			}
		}

		strncat_char(input, c, MAX_IDENT);
		t_value matching_val = 0;
		int matching = get_matching_token(input, &matching_val);
		if(matching != TOK_INVALID)
		{
			strncpy(longest_input, input, MAX_IDENT);
			longest_matching_token = matching;
			longest_matching_value = matching_val;

			if(input_peek()==EOF || input_eof())
				break;
		}
		else
		{
			// no more matches
			input_putback(/*c*/);
			break;
		}
	}

	// at EOF
	if(longest_matching_token == TOK_INVALID && strlen(input) == 0)
	{
		*lval = 0;
		strncpy(text, longest_input, MAX_IDENT);
		return TOK_END;
	}

	// nothing matches
	if(longest_matching_token == TOK_INVALID)
	{
		fprintf(stderr, "Invalid input in lexer: \"%s\".\n", input);

		*lval = 0;
		strncpy(text, longest_input, MAX_IDENT);
		return TOK_INVALID;
	}

	// found match
	if(longest_matching_token != TOK_INVALID)
	{
		*lval = longest_matching_value;
		strncpy(text, longest_input, MAX_IDENT);
		return longest_matching_token;
	}

	// should not get here
	*lval = 0;
	strncpy(text, longest_input, MAX_IDENT);
	return TOK_INVALID;
}
// ------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// lexer interface
// ----------------------------------------------------------------------------
void next_lookahead()
{
	g_lookahead = lex(&g_lookahead_val, g_lookahead_text);
}


int match(int expected)
{
	if(g_lookahead != expected)
	{
		fprintf(stderr, "Could not match symbol! Expected: %d, got %d.\n", expected, g_lookahead);
		return 0;
	}

	return 1;
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// productions
// ----------------------------------------------------------------------------
/**
 * +,- terms
 * (lowest precedence, 1)
 */
static t_value plus_term()
{
	// plus_term -> mul_term plus_term_rest
	if(g_lookahead == '(' || g_lookahead == TOK_VALUE || g_lookahead == TOK_IDENT)
	{
		t_value term_val = mul_term();
		t_value expr_rest_val = plus_term_rest(term_val);

		return expr_rest_val;
	}
	else if(g_lookahead == '+')	// unary +
	{
		next_lookahead();
		t_value term_val = mul_term();
		t_value expr_rest_val = plus_term_rest(term_val);

		return expr_rest_val;
	}
	else if(g_lookahead == '-')	// unary -
	{
		next_lookahead();
		t_value term_val = -mul_term();
		t_value expr_rest_val = plus_term_rest(term_val);

		return expr_rest_val;
	}

	if(g_lookahead == 0 || g_lookahead == EOF)
		exit(0);

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


static t_value plus_term_rest(t_value arg)
{
	// plus_term_rest -> '+' mul_term plus_term_rest
	if(g_lookahead == '+')
	{
		next_lookahead();
		t_value term_val = arg + mul_term();
		t_value expr_rest_val = plus_term_rest(term_val);

		return expr_rest_val;
	}

	// plus_term_rest -> '-' mul_term plus_term_rest
	else if(g_lookahead == '-')
	{
		next_lookahead();
		t_value term_val = arg - mul_term();
		t_value expr_rest_val = plus_term_rest(term_val);

		return expr_rest_val;
	}
	// plus_term_rest -> epsilon
	else if(g_lookahead == ')' || g_lookahead == TOK_END || g_lookahead == ',')
	{
		return arg;
	}

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


/**
 * *,/,% terms
 * (precedence 2)
 */
static t_value mul_term()
{
	// mul_term -> pow_term mul_term_rest
	if(g_lookahead == '(' || g_lookahead == TOK_VALUE || g_lookahead == TOK_IDENT)
	{
		t_value factor_val = pow_term();
		t_value term_rest_val = mul_term_rest(factor_val);

		return term_rest_val;
	}

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


static t_value mul_term_rest(t_value arg)
{
	// mul_term_rest -> '*' pow_term mul_term_rest
	if(g_lookahead == '*')
	{
		next_lookahead();
		t_value factor_val = arg * pow_term();
		t_value term_rest_val = mul_term_rest(factor_val);

		return term_rest_val;
	}

	// mul_term_rest -> '/' pow_term mul_term_rest
	else if(g_lookahead == '/')
	{
		next_lookahead();
		t_value factor_val = arg / pow_term();
		t_value term_rest_val = mul_term_rest(factor_val);

		return term_rest_val;
	}

	// mul_term_rest -> '%' pow_term mul_term_rest
	else if(g_lookahead == '%')
	{
		next_lookahead();
		t_value factor_val = fmod(arg, pow_term());
		t_value term_rest_val = mul_term_rest(factor_val);

		return term_rest_val;
	}

	// mul_term_rest -> epsilon
	else if(g_lookahead == '+' || g_lookahead == '-' || g_lookahead == ')'
		|| g_lookahead == TOK_END || g_lookahead == ',')
	{
		return arg;
	}

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


/**
 * ^ terms
 * (precedence 3)
 */
static t_value pow_term()
{
	// pow_term -> factor pow_term_rest
	if(g_lookahead == '(' || g_lookahead == TOK_VALUE || g_lookahead == TOK_IDENT)
	{
		t_value factor_val = factor();
		t_value term_rest_val = pow_term_rest(factor_val);

		return term_rest_val;
	}

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


static t_value pow_term_rest(t_value arg)
{
	// pow_term_rest -> '^' factor pow_term_rest
	if(g_lookahead == '^')
	{
		next_lookahead();
		t_value factor_val = pow(arg, factor());
		t_value term_rest_val = pow_term_rest(factor_val);

		return term_rest_val;
	}

	// pow_term_rest -> epsilon
	else if(g_lookahead == '+' || g_lookahead == '-' || g_lookahead == ')'
		|| g_lookahead == TOK_END || g_lookahead == ','
		|| g_lookahead == '*' || g_lookahead == '/' || g_lookahead == '%')
	{
		return arg;
	}

	fprintf(stderr, "Invalid lookahead in %s: %d.\n", __func__, g_lookahead);
	return 0.;
}


/**
 * () terms, real factor or identifier
 * (highest precedence, 4)
 */
static t_value factor()
{
	// factor -> '(' plus_term ')'
	if(g_lookahead == '(')
	{
		next_lookahead();
		t_value expr_val = plus_term();
		match(')');
		next_lookahead();

		return expr_val;
	}

	// factor -> TOK_VALUE
	else if(g_lookahead == TOK_VALUE)
	{
		t_value val = g_lookahead_val;
		next_lookahead();

		return val;
	}

	// factor -> TOK_IDENT
	else if(g_lookahead == TOK_IDENT)
	{
		char ident[MAX_IDENT];
		strncpy(ident, g_lookahead_text, MAX_IDENT);

		next_lookahead();

		// function call
		// using next g_lookahead, grammar still ll(1)?
		if(g_lookahead == '(')
		{
			next_lookahead();

			// 0-argument function
			// factor -> TOK_IDENT '(' ')'
			if(g_lookahead == ')')
			{
				next_lookahead();

				// TODO
				//auto iter = m_mapFuncs0.find(ident);
				//if(iter == m_mapFuncs0.end())
				{
					fprintf(stderr, "Unknown function: \"%s\".\n", ident);
					return 0.;
				}

				//return (*iter->second)();
			}

			// function with arguments
			else
			{
				// first argument
				t_value expr_val1 = plus_term();

				// one-argument-function
				// factor -> TOK_IDENT '(' plus_term ')'
				if(g_lookahead == ')')
				{
					next_lookahead();

					if(strncmp(ident, "sin", MAX_IDENT)==0)
					{
						return sin(expr_val1);
					}
					else if(strncmp(ident, "cos", MAX_IDENT)==0)
					{
						return cos(expr_val1);
					}
					else if(strncmp(ident, "tan", MAX_IDENT)==0)
					{
						return tan(expr_val1);
					}
					else
					{
						fprintf(stderr, "Unknown function: \"%s\".\n", ident);
						return 0.;
					}
				}

				// two-argument-function
				// factor -> TOK_IDENT '(' plus_term ',' plus_term ')'
				else if(g_lookahead == ',')
				{
					next_lookahead();
					t_value expr_val2 = plus_term();
					match(')');
					next_lookahead();

					if(strncmp(ident, "atan2", MAX_IDENT)==0)
					{
						return atan2(expr_val1, expr_val2);
					}
					else
					{
						fprintf(stderr, "Unknown function: \"%s\".\n", ident);
						return 0.;
					}
				}
				else
				{
					fprintf(stderr, "Invalid function call to \"%s\".\n", ident);
				}
			}
		}

		// assignment
		else if(g_lookahead == '=')
		{
			next_lookahead();
			t_value assign_val = plus_term();
			assign_or_insert_symbol(ident, assign_val);
			return assign_val;
		}

		// variable lookup
		else
		{
			const struct Symbol* sym = find_symbol(ident);
			if(!sym)
			{
				fprintf(stderr, "Unknown identifier \"%s\".\n", ident);
				return 0.;
			}

			return sym->value;
		}
	}

	fprintf(stderr, "Invalid lookahead in %s: \"%d\".\n", __func__, g_lookahead);
	return 0.;
}



t_value parse(const char* str)
{
	set_input(str);
	next_lookahead();
	return plus_term();
}
// ------------------------------------------------------------------------



int main()
{
	init_symbols();

	t_value val1 = parse("a = 2 + 3*4");
	t_value val2 = parse("(2 + (b=3))*4 + b*2");

#ifdef USE_INTEGER
	printf("%d\n%d\n\n", val1, val2);
#else
	printf("%f\n%f\n\n", val1, val2);
#endif
	print_symbols();

	deinit_symbols();
	return 0;
}
