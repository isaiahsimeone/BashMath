#include "math_parser.h"

extern char buffer[BUFF_SZ];
extern int error_encountered;

void unknown_seq_error()
{
    printf("%s\n", buffer);
    int at_pos = peek_token().col_pos;
    P_SPACE(at_pos);
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
    printf("^ Error: Expected %s but encountered %s\n",get_token_name(expected),
           get_token_name(encountered.type));

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

void unassigned_lvalue_err(Token unassigned_lvalue)
{
    if (error_encountered == 0)
        stop_parsing();
    else
        return ;
    printf("%s\n", buffer); /* Print originally entered expression */
    P_SPACE(unassigned_lvalue.col_pos);
    printf("^ Error: Unassigned Identifier '%s'\n", unassigned_lvalue.lvalue);
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