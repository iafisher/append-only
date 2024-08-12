// This file implements a simple Lisp interpreter. It is written using a technique
// called append-only programming: the programmer is permitted to append code to the
// file, but not to edit code once it has been appended.

// First, we know that we must have an evaluator which turns an abstract syntax tree
// into a result. For our simple Lisp dialect, assume that all results are integers.
// We do not yet know what the abstract syntax tree will look like, so leave it
// declared but undefined.

struct Tree;

int eval(struct Tree);

// Let's write a few test cases.

// First, we'll need an assertion function.

#include <stdio.h>

void assert_int_eq(int actual, int expected) {
  if (actual != expected) {
    printf("assertion failure: expected %d, got %d\n", expected, actual);
  }
}

// We can't yet call `eval` because we don't have a way of constructing a parse tree.
// Instead, let's define a wrapper function that takes a string.

int eval_string(const char*);

// Now we can write some basic eval tests.

void test_eval1() {
  assert_int_eq(eval_string("(+ 1 2)"), 3);
}

void test_eval2() {
  assert_int_eq(
    eval_string(
      "(* (- 7 4) (+ (/ 26 2) 1))"
    ),
    42
  );
}

// Now that we know what the final API is going to look like, let's start building
// from the ground up.

// We know that the first step is going to be to tokenize the input string into a
// stream of tokens.

struct Tokenizer;
struct Token;

// Tokens have a type, plus a slice (pointer + length) of the original strings.

struct Token {
  int t;
  const char* s;
  size_t n;
};

// Before we define the Tokenizer data structure, let's work out what the public
// API will be.

// Clearly we'll need a way to initialize the tokenizer from the input string.
struct Tokenizer tokenizer_init(const char* p);

// Also, retrieve the current token, and advance to the next token.
struct Token tokenizer_current(struct Tokenizer*);
void tokenizer_advance(struct Tokenizer*);

// And a way to tell if the tokenizer has reached the end of the string.
int tokenizer_done(struct Tokenizer*);

// Now we can define the Tokenizer data structure.
struct Tokenizer {
  const char* p;
  // index of next character to read
  size_t i;
  // length of p
  size_t n;
  // current token
  struct Token t;
};

// Defining tokenizer_init is straightforward

#include <string.h>

struct Tokenizer tokenizer_init(const char* p) {
  struct Tokenizer tz;
  tz.p = p;
  tz.i = 0;
  tz.n = strlen(p);
  // leave t undefined; users must call tokenizer_advance() first
  return tz;
}

// tokenizer_done is also easy

int tokenizer_done(struct Tokenizer* tz) {
  return tz->i >= tz->n;
}

// tokenizer_current just returns the `t` field
struct Token tokenizer_current(struct Tokenizer* tz) {
  return tz->t;
}

// Before defining tokenizer_advance, let's start writing the parser to get an
// idea of how it will be used.

// The parser should take in a tokenizer and return a tree.
// I don't know yet whether the parser will need to be stateful, so let's
// declare a Parser type to be safe.
struct Parser;

struct Parser parser_init(struct Tokenizer* tz);
struct Tree parser_parse(struct Parser*);


// Take the opportunity now to define our token types.

int TOKEN_LPAREN = 1;
int TOKEN_RPAREN = 2;
int TOKEN_NUM = 3;
int TOKEN_PLUS = 4;
int TOKEN_MINUS = 5;
int TOKEN_MUL = 6;
int TOKEN_DIV = 7;
int TOKEN_EOF = 8;
int TOKEN_UNKNOWN = 9;

// Some tree initialization functions
struct Tree* binary_node(char op, struct Tree* left, struct Tree* right);
struct Tree* leaf_node(int);

// Simple Tree class
struct Tree {
  // if not NULL, this is a branching node
  struct Tree* left;
  struct Tree* right;
  int value;
  char op;
};

// Define our init functions
#include <stdlib.h>

struct Tree* binary_node(char op, struct Tree* left, struct Tree* right) {
  struct Tree* r = malloc(sizeof *r);
  r->left = left;
  r->right = right;
  r->op = op;
  return r;
}

struct Tree* leaf_node(int x) {
  struct Tree* r = malloc(sizeof *r);
  r->value = x;
  r->left = NULL;
  r->right = NULL;
  return r;
}

// We're definitely going to need a recursive function to parse expressions.

struct Tree* match_expression(struct Parser*);

// Also, since trees are going to be heap-allocated, we should change the
// signature of parser_parse to return a pointer.

struct Tree* parser_parse2(struct Parser*);


// Now, parser_parse2 can just delegate to match_expression

void parser_bail(const char* msg) {
  fprintf(stderr, "parser error: %s\n", msg);
  exit(1);
}

int parser_done(struct Parser*);

struct Tree* parser_parse2(struct Parser* p) {
  struct Tree* r = match_expression(p);
  if (!parser_done(p)) {
    parser_bail("trailing input");
  }
  return r;
}

