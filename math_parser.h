#ifndef MATH_PARSER
#define MATH_PARSER

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h> /* For isdigit() */
#include <string.h>
#include <errno.h>

/* Maximum size of an input expression */
#define BUFF_SZ 1024
#define MAX_TOKENS 256
#define MAX_IDENT_LENGTH 50

extern int parse_level;
/* Prints n spaces inline */
#define P_SPACE(n) for (int s = 0; s < (n - 1); s++) printf(" ");

#define BASHMATH_DEBUG

#ifdef BASHMATH_DEBUG
    #define DEBUG_PRINT printf
    #define LEVEL_PRINT P_SPACE(parse_level); printf
    #define PARSE_ENTRY P_SPACE(parse_level); parse_level++; printf
    #define PARSE_EXIT parse_level--; P_SPACE(parse_level); printf
#else
    #define DEBUG_PRINT
    #define LEVEL_PRINT
    #define PARSE_ENTRY
    #define PARSE_EXIT
#endif

static const char* token_name[] =
    {
        "EOF",
        "Number",
        "Modulus",
        "Multiply",
        "Divide",
        "Exponentiate",
        "LParen",
        "RParen",
        "Plus",
        "Minus",
        "Xor",
        "Or",
        "And",
        "LShift",
        "RShift",
        "Range",
        "Sum",
        "Over",
        "In",
        "Help",
        "Identifier",
        "Assignment",
        "Illegal"
    };

typedef enum {
    ENDOFFILE = 0,
    NUMERIC = 1,
    MODULUS = 2,
    MULTIPLY = 3,
    DIVIDE = 4,
    EXPONENTIATE = 5,
    LPAREN = 6,
    RPAREN = 7,
    PLUS = 8,
    MINUS = 9,
    BIT_XOR = 10,
    BIT_OR = 11,
    BIT_AND = 12,
    LSHIFT = 13,
    RSHIFT = 14,
    RANGE = 15,
    KW_SUM = 16,
    KW_OVER = 17,
    KW_IN = 18,
    KW_HELP = 19,
    IDENTIFIER = 20,
    ASSIGN = 21,
    ILLEGAL = 22
} Terminal;

typedef enum {
    BAD_SYNTAX = 1,
    UNEXPECTED_EOF = 2,
    INP_TOO_LONG = 3
} Error;

typedef struct {
    Terminal type;
    long long val;
    char* lvalue;
    int lvalue_is_assigned;
    int col_pos;
} Token;

/* Prototypes */
char get_next_char(void);
char get_next_char_report_whitespace(int*);
Token* next(void);
void handle_expression(char*);
long get_numerical_value(char);
char* get_identifier_token(char);
int legal_numeric(char, int);
void add_identifier(Token*);
Token* get_identifier(char*);
int is_match(Terminal);
int match(Terminal);

Token peek_token(void);
Token peek_next_token(void);
Token peek_last_token(void);
Token consume_next_token(void);
const char* get_token_name(Terminal);

/* Error functions */
int match_error(Token, Terminal);
int syntax_error(Token*, Error);
void div_by_zero_error(Terminal);
void unknown_seq_error(void);
void unassigned_lvalue_err(Token);
void paren_error(Terminal, int);
void stop_parsing(void);
void display_help(void);

/* Parsing functions */
long long parse_block(void);
long long parse_assignment(void);
long long parse_summation(void);
void      parse_subrange(int*, int*);
long long parse_exp(void);
long long parse_bitwise_or(void);
long long parse_bitwise_xor(void);
long long parse_bitwise_and(void);
long long parse_bitshift(void);
long long parse_arith(void);
long long parse_term(void);
long long parse_exponent(void);
long long parse_factor(void);
Token*    parse_get_lvalue(void);

#endif /* MATH_PARSER */