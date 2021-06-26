#include "math_parser.h"

extern int token_stream_idx;
extern int error_encountered;
extern Token* token_stream;

/*
 * Python-like EBNF Math grammar
 *
 * --------- Lowest Precedence ---------
 * Block        -> Assignment | Summation | Exp
 * Assignment   -> LValue ASSIGN Exp
 * Summation    -> KW_SUM LValue KW_OVER Subrange KW_IN Exp
 * Subrange     -> Exp RANGE Exp
 * Exp          -> BitwiseOr EOF
 * BitwiseOr    -> BitwiseXor {OR BitwiseXor}
 * BitwiseXor   -> BitwiseAnd {XOR BitwiseAnd}
 * BitwiseAnd   -> Bitshift {AND Bitshift}
 * Bitshift     -> Arith {(LSHIFT | RSHIFT) Arith}
 * Arith        -> [PLUS | MINUS] Term {(PLUS | MINUS) Term}
 * Term         -> Exponent {(TIMES | DIVIDE | MODULUS) Exponent}
 * Exponent     -> Factor [EXPONENTIAL Factor]
 * Factor       -> LPAREN Exp RPAREN | {(PLUS | MINUS)} Numeric | LValue
 * Numeric      -> ['0x' | '0b' | '0'] NUMBER       (get_numerical_value())
 * LValue       -> IDENTIFIER
 * --------- Highest Precedence ---------
 */

/*
 * Rule: Block -> Assignment | Summation | Exp
 */
long long parse_block() {
    PARSE_ENTRY("Parsing Block\n");
    long long value;
    if (is_match(KW_SUM))
        value = parse_summation();
    else if (is_match(IDENTIFIER) && peek_next_token().type == ASSIGN)
        value = parse_assignment();
    else
        value = parse_exp();

    PARSE_ENTRY("Finished Block\n");
    return value;
}

/*
 * Rule: Assignment -> LValue ASSIGN Exp
 */
long long parse_assignment()
{
    PARSE_ENTRY("Parsing assignment\n");
    Token* target = parse_get_lvalue();
    target->lvalue_is_assigned = 1;
    match(ASSIGN);
    long long assigned_val = parse_exp();
    target->val = assigned_val;
    PARSE_EXIT("Finished assignment\n");
    return assigned_val;
}

/*
 * Rule: Summation -> KW_SUM LValue KW_OVER Subrange KW_IN Exp
 */
long long parse_summation()
{
    PARSE_ENTRY("Parsing summation\n");
    match(KW_SUM);
    Token* target = parse_get_lvalue();
    target->lvalue_is_assigned = 1;
    long long acculumlator = 0;
    
    match(KW_OVER);
    // parse subrange here
    int lower_bound, upper_bound;
    parse_subrange(&lower_bound, &upper_bound);
    match(KW_IN);
    
    // Count how many tokens were analysed in parsing of the expression to be summed
    int parse_count_0 = token_stream_idx;
    target->val = lower_bound;
    acculumlator += parse_exp();
    int parse_count_1 = token_stream_idx;
    int tokens_in_exp = parse_count_1 - parse_count_0;
    token_stream_idx -= tokens_in_exp;

    for (int i = lower_bound + 1; i <= upper_bound && !error_encountered; i++) {
        target->val = i;
        acculumlator += parse_exp();
        token_stream_idx -= tokens_in_exp;
    }
    token_stream_idx += tokens_in_exp;

    PARSE_EXIT("Finished summation\n");
    return acculumlator;
}

/*
 * Rule: Subrange -> Exp RANGE Exp
 */
void parse_subrange(int* lower_bound, int* upper_bound)
{
    PARSE_ENTRY("Parsing subrange\n");
    *lower_bound = parse_exp();
    match(RANGE);
    *upper_bound = parse_exp();
    PARSE_EXIT("Finished subrange\n");
}

/*
 * Rule: Exp -> BitwiseOr EOF
 */
long long parse_exp()
{
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
 * Rule: Factor -> LPAREN Exp RPAREN | {(MINUS | PLUS)} NUMBER | LValue
 */
long long parse_factor()
{
    PARSE_ENTRY("Parsing factor\n");
    long long value = 0xBEEF; // garbage placeholder
    if (is_match(LPAREN)) {
        int paren_pos = peek_token().col_pos;
        match(LPAREN);
        value = parse_exp();
        if (is_match(RPAREN))
            match(RPAREN);
        else
            /* The opening LParen wasn't closed */
            paren_error(RPAREN, paren_pos);
    }
    /* Sequence started with an RParen */
    else if (is_match(RPAREN)) {
        /* If the token directly before this RParen is an LParen, then it's just empty parentheses */
        if (peek_last_token().type == LPAREN && token_stream_idx > 0)
            return 0;
        int paren_pos = peek_token().col_pos;
        paren_error(LPAREN, paren_pos);
    } else if (is_match(IDENTIFIER)) {
        Token* parsed_lvalue = parse_get_lvalue();
        if (!parsed_lvalue->lvalue_is_assigned)
            unassigned_lvalue_err(*parsed_lvalue);
        else
            value = parsed_lvalue->val;
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

/*
 * Rule: LValue -> IDENTIFIER
 */
Token* parse_get_lvalue()
{
    PARSE_ENTRY("Parsing LValue\n");

    char* target_lvalue = malloc(MAX_IDENT_LENGTH * sizeof(char));
    memset(target_lvalue, 0, sizeof(char)* MAX_IDENT_LENGTH);
    //target_lvalue = peek_token().lvalue; /* Get the readable name of the current lvalue */
    strcpy(target_lvalue, peek_token().lvalue);
    match(IDENTIFIER);
    PARSE_EXIT("Finished LValue\n");
    return get_identifier(target_lvalue);
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
    Token current_token = peek_token();

    if (current_token.type == expected) {
        LEVEL_PRINT("Matched: %s\n", get_token_name(expected));
        return token_stream_idx++; /* Consider the current token matched and move on */
    }

    return match_error(current_token, expected);
}

// returns the next token in the stream
Token peek_token()
{
    return token_stream[token_stream_idx];
}

Token peek_next_token()
{
    return token_stream[token_stream_idx + 1];
}

Token peek_last_token()
{
    return token_stream[token_stream_idx - 1];
}

// Consumes the current token and returns it
Token consume_token()
{
    return token_stream[token_stream_idx++];
}