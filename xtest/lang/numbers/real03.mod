(*
RUN: %comp %s | filecheck %s
CHECK: Min: 2.22507E-308
CHECK-NEXT: Max: 1.79769E+308
CHECK-NEXT: Abs: 1
CHECK-NEXT: Abs: 1.2
CHECK-NEXT: Floor: -2
CHECK-NEXT: FLT: 3
CHECK-NEXT: 0
*)

MODULE real03; (* REAL *)
IMPORT Out;
VAR x : REAL;
BEGIN
    Out.String("Min: "); Out.Real(MIN(REAL), 0); Out.Ln;
    Out.String("Max: "); Out.Real(MAX(REAL), 0); Out.Ln;
    Out.String("Abs: "); Out.Int(ABS(-1), 0); Out.Ln;
    x := -1.2;
    Out.String("Abs: "); Out.Real(ABS(x), 0); Out.Ln;
    Out.String("Floor: "); Out.Int(FLOOR(x), 0); Out.Ln;
    Out.String("FLT: "); Out.Real(FLT(3), 0); Out.Ln;
    RETURN 0;
END real03.