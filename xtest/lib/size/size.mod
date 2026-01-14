MODULE size; (* LEN function*)
IMPORT Out;
TYPE arrayType = ARRAY 7 OF INTEGER;
VAR x : INTEGER;
    y : ARRAY 7 OF INTEGER;

PROCEDURE f(x : INTEGER) : INTEGER;
BEGIN
  RETURN 0
END f;

BEGIN
    x := SIZE(BOOLEAN);
    Out.Int(x, 0); Out.Ln;
    x := SIZE(CHAR);
    Out.Int(x, 0); Out.Ln;
     x := SIZE(INTEGER);
    Out.Int(x, 0); Out.Ln;
    x := SIZE(arrayType);
    Out.Int(x, 0); Out.Ln;
END size.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 4
CHECK-NEXT: 8
CHECK-NEXT: 56
CHECK-NEXT: 0
*)
