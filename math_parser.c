#include <stdlib.h>
#include <stdio.h>
#include <ctype.h> /* For isdigit */
#include <string.h>
#include <errno.h>
#include "math_parser.h"

/* Prints n spaces inline */
#define P_SPACE(n) for (int s = 0; s < (n - 1); s++) printf(" ");

//#define DEBUG

#ifdef DEBUG
    int parse_level = 0;
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

/* Globals */
char buffer[BUFF_SZ];
int buff_idx = 0;
int buff_sz = 0;
int current_column = 0;
char nextCh;
Token* token_stream;
int token_stream_idx;
int tokens_in_stream;
int error_encountered;

/* Gets the next token from input */
Token* next()
{
    char ch;
    Token* token = malloc(sizeof(Token));
    token->val = 0x80808080; /* Initially garbage value */
    token->col_pos = 0;  /* Default column position */

    ch = nextCh;
    nextCh = get_next_char();

    if (ch == ENDOFFILE) {
        token->type = ENDOFFILE;
        token->col_pos = current_column;
        return token;
    }

    if (isdigit(ch)) {
        token->type = NUMERIC;
        token->col_pos = current_column;
        DEBUG_PRINT("Token: %s, cp: %d\n", token_name[token->type], token->col_pos);
        token->val = get_numerical_value(ch);
        return token;
    }

    switch (ch) {
        case '/':
            token->type = DIVIDE;
            break;
        case '*':
            if (nextCh == '*') {
                nextCh = get_next_char();
                token->type = EXPONENTIATE;
            }
            else
                token->type = MULTIPLY;
            break;
        case '%':
            token->type = MODULUS;
            break;
        case '+':
            token->type = PLUS;
            break;
        case '-':
            token->type = MINUS;
            break;
        case '(':
            token->type = LPAREN;
            break;
        case ')':
            token->type = RPAREN;
            break;
        case '^':
            token->type = BIT_XOR;
            break;
        case '&':
            token->type = BIT_AND;
            break;
        case '|':
            token->type = BIT_OR;
            break;
        case '>':
            if (nextCh == '>') {
                nextCh = get_next_char();
                token->type = RSHIFT;
            }
            else
                token->type = ILLEGAL;
            break;
        case '<':
            if (nextCh == '<') {
                nextCh = get_next_char();
                token->type = LSHIFT;
            }
            else
                token->type = ILLEGAL;
            break;
        default:
            token->type = ILLEGAL;
    }
    token->col_pos = current_column;
    DEBUG_PRINT("Token: %s, cp: %d\n", token_name[token->type], token->col_pos);
    return token;
}

long get_numerical_value(char ch)
{
    char number[BUFF_SZ];
    memset(number, 0, BUFF_SZ); // needed?
    int base = 0, idx = 0;
    long long value;

    /*
     * Ignore leading "0b" for binary numbers,
     * which strtol() doesn't handle for us
     */
    if (ch == '0' && nextCh == 'b') {
        base = 2;
        ch = get_next_char();       /* Has number after 'b' */
        nextCh = get_next_char();   /* Has number 2 after 'b' */
    }

    /* first num */
    number[idx++] = ch;
    /*
     * Keep building the number until a character that
     * is not legal in a numeric or EOF is ecountered
     */
    while (nextCh != 0 && legal_numeric(nextCh) && idx < BUFF_SZ) {                 // 0 WAS EOF
        number[idx++] = nextCh;
        nextCh = get_next_char();
    }
    errno = 0;
    value = strtoll(number, NULL, base);

    /* Error checking for strtol here */
    if (value == 0 && errno != 0)
        DEBUG_PRINT("STRTOL ERROR!\n");

    return value;
}

