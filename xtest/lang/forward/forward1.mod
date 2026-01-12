MODULE forward1;
IMPORT Out;

PROCEDURE ^f (x : INTEGER): INTEGER;

PROCEDURE g() : INTEGER;
BEGIN
    Out.String("g()"); Out.Ln;
    RETURN f(1);
END g;

PROCEDURE f (x : INTEGER): INTEGER;
VAR y: INTEGER;
BEGIN
     Out.String("f()"); Out.Ln;
    RETURN 0;
END f;

BEGIN
    g;
END forward1.


(*
RUN: %comp %s | filecheck %s
CHECK: g()
CHECK-NEXT: f()
CHECK-NEXT: 0
*)
