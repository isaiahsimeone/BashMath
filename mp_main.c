#include "math_parser.h"

/* A 2D array of current identifier tokens */
Token** identifiers;
/* An array of Tokens comprising the input expression */
Token* token_stream;
/* The input string constituting the input expression */
char buffer[BUFF_SZ];
/* Used to index the above buffer */
int buff_idx = 0;
/* The size of the above buffer */
int buff_sz = 0;
/* Tracks the position of symbols within the expression buffer */
int current_column = 0;
/* The next character in the buffer */
char nextCh;
/* Used to index the stream of tokens (token_stream) */
int token_stream_idx;
/* The number of initialised identifier tokens */
int identifier_count;
/* The number of tokens in the token_stream */
int tokens_in_stream;
/* 1 if an error is encountered, 0 otherwise */
int error_encountered;
/* Used to track the commencement of a session in BASH */
int session_started = 0;
/* Tracks how many functions deep parsing is */
int parse_level;

/*
 * Returns a string comprising of the LValue of an identifier token
 * 
 *      ch: the character that commences an identifier token
 *
 * returns: A string comprising the dentifiers name
 */
char* get_identifier_token(char ch)
{
    char* identifier = malloc(MAX_IDENT_LENGTH * sizeof(char));
    memset(identifier, 0, MAX_IDENT_LENGTH * sizeof(char));

    int idx = 0;
    int encountered_whitespace = 0;

    while (nextCh != 0 && isalpha(nextCh) && !encountered_whitespace
            && idx < MAX_IDENT_LENGTH) {
        identifier[idx++] = nextCh;
        nextCh = get_next_char_report_whitespace(&encountered_whitespace);
    }
    identifier[idx++] = '\0';

    return identifier;
}

/*
 * The entry point for mathematical expression evaluation. scans, lexes and
 * parses the input expression before evaluation the result
 *
 * expression: a string comprising of the input expression to be processed
 */
void handle_expression(char* expression)
{
    /* Reset globals */
    memset(buffer, 0, BUFF_SZ);
    buff_idx = 0;
    buff_sz = strlen(expression);
    /* 
     * One less due to initial call to get_next_char() 
     * below, which modifies this variable 
     */
    current_column = -1;
    error_encountered = 0;
    parse_level = 0;

    if (buff_sz >= BUFF_SZ) {
        syntax_error(NULL, INP_TOO_LONG);
        return ;
    }

    memcpy(buffer, expression, strlen(expression));
    free(expression);
    /* Initialise next char */
    nextCh = get_next_char();

    /* Constructs the token stream */
    token_stream = malloc(sizeof(Token) * MAX_TOKENS);
    memset(token_stream, 0, sizeof(Token) * MAX_TOKENS);
    if (!session_started) {
        identifiers = malloc(sizeof(Token) * MAX_TOKENS);
        memset(identifiers, 0, sizeof(Token) * MAX_TOKENS);
        identifier_count = 0;
        session_started = 1;
    }

    tokens_in_stream = 0;
    token_stream_idx = 0;
    Token* current_token;

    while ((current_token = next())) {
        token_stream[tokens_in_stream++] = *current_token;
        /* Is this token also an identifier token? */
        if (current_token->type == IDENTIFIER)
            add_identifier(current_token);

        if (current_token->type == ENDOFFILE)
            /* The first and only token was EOF */
            if (tokens_in_stream == 1) {
                syntax_error(current_token, UNEXPECTED_EOF);
                return ;
            } else
                break;

        if (current_token->type == ILLEGAL) {
            syntax_error(current_token, BAD_SYNTAX);
            return;
        }
    }
    
    DEBUG_PRINT("=========================\n");
    DEBUG_PRINT("Token stream generated...\n");
    DEBUG_PRINT("=========================\n");

    if (is_match(KW_HELP)) {
        match(KW_HELP);
        display_help();
        return ;
    }

    long long answer = parse_block();
    DEBUG_PRINT("There are %d identifiers\n", identifier_count);

    if (error_encountered)
        return ;
    else if (token_stream_idx != tokens_in_stream - 1)
        unknown_seq_error();
    else
        /* Print the answer */
        printf("%lld\n", answer);
}

/*
 * Adds an identifier token to the array of identifiers. Ready for use
 * in expressions.
 *
 * token: The token that was identified as an identifier which should be
 *        added to the list of identifiers
 */
void add_identifier(Token* token)
{
    for (int i = 0; i < identifier_count; i++) {
        if (strcmp(identifiers[i]->lvalue, token->lvalue) == 0) {
            /* Identifier already in use - update col pos and return*/
            identifiers[i]->col_pos = token->col_pos;
            return ; 
        }
    }
    token->val = 0x80808080; /* Garage placeholder value */
    token->lvalue_is_assigned = 0;
    identifiers[identifier_count++] = token;
}

/*
 * Returns the identifier token associated with the specified readable
 * LValue name.
 *
 *  lvalue: A string comprising the readable name of the target identifier
 *
 * returns: The target identifier token, or NULL if there is no such identifier
 *          token with the specified LValue name.
 */
Token* get_identifier(char* lvalue)
{
    for (int i = 0; i < identifier_count; i++) {
        if (strcmp(identifiers[i]->lvalue, lvalue) == 0)
           return identifiers[i];
    }
    return NULL;
}

/*
 * Prints a help dialog to stdout with instructions on how to use BashMath
 */
void display_help() 
{
    printf("Expressions can be input by starting a command with \neither: a number"
        "or with the character '='\n\n");
    printf("For example:\n$ =1+1\n$ 2\n\n");
    printf("* Python-like EBNF Math grammar\n"
     "*\n"
     "* --------- Lowest Precedence ---------\n"
     "* Block        -> Assignment | Summation | Exp\n"
     "* Assignment   -> LValue ASSIGN Exp\n"
     "* Summation    -> KW_SUM LValue KW_OVER Subrange KW_IN Exp\n"
     "* Subrange     -> Exp RANGE Exp\n"
     "* Exp          -> BitwiseOr EOF\n"
     "* BitwiseOr    -> BitwiseXor {OR BitwiseXor}\n"
     "* BitwiseXor   -> BitwiseAnd {XOR BitwiseAnd}\n"
     "* BitwiseAnd   -> Bitshift {AND Bitshift}\n"
     "* Bitshift     -> Arith {(LSHIFT | RSHIFT) Arith}\n"
     "* Arith        -> [PLUS | MINUS] Term {(PLUS | MINUS) Term}\n"
     "* Term         -> Exponent {(TIMES | DIVIDE | MODULUS) Exponent}\n"
     "* Exponent     -> Factor [EXPONENTIAL Factor]\n"
     "* Factor       -> LPAREN Exp RPAREN | {(PLUS | MINUS)} Numeric | LValue\n"
     "* Numeric      -> ['0x' | '0b' | '0'] NUMBER\n"
     "* LValue       -> IDENTIFIER\n"
     "* --------- Highest Precedence ---------\n");
}