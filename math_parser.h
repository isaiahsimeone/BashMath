#ifndef MATH_PARSER
#define MATH_PARSER

/* Token types */
#define NUMBER_TOKEN 1
#define LEXICAL_TOKEN 2

/* Maximum size of an input expression */
#define BUFF_SZ 1024
#define MAX_TOKENS 256

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
    ILLEGAL = 15
} Terminal;

typedef enum {
    BAD_SYNTAX = 1,
    UNEXPECTED_EOF = 2,
    INP_TOO_LONG = 3
} Error;

typedef struct {
    Terminal type;
    long long val;
    int col_pos;
} Token;

/* Prototypes */
char get_next_char(void);
Token* next(void);
void handle_expression(char*);
long get_numerical_value(char);
int legal_numeric(char);

int is_match(Terminal);
int match(Terminal);

Token peek_token(void);
Token consume_next_token(void);

/* Error functions */
int match_error(Token, Terminal);
int syntax_error(Token*, Error);
void div_by_zero_error(Terminal);
void unknown_seq_error(void);
void paren_error(Terminal, int);
void stop_parsing(void);

/* Parsing functions */
long long parse_exp(void);
long long parse_bitwise_or(void);
long long parse_bitwise_xor(void);
long long parse_bitwise_and(void);
long long parse_bitshift(void);
long long parse_arith(void);
long long parse_term(void);
long long parse_exponent(void);
long long parse_factor(void);

#endif /* MATH_PARSER */