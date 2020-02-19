# axcomp

AX compiler

## Lexer 

* Integers.
* Comments: `// until to end of line`
* Whitespace: space, tab, newline

## Grammar

```
prog -> expr (; expr)*

expr -> INTEGER
```
