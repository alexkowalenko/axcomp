# axcomp

AX compiler

## Lexer

* Integers.
* Identifiers - start with letter, should be UTF-8.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline. UTF-8 encoding for Unicode characters.
* Keywords: `MODULE`, `BEGIN`, `END`.

## Grammar

```ebnf
module -> "MODULE" IDENT ";"
          "BEGIN"
                statement_seq
          "END" IDENT "."
          
statement_seq -> (expr ";")+

expr -> INTEGER

IDENT -> letter (letter | digit)*

INTEGER -> digit+
```

## Example program

```pascal
(* Sample program *)
MODULE test;
BEGIN
    12;
    24;
END test.
```
