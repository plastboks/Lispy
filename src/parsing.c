#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>
#include <math.h>

#include "mpc.h"

static char input[2048];

long eval_op(long x, char* op, long y);
long eval(mpc_ast_t* t);

long eval(mpc_ast_t* t)
{
    if (strstr(t->tag, "number")) { return atoi(t->contents); }

    char* op = t->children[1]->contents;

    long x = eval(t->children[2]);

    int i = 3;

    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int min(int x, int y)
{
    if (x < y)
        return x;
    else
        return y;
}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

long eval_op(long x, char* op, long y)
{
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    if (strcmp(op, "%") == 0) { return x % y; }
    if (strcmp(op, "^") == 0) { return pow(x, y); }
    if (strcmp(op, "min") == 0) { return min(x, y); }
    if (strcmp(op, "max") == 0) { return max(x, y); }
    return 0;
}

int main(int argc, char** argv)
{
    /* Create Some Parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                                     \
        number   : /-?[0-9]+/ ;                                             \
        operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\";   \
        expr     : <number> | '(' <operator> <expr>+ ')' ;                  \
        lispy    : /^/ <operator> <expr>+ /$/ ;                             \
      ",
      Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char* input = readline("lispy> ") ;

        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
