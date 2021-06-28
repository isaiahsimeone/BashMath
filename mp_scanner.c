#include "math_parser.h"

extern char buffer[BUFF_SZ];
extern int buff_idx;
extern int buff_sz;
extern int current_column;
extern char nextCh;

/*
 * Returns the next tokenised item from the input expression.
 *
 * returns: The next token identified from the input expression
 *          buffer.
 */
Token* next()
{
    char ch;
    Token* token = malloc(sizeof(Token));
    token->val = 0x80808080; /* Initially garbage value */
    token->col_pos = 0;  /* Default column position */

    ch = nextCh;
    
    if (ch == ENDOFFILE) {
        token->type = ENDOFFILE;
        token->col_pos = current_column + 1;
        return token;
    }

    if (isalpha(ch)) {
        token->type = IDENTIFIER;
        token->col_pos = current_column + 1;
        
        token->val = 0x80808080; /* Garbage place holder */
        token->lvalue = get_identifier_token(ch);
        /* Extended keywords */
        if (strcmp(token->lvalue, "sum") == 0)
            token->type = KW_SUM;

        if (strcmp(token->lvalue, "over") == 0)
            token->type = KW_OVER;

        if (strcmp(token->lvalue, "in") == 0)
            token->type = KW_IN;

        if (strcmp(token->lvalue, "help") == 0)
            token->type = KW_HELP;
        DEBUG_PRINT("Token: %s, cp: %d, lv: %s\n", 
            get_token_name(token->type), token->col_pos, token->lvalue);
        return token;
    }

    if (isdigit(ch)) {
        token->type = NUMERIC;
        token->col_pos = current_column + 1;
        DEBUG_PRINT("Token: %s, cp: %d\n", get_token_name(token->type),
            token->col_pos);
        token->val = get_numerical_value(ch);
        return token;
    }

    nextCh = get_next_char();

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
        case '=':
            token->type = ASSIGN;
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
        case '.':
            if (nextCh == '.') {
                nextCh = get_next_char();
                if (nextCh == '.') {
                    nextCh = get_next_char();
                    token->type = RANGE;
                } else
                    token->type = ILLEGAL;
            } else {
                token->type = ILLEGAL;
            }
            break;
        default:
            token->type = ILLEGAL;
    }
    token->col_pos = current_column;
    DEBUG_PRINT("Token: %s, cp: %d\n", 
        get_token_name(token->type), token->col_pos);
    return token;
}

/*
 * Determines whether a provided number is a valid numeric for the specified
 * number base. (e.g. alphabetical numbers are only appropriate in hexadecimal)
 * 
 *      ch: A character that is tested to be belonging to the specific base
 *    base: the base to test the character against
 *
 * returns: 1 if the character is legal for the specific base, 0 otherwise.
 */
int legal_numeric(char ch, int base)
{
    if (base == 16) {
        /* Is a digit or part of a hexadecimal number */
        return (isdigit(ch) || ((ch - 'A') >= 0 && (ch - 'A') <= 5)
            || ((ch - 'a') >= 0 && (ch - 'a') <= 5));
    }
    return isdigit(ch);
 
}

/*
 * Returns the next character in the input buffer sequence of characters.
 *
 *  returns: The next non-whitespace character in the input buffer
 */
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

    /* Skip over whitespace, if there is any */
    while (buffer[buff_idx] == ' ') {
        current_column++;
        buff_idx++;
    }

    current_column++;
    return buffer[buff_idx++];
}

/*
 * Returns the next character in the input buffer sequence of characters.
 * If any whitespace is encountered upon advancing to the next non-whitespace
 * character, this function reports it via the skipped_whitespace argument.
 *
 * skipped_whitespace: set to 1 if whitespace is encountered upon advancing to
 *                     the next non-whitespace character, 0 otherwise.
 *
 *  returns: The next non-whitespace character in the input buffer
 */
char get_next_char_report_whitespace(int* skipped_whitespace)
{
    *skipped_whitespace = 0;
    /*
     * End of file once the buffer index goes
     * over the size of the buffer
     */
    if (buff_idx == buff_sz + 1) {
        current_column++;
        return ENDOFFILE;
    }

    /* Skip over whitespace, if there is any */
    while (buffer[buff_idx] == ' ') {
        *skipped_whitespace = 1;
        current_column++;
        buff_idx++;
    }

    current_column++;
    return buffer[buff_idx++];
}

/*
 * Returns the numerical value of a number token from any base,
 * to decimal.
 *
 *       ch: The character that identifies the start of a numerical input
 *
 *  returns: The value of the numerical expression in decimal.
 */
long long get_numerical_value(char ch)
{
    char number[BUFF_SZ];
    memset(number, 0, BUFF_SZ);
    int base = 0, idx = 0;
    long long value;

    /*
     * Ignore leading "0b" for binary numbers,
     * which strtol() doesn't handle for us
     */
    nextCh = get_next_char();
    if (ch == '0' && nextCh == 'b') {
        base = 2;
        ch = get_next_char();       /* Has number after 'b' */
        nextCh = get_next_char();   /* Has number 2 after 'b' */
    }

    if (ch == '0' && (nextCh == 'x' || nextCh == 'X')) {
        base = 16;
        ch = get_next_char();
        nextCh = get_next_char();
    }

    /* first num */
    number[idx++] = ch;
    /*
     * Keep building the number until a character that
     * is not legal in a numeric or EOF is ecountered
     */
    while (nextCh != 0 && legal_numeric(nextCh, base) && idx < BUFF_SZ) {
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

/*
 * Returns a string comprising of a terminal symbols readable name
 * e.g. get_token_name(LPAREN) -> "("
 *
 * terminal: The terminal symbol to lookup the readable name of
 *
 *  returns: A string comprising of the specified terminal symbols
 *           readable name.
 */
const char* get_token_name(Terminal terminal)
{
    return token_name[terminal];
}