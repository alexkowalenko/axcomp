# axcomp

AX compiler

## Lexer

* Integers.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline

## Grammar

```ebnf
module -> (expr ;)+

expr -> INTEGER
```
