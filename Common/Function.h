
#define ExpTyp_Function 1
#define ExpTyp_Var_X    2
#define ExpTyp_Var_Y    3
#define ExpTyp_Time     4
#define ExpTyp_Number   5

#define ExpFnc_plus     1
#define ExpFnc_minus    2
#define ExpFnc_mult     3
#define ExpFnc_div      4
#define ExpFnc_sqr      5
#define ExpFnc_sqrt     6
#define ExpFnc_sin      7
#define ExpFnc_cos      8
#define ExpFnc_exp      9
#define ExpFnc_log     10
#define ExpFnc_abs     11
#define ExpFnc_sgn     12
#define ExpFnc_atn     13

typedef struct Expr *ExprPtr;

struct Expr {
  unsigned char type;
  unsigned char func;
  double        valu;
  ExprPtr       par1;
  ExprPtr       par2;
};

extern double Var_X;
extern double Var_Y;
extern double VTime;

extern double CalcExpr(Expr *Fn);

extern ExprPtr ParseFunc(char *func);
