MODULE array15; 
IMPORT Out;
VAR x: ARRAY 3 OF INTEGER;

PROCEDURE f(VAR a : ARRAY 3 OF INTEGER);
BEGIN
    a[0] := 1;
END f;

BEGIN
    Out.Int(x[0], 0); Out.Ln;
    f(x); 
    Out.Int(x[0], 0); Out.Ln;
END array15.


(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
*)