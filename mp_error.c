#include "math_parser.h"

/* The buffer containing the input expression */
extern char buffer[BUFF_SZ];
/* 1 if an error has been encountered, 0 otherwise */
extern int error_encountered;

/*
 * Prints an error message to stderr stating that a symbol was encountered
 * in the input expression that was not part of any known sequence.
 */
void unknown_seq_error()
{
    fprintf(stderr, "%s\n", buffer);
    int at_pos = peek_token().col_pos;
    P_SPACE(stderr, at_pos);
    fprintf(stderr, "^ Error: First in unknown sequence\n");
}

/*
 * Prints a message to stderr depending on the kind of error passed to
 * this function. E.g. UNEXPECTED_EOF, BAD_SYNTAX, etc.
 *
 * token: In case of a BAD_SYNTAX error, the token corresponding to where
 *        this error took place in the input expression.
 *     e: A member of the Error enum corresponding to which error was
 *        encountered.
 */
void syntax_error(Token* token, Error e)
{
    if (e == UNEXPECTED_EOF)
        fprintf(stderr, "Unexpected End of file\n");
    else if (e == BAD_SYNTAX) {
        fprintf(stderr, "%s\n", buffer);
        P_SPACE(stderr, token->col_pos);
        fprintf(stderr, "^ Error: Syntax error\n");
    } else if (e == INP_TOO_LONG)
        fprintf(stderr, "Error: Input too long\n");
}

/*
 * Displays an error to stderr where an encountered Terminal symbol did
 * not match the expected terminal symbol.
 *
 * encountered: The terminal symbol that was erroneously encountered
 *    expected: The terminal symbol that was expected to be encountered
 *
 *     returns: MATCH_ERR, a member of the Error enum
 */
int match_error(Token encountered, Terminal expected)
{
    if (!error_encountered)
        stop_parsing();
    else
        return MATCH_ERR;

    fprintf(stderr, "%s\n", buffer);
    P_SPACE(stderr, encountered.col_pos);
    fprintf(stderr, "^ Error: Expected %s but encountered %s\n",
        get_token_name(expected), get_token_name(encountered.type));

    return MATCH_ERR;
}

/*
 * Displays an error to stderr. This error states that there was an attempt
 * to divide by zero in either a division or modulus operation.
 *
 * offender: A Terminal, either DIVIDE or MODULUS that was involved in an
 *           illegal operation with zero.
 */
void div_by_zero_error(Terminal offender)
{
    if (!error_encountered)
        stop_parsing();
    else
        return ;
    const char* operator = (offender == DIVIDE ? "Division" : "Modulus");
    fprintf(stderr, "%s by 0 error\n", operator);
}

/*
 * Prints an error to stderr stating that either a left/right parenthesis
 * was unmatched. 
 *
 * missing_paren:    The type of missing parentheses, either LPAREN, or RPAREN
 * paired_paren_pos: The position of the parenthesis that was unmatched 
 */
void paren_error(Terminal missing_paren, int paired_paren_pos)
{
    if (!error_encountered)
        stop_parsing();
    else
        return ;
    fprintf(stderr, "%s\n", buffer); /* Print originally entered expression */
    P_SPACE(stderr, paired_paren_pos);
    if (missing_paren == LPAREN)
        fprintf(stderr, "^ Error: Missing LParen '(' to open\n");
    else
        fprintf(stderr, "^ Error: Missing RParen ')' to close\n");
}

/*
 * Prints an error to stderr stating that an identifier/LValue has no
 * assigned value, and hence cannot be used in an expression.
 *
 * unassigned_lvalue: The Token which was entered in an expression, that
 *                    has no value assigned.
 */
void unassigned_lvalue_err(Token unassigned_lvalue)
{
    if (!error_encountered)
        stop_parsing();
    else
        return ;
    fprintf(stderr, "%s\n", buffer); /* Print originally entered expression */
    P_SPACE(stderr, unassigned_lvalue.col_pos);
    fprintf(stderr, "^ Error: Unassigned Identifier '%s'\n",
        unassigned_lvalue.lvalue);
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