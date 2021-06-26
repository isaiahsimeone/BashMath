#include "math_parser.h"

/* Globals */
Token** identifiers;
Token* token_stream;
char buffer[BUFF_SZ];
int buff_idx = 0;
int buff_sz = 0;
int current_column = 0;
char nextCh;
int token_stream_idx;
int identifier_count;
int tokens_in_stream;
int error_encountered;
int session_started = 0;
int parse_level;

char* get_identifier_token(char ch)
{
    char* identifier = malloc(MAX_IDENT_LENGTH * sizeof(char));
    memset(identifier, 0, MAX_IDENT_LENGTH * sizeof(char));

    int idx = 0;
    int encountered_whitespace = 0;

////////////// DON'T NEED ISALPHA() CHECK HERE. WHITESPACE IS ENOUGH since a variable like 'bar1' is fine
    while (nextCh != 0 && isalpha(nextCh) && !encountered_whitespace && idx < MAX_IDENT_LENGTH) {
        identifier[idx++] = nextCh;
        nextCh = get_next_char_report_whitespace(&encountered_whitespace);
    }
    identifier[idx++] = '\0';

    return identifier;
}


/* Math evaulation entry point */
void handle_expression(char* expression)
{
    /* Reset globals */
    memset(buffer, 0, BUFF_SZ);
    buff_idx = 0;
    buff_sz = strlen(expression);
    /* One less due to initial call to get_next_char() below, which modifies this variable */
    current_column = -1;
    error_encountered = 0;
    parse_level = 0;

    if (buff_sz >= BUFF_SZ) {
        syntax_error(NULL, INP_TOO_LONG);   /* New type of error???????????? like constraint_error.. Something to do with inp too large */
        return ;
    }

    memcpy(buffer, expression, strlen(expression));
    free(expression);
    /* Initialise next char */
    nextCh = get_next_char(); // at column 1 after this, 1st char in buffer too (2nd from 0)


    // construct token stream
    token_stream = malloc(sizeof(Token) * MAX_TOKENS);
    memset(token_stream, 0, sizeof(Token) * MAX_TOKENS); // needed?
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

Token* get_identifier(char* lvalue)
{
    for (int i = 0; i < identifier_count; i++) {
        if (strcmp(identifiers[i]->lvalue, lvalue) == 0)
           return identifiers[i];
    }
    return NULL;
}

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