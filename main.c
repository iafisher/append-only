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
