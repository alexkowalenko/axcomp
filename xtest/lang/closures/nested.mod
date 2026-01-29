MODULE nested; (* Nested Procedures *)
IMPORT Out;
VAR x: INTEGER;

PROCEDURE f(): INTEGER;
    CONST x = 2;
    
    PROCEDURE g(): INTEGER;
    CONST x = 1;
    BEGIN 
        Out.Int(x, x); Out.Ln;
        RETURN x;
    END g;
BEGIN
    g();
    Out.Int(x, x); Out.Ln;
    RETURN x;
END f;

 PROCEDURE g(): INTEGER;
    CONST x = 4;
    BEGIN 
        Out.Int(x, x); Out.Ln;
        RETURN x;
    END g;

BEGIN
    x := 3;
    Out.Int(x, x); Out.Ln;
    f();
    Out.Int(x, x); Out.Ln;
    g();
END nested.

(*
RUN: %comp %s | filecheck %s
CHECK:   3
CHECK-NEXT:       1
CHECK-NEXT:        2
CHECK-NEXT:         3
CHECK-NEXT:          4
*)

