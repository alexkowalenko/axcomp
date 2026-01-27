MODULE string9; (* compatibity ARRAY OF CHAR *)

IMPORT Out;

VAR x : ARRAY OF CHAR;
    y : ARRAY OF CHAR;
    z : STRING;

PROCEDURE id(s : ARRAY OF CHAR) : ARRAY OF CHAR;
BEGIN
    RETURN s;
END id;
    
BEGIN
    x := "Hello!";
    NEW(y, 5);
    z := x;
    Out.String(x); Out.Ln;
    Out.String(id(x)); Out.Ln;
     Out.String(z); Out.Ln;
    RETURN 0;
END string9.

(*
RUN: %comp %s | filecheck %s
CHECK:Hello!
CHECK-NEXT: Hello!
CHECK-NEXT: Hello!
CHECK-NEXT: 0
*)