char get_next_char()
{
    /*
     * End of file once the buffer index goes
     * over the size of the buffer
     */
    if (buff_idx == buff_sz + 1) {
        current_column++;
        return ENDOFFILE;
    }
    /* Skip over whitespace */
    while (buffer[buff_idx] == ' ') {
        current_column++;
        buff_idx++;
    }
    current_column++;
    return buffer[buff_idx++];
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

    if (buff_sz >= BUFF_SZ) {
        syntax_error(NULL, INP_TOO_LONG);   /* New type of error???????????? like constraint_error.. Something to do with inp too large */
        return ;
    }

    memcpy(buffer, expression, strlen(expression));
    //free(expression);
    /* Initialise next char */
    nextCh = get_next_char(); // at column 1 after this, 1st char in buffer too (2nd from 0)

    // free expression here??


    // construct token stream
    token_stream = malloc(sizeof(Token) * MAX_TOKENS);
    memset(token_stream, 0, sizeof(Token) * MAX_TOKENS); // needed?
    tokens_in_stream = 0;
    token_stream_idx = 0;
    Token* current_token;



    while ((current_token = next())) {
        token_stream[tokens_in_stream++] = *current_token;
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
    /*
    do {
        if (current_token->type == ILLEGAL) {
            // We stop generating the stream if there's an illegal token.. No point yaknow?
            syntax_error(current_token, BAD_SYNTAX);
            return ;
        }
        token_stream[tokens_in_stream++] = *current_token;
    } while ((current_token = next())->type != ENDOFFILE);
*/
    // Append that final EOF token
  //  token_stream[tokens_in_stream++] = *next();

    DEBUG_PRINT("=========================\n");
    DEBUG_PRINT("Token stream generated...\n");
    DEBUG_PRINT("=========================\n");

    long long answer = parse_exp();
    if (error_encountered)
        return ;
    else if (token_stream_idx != tokens_in_stream - 1)
        unknown_seq_error();
    else
        /* Print the answer */
        printf("%lld\n", answer);
}

//TODO: pass to this funcito whether it is dealing with a hex value.. Then those values are okay?
//gotta do something because currently 1f+3+ is accepted
int legal_numeric(char ch)
{
    /* Is a digit or part of a hexadecimal number */
    return (isdigit(ch) || ch == 'x' || ch == 'X'
        || ((ch - 'A') >= 0 && (ch - 'A') <= 5)     /* Allow hex characters */
        || ((ch - 'a') >= 0 && (ch - 'a') <= 5));
}


/*************** The Parser ***************/

/*
 * Python-like EBNF Math grammar
 *
 * --------- Lowest Precedence ---------
 * Exp          -> BitwiseOr EOF
 * BitwiseOr    -> BitwiseXor {OR BitwiseXor}
 * BitwiseXor   -> BitwiseAnd {XOR BitwiseAnd}
 * BitwiseAnd   -> Bitshift {AND Bitshift}
 * Bitshift     -> Arith {(LSHIFT | RSHIFT) Arith}
 * Arith        -> [PLUS | MINUS] Term {(PLUS | MINUS) Term}
 * Term         -> Exponent {(TIMES | DIVIDE | MODULUS) Exponent}
 * Exponent     -> Factor [EXPONENTIAL Factor]
 * Factor       -> LPAREN Exp RPAREN | {(PLUS | MINUS)} Numeric
 * Numeric      -> ['0x' | '0b' | '0'] NUMBER       (get_numerical_value())
 * --------- Highest Precedence ---------
 */

/*
 * Rule: Exp -> BitwiseOr EOF
 */
long long parse_exp() {
    PARSE_ENTRY("Parsing expression\n");
    long long value = parse_bitwise_or();
    PARSE_EXIT("Finished expression\n");
    return value;
}

/*
 * Rule: BitwiseOr -> BitwiseXor {OR BitwiseXor}
 */
long long parse_bitwise_or()
{
    PARSE_ENTRY("Parsing bitwise or\n");
    long long value = parse_bitwise_xor();
    while (is_match(BIT_OR)) {
        match(BIT_OR);
        value |= parse_bitwise_xor();
    }
    PARSE_EXIT("Finished bitwise or\n");
    return value;
}

/*
 * Rule: BitwiseXor -> BitwiseAnd {XOR BitwiseAnd}
 */
long long parse_bitwise_xor() {
    PARSE_ENTRY("Parsing bitwise xor\n");
    long long value = parse_bitwise_and();
    while (is_match(BIT_XOR)) {
        match(BIT_XOR);
        value ^= parse_bitwise_and();
    }
    PARSE_EXIT("Finished bitwise xor\n");
    return value;
}


/*
 * Rule: BitwiseAnd -> Bitshift {AND Bitshift}
 */
long long parse_bitwise_and() {
    PARSE_ENTRY("Parsing bitwise and\n");
    long long value = parse_bitshift();
    while (is_match(BIT_AND)) {
        match(BIT_AND);
        value &= parse_bitshift();
    }
    PARSE_EXIT("Finished bitwise and\n");
    return value;
}

/*
 * Rule: Bitshift -> Arith {(LSHIFT | RSHIFT) Arith}
 */
long long parse_bitshift() {
    PARSE_ENTRY("Parsing bit shift\n");
    long long value = parse_arith();
    while (is_match(LSHIFT) || is_match(RSHIFT)) {
        if (is_match(LSHIFT)) {
            match(LSHIFT);
            value <<= parse_arith();
        }
        else if (is_match(RSHIFT)) {
            match(RSHIFT);
            value >>= parse_arith();
        }
    }
    PARSE_EXIT("Finished bit shift\n");
    return value;
}

/*
 * Rule: Arith -> [PLUS | MINUS] Term {(PLUS | MINUS) Term}
 */
long long parse_arith() {
    PARSE_ENTRY("Parsing arith\n");
    int sign = 1;
    long long value;
    if (is_match(PLUS))
        match(PLUS); /* Value already positive */
    else if (is_match(MINUS)) {
        match(MINUS);
        sign = -1;
    }
    value = sign * parse_term();
    while (is_match(PLUS) || is_match(MINUS)) {
        if (is_match(PLUS)) {
            match(PLUS);
            value += parse_term();
        }
        else if (is_match(MINUS)) {
            match(MINUS);
            value -= parse_term();
        }
    }
    PARSE_EXIT("Finished arith\n");
    return value;
}

/*
 * Rule: Term -> Exponent {(MULTIPLY | DIVIDE | MODULUS) Exponent}
 */
long long parse_term() {
    PARSE_ENTRY("Parsing term\n");
    long long value = parse_exponent();
    while (is_match(MULTIPLY) || is_match(DIVIDE) || is_match(MODULUS)) {
        if (is_match(MULTIPLY)) {
            match(MULTIPLY);
            value *= parse_exponent();
        }
        else if (is_match(DIVIDE)) {
            match(DIVIDE);
            long long res = parse_exponent();
            if (res == 0)
                div_by_zero_error(DIVIDE);
            else
                value /= res;
        }
        else if (is_match(MODULUS)) {
            match(MODULUS);

            long long res = parse_exponent();
            if (res == 0)
                div_by_zero_error(MODULUS);
            else
                value %= res;
        }
    }
    PARSE_EXIT("Finished term\n");
    return value;
}

/*
 * Rule: Exponent -> Factor [EXPONENTIAL Factor]
 */
long long parse_exponent() {
    PARSE_ENTRY("Parsing exponent\n");
    long long value = parse_factor();
    if (is_match(EXPONENTIATE)) {
        match(EXPONENTIATE);
        //value = (long long) powl(value, parse_factor());
        value = value * parse_factor();
    }
    PARSE_EXIT("Finished exponent\n");
    return value;
}

/*
 * Rule: Factor -> LPAREN Exp RPAREN | {(MINUS | PLUS)} NUMBER
 */
long long parse_factor() {
    PARSE_ENTRY("Parsing factor\n");
    long long value = 0xBEEF; // garbage placeholder
    if (is_match(LPAREN)) {
        int paren_pos = peek_token().col_pos;
        match(LPAREN);
        value = parse_exp(); // back to the top for you
        if (is_match(RPAREN))
            match(RPAREN);
        else
            /* The opening LParen wasn't closed */
            paren_error(RPAREN, paren_pos);
    }
    /* Sequence started with an RParen */
    else if (is_match(RPAREN)) {
        int paren_pos = peek_token().col_pos;
        paren_error(LPAREN, paren_pos);
    } else {
        /* MUST be a number (optionally preceded by a sign) */
        int sign = 1;
        while (is_match(MINUS) || is_match(PLUS)) {
            if (is_match(MINUS)) {
                match(MINUS);
                sign *= -1;
            } else if (is_match(PLUS)) {
                match(PLUS);
            }
        }

        value = sign * peek_token().val; /* Cheating a bit... */
        match(NUMERIC);
    }
    PARSE_EXIT("Finished factor\n");
    return value;
}

// peeks the next token in the stream and compares to the given
int is_match(Terminal terminal)
{
    return (!error_encountered
        && token_stream[token_stream_idx].type == terminal);
}

// Advances stream index if match success
// Returns index of matched token in tok stream or 0
int match(Terminal expected)
{
    Token current_token = token_stream[token_stream_idx];

    if (current_token.type == expected) {
        LEVEL_PRINT("Matched: %s\n", token_name[expected]);
        return token_stream_idx++; /* Consider the current token matched and move on */
    }

    return match_error(current_token, expected);
}

// Peeks the next token in the stream
Token peek_token()
{
    return token_stream[token_stream_idx];
}

// Consumes the current token and returns it
Token consume_token()
{
    return token_stream[token_stream_idx++];
}


/******************************** Errors ***************************/

void unknown_seq_error()
{
    printf("%s\n", buffer);
    int at_pos = token_stream[token_stream_idx].col_pos;            // clean this up
    P_SPACE(at_pos); // why no -2 here?
    printf("^ Error: First in unknown sequence\n");
}

int syntax_error(Token* token, Error e)
{
    if (e == UNEXPECTED_EOF)
        printf("Unexpected End of file\n");
    else if (e == BAD_SYNTAX) {
        printf("%s\n", buffer);
        P_SPACE(token->col_pos);
        printf("^ Error: Syntax error\n");
    } else if (e == INP_TOO_LONG)
        printf("Error: Input too long\n");

    return e;
}

int match_error(Token encountered, Terminal expected)
{
    if (error_encountered == 0)
        stop_parsing();
    else
        return 0;

    printf("%s\n", buffer);
    P_SPACE(encountered.col_pos);
    printf("^ Error: Expected %s but encountered %s\n",token_name[expected],
           token_name[encountered.type]);

    //DEBUG_PRINT("METoken: %s, cp: %d\n", token_name[encountered.type], encountered.col_pos);

    return 0;
}

// Call for error involving 0 with modulus and division
void div_by_zero_error(Terminal offender)
{

    if (error_encountered == 0)
        stop_parsing();
    else
        return ;
    const char* operator = (offender == DIVIDE ? "Division" : "Modulus");
    printf("%s by 0 error\n", operator);
}

//Paired paren pos = position of the other paren that was not opened or closed
void paren_error(Terminal missing_paren, int paired_paren_pos)
{
    if (error_encountered == 0)
        stop_parsing();
    else
        return ;
    printf("%s\n", buffer); /* Print originally entered expression */
    P_SPACE(paired_paren_pos);
    if (missing_paren == LPAREN)
        printf("^ Error: Missing LParen '(' to open\n");
    else
        printf("^ Error: Missing RParen ')' to close\n");
}

/*
 * Any fatal error should call stop_parsing() to cause all future
 * calls to is_match() to fail. This as a result effectively stops
 * all parsing (i.e. parsing functions return ASAP without attempting
 * any further matching)
 */
void stop_parsing()
{
    error_encountered = 1;
}