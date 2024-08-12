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
