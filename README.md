# axcomp

AX compiler

## Lexer

* Integers.
* Identifiers - start with letter, should be UTF-8, can contain '_'.
* Comments: `(* hello sunshine! *)`
* Whitespace: space, tab, newline. UTF-8 encoding for Unicode characters.
* Keywords: `MODULE`, `BEGIN`, `END`, `DIV`, `MOD`, `CONST`, `TYPE`, `VAR`.
* Types: INTEGER (64-bit signed).

## Grammar

Notation:

* `A = B` A devires from B.
* `A | B` either A or B.
* `()` grouping.
* `()*` zero or more times.
* `()+` one or more times (must at least occur once) (`{}` in EBNF).
* `[]` zero or one times (optional)  

```ebnf
module = "MODULE" IDENT ";"
         declarations
         procedures
         ["BEGIN"
                statement_seq]
         "END" IDENT "."

declarations = ["CONST" (IDENT "=" INTEGER ";")* ]
               ["TYPE" (IDENT "=" type ";")* ]
               ["VAR" (IDENT ":" type ";")* ]

procedures = ( procedureDeclaration ";")*

procedureDeclaration = procedureHeading ";" procedureBody

procedureHeading = "PROCEDURE" IDENT [formalParameters]

formalParameters = "(" [fpection {";" fpSection}] ")" [ ":" IDENT ]

fpSection = ["VAR"] identList ":" type

procedureBody = declarations ["BEGIN" statement_seq] "END" IDENT.

type = INDENT

identList = IDENT ("," IDENT)+

statement_seq = (statement ";")+

statement = assignment
    | procedureCall
    | ifStatment
    | forStatement
    | whileStatement
    | repeatStatement
    | loopStatment
    | block
    | "EXIT"
    | "RETURN" [expr]

assignment = IDENT ":=" expr

procedureCall = IDENT "(" expr ( "," expr )* ")"

ifStatement = "IF" expression "THEN" statement_seq
    ( "ELSIF" expression "THEN" statement_seq )*
    [ "ELSE" statement_seq ] "END"

forStatement = "FOR" IDENT ":=" expr "TO" expr [ "BY" INTEGER ] "DO"
    statement_seq "END"

whileStatement = "WHILE" expr "DO" statement_seq "END"

repeatStatement = "REPEAT" statement_seq "UNTIL" expr

loopStatement = "LOOP" statement_seq "END"

block = "BEGIN" statement_seq "END"

expr = simpleExpr [ relation simpleExpr]

relation = "=" | "#" | "<" | "<=" | ">" | ">="

simpleExpr = ('+' | '-' )? term ( ('+' | '-' | "OR" ) term)*

term = factor ( ( '*' | 'DIV' | 'MOD' | "&" ) factor)*

factor = IDENT | procedureCall | INTEGER | "TRUE" | "FALSE" | '(' expr ')' | "~" factor

IDENT = letter (letter | digit | '_')*

INTEGER = digit+
```

## Example program

```pascal
(* Sample program *)
MODULE test;
CONST
    x = 1;
VAR
    z : INTEGER;
    check : BOOLEAN;

PROCEDURE f;
    VAR yy : INTEGER;
BEGIN
    RETURN ;
END f;

PROCEDURE g(x: INTEGER; y: INTEGER): INTEGER;
BEGIN
    RETURN x + y * y;
END f;

BEGIN
    z := 12;
    check := FALSE;
    f();
    IF z < 12 THEN
        z := z + 1;
    ELSE
        z := z - 1;
    END;

    FOR i := 0 TO 9 BY 2 DO
        z := i * i + z;
    END;

    WHILE x > 4 DO
        x := x - 1;
    END;

    REPEAT
        z = z + 1;
    UNTIL z > 10;

    LOOP
        IF x < 3 THEN
            EXIT;
        END;
        x := x - 1;
    END;

    BEGIN
        x := 4;
    END;

    RETURN (3 * g(1, 3)) + ((2 + (g(2, 3) + 1)) * 4);
END test.
```

## Builtin functions

WriteInt(x: INTEGER)
WriteLn()

## Notes

Changes to the standard definition:

* Procedures don't have nested procedures.

## Program options

Program `ax`:

* `-f,--file filename` - compile named file.
* `-m,--main` - the `BEGIN` and `END` part of this module will be executed as the `main()` of the program.

* `-D`string - Debugging options `string` is composed of the following characters:

   `p` - print out parsed tree.
