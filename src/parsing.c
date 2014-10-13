#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <histedit.h>
#include <math.h>

#include "mpc.h"

typedef struct {
    int type;
    long num;
    double decimal;
    int err;
} lval;

enum { LVAL_ERR, LVAL_NUM, LVAL_DEC };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval eval_op(lval x, char* op, lval y);
lval eval(mpc_ast_t* t);
int min(int x, int y);
int max(int x, int y);
void lval_print(lval v);
void lval_println(lval v);

lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_dec(double x)
{
    lval v;
    v.type = LVAL_DEC;
    v.decimal = x;
    return v;
}

lval lval_err(int x)
{
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v)
{
    switch (v.type) {
        case LVAL_NUM:
            printf("%li", v.num);
            break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) { printf("Error: Division by Zero!"); }
            if (v.err == LERR_BAD_OP) { printf("Error: Invalid operator!"); }
            if (v.err == LERR_BAD_NUM) { printf("Error: Invalid Number! "); }
            break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval(mpc_ast_t* t)
{
    if (strstr(t->tag, "number")) { 
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }
    if (strstr(t->tag, "decimal")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_dec(x) : lval_err(LERR_BAD_NUM);
    }

    char* op = t->children[1]->contents;
    lval x = eval(t->children[2]);

    if (strstr(t->children[3]->tag, "expr")) {
        int i = 3;

        while (strstr(t->children[i]->tag, "expr")) {
            x = eval_op(x, op, eval(t->children[i]));
            i++;
        }
    } else if (strcmp(op, "-") == 0) {
        x.num = -x.num;
    }

    return x;
}

int min(int x, int y)
{
    if (x < y) { return x; }
    return y;
}

int max(int x, int y)
{
    if (x > y) { return x; }
    return y;
}

lval eval_op(lval x, char* op, lval y)
{
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
    if (strcmp(op, "/") == 0) { 
        return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    }
    if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
    if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
    if (strcmp(op, "min") == 0) { return lval_num(min(x.num, y.num)); }
    if (strcmp(op, "max") == 0) { return lval_num(max(x.num, y.num)); }

    return lval_err(LERR_BAD_OP);
}

int main(int argc, char** argv)
{
    /* Create Some Parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Decimal  = mpc_new("decimal");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                                     \
        number   : /-?[0-9]+/ ;                                             \
        decimal  : /-?[0-9]+\\.[0-9]+/;                                     \
        operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\";   \
        expr     : <decimal> | <number> | '(' <operator> <expr>+ ')' ;      \
        lispy    : /^/ <operator> <expr>+ /$/ ;                             \
      ",
      Decimal, Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char* input = readline("lispy> ") ;

        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    mpc_cleanup(4, Decimal, Number, Operator, Expr, Lispy);
    return 0;
}