// This seems like a function we'll need in the parser.
struct Token consume(struct Parser*, int);

// another potentially useful function
int is_op_token(int t) {
  return t == TOKEN_PLUS || t == TOKEN_MINUS || t == TOKEN_MUL || t == TOKEN_DIV;
}

// let's take a shot at defining match_expression

struct Token parser_current(struct Parser*);
void parser_advance(struct Parser*);

struct Tree* match_expression(struct Parser* p) {
  consume(p, TOKEN_LPAREN);
  struct Token t = parser_current(p);
  if (!is_op_token(t.t)) {
    parser_bail("expected op");
  }
  parser_advance(p);
  struct Tree* left = match_expression(p);
  struct Tree* right = match_expression(p);
  consume(p, TOKEN_RPAREN);
  return binary_node(*t.s, left, right);
}

// problem: match_expression is wrong because it doesn't handle the base case of
// an integer expression.

// let's fix that

struct Tree* match_binary_expression(struct Parser* p);

struct Tree* match_expression2(struct Parser* p) {
  struct Token t = parser_current(p);
  if (t.t == TOKEN_LPAREN) {
    return match_binary_expression(p);
  } else if (t.t == TOKEN_NUM) {
    parser_advance(p);
    return leaf_node(strtol(t.s, NULL, 10));
  } else {
    parser_bail("expected expression");
  }
}

// now we can define match_binary_expression like the old match_expression

struct Tree* match_binary_expression(struct Parser* p) {
  consume(p, TOKEN_LPAREN);
  struct Token t = parser_current(p);
  if (!is_op_token(t.t)) {
    parser_bail("expected op");
  }
  parser_advance(p);
  struct Tree* left = match_expression2(p);
  struct Tree* right = match_expression2(p);
  consume(p, TOKEN_RPAREN);
  return binary_node(*t.s, left, right);
}

// now we need to redefine parser_parse2 (again) to use the new match_expression2
struct Tree* parser_parse3(struct Parser* p) {
  struct Tree* r = match_expression2(p);
  if (!parser_done(p)) {
    parser_bail("trailing input");
  }
  return r;
}

// now that the top-level parsing functions are done, let's work our way down.


struct Token consume(struct Parser* p, int t) {
  struct Token tk = parser_current(p);
  if (tk.t != t) {
    parser_bail("unexpected token type");
  }
  return tk;
}

// it's evident now that parser_advance, parser_current, and parser_done just
// need to call out to the tokenizer class

// so we can finally define the parser

struct Parser {
  struct Tokenizer tz;
};

// ...and define the parser functions as simple wrappers

int parser_done(struct Parser* p) {
  return tokenizer_done(&p->tz);
}

struct Token parser_current(struct Parser* p) {
  return tokenizer_current(&p->tz);
}

void parser_advance(struct Parser* p) {
  tokenizer_advance(&p->tz);
}

// we still need to define tokenizer_advance. but I think it will be more fun to
// work on the evaluator instead


// we can split eval into two cases: recursive and not

// also, for consistency let's have eval take a pointer argument

int eval2(struct Tree*);

int eval_leaf(struct Tree*);
int eval_binary(struct Tree*);

// eval_leaf is dead simple

int eval_leaf(struct Tree* tr) {
  return tr->value;
}

int eval_binary(struct Tree* tr) {
  int left = eval2(tr->left);
  int right = eval2(tr->right);
  if (tr->op == '+') {
    return left + right;
  } else if (tr->op == '-') {
    return left - right;
  } else if (tr->op == '*') {
    return left * right;
  } else if (tr->op == '/') {
    return left / right;
  } else {
    // should never happen
    return -1;
  }
}

// eval2 just needs to select the proper function

int eval2(struct Tree* tr) {
  if (tr->left != NULL) {
    return eval_binary(tr);
  } else {
    return eval_leaf(tr);
  }
}

// we are now prepared to define eval_string
// (it's a good thing we aren't using eval directly in the test cases, or else
//  we'd have to redefine them to use eval2)

struct Parser parser_init(struct Tokenizer* tz) {
  struct Parser p;
  p.tz = *tz;
  return p;
}

int eval_string(const char* p) {
  struct Tokenizer tz = tokenizer_init(p);
  struct Parser pr = parser_init(&tz);
  struct Tree tr = parser_parse(&pr);
  return eval2(&tr);
}

// oops, accidentally used parser_parse instead of parser_parse2 in eval_string
// fortunately parser_parse isn't defined and we can patch it up to use
// parser_parse2

struct Tree parser_parse(struct Parser* p) {
  struct Tree* tr = parser_parse2(p);
  return *tr;
}

// returning a tree struct instead of a pointer means that users of parser_parse
// won't be able to free the heap memory -- which is okay because we weren't
// doing that anyway

