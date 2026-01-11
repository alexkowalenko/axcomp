(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)

MODULE real02; (* REAL *)
IMPORT Out;
CONST pi = 3.1415927;
VAR x : REAL;

BEGIN
    x := pi - 1;
    Out.Real(x, 0); Out.Ln;

    x := pi + 1;
    Out.Real(x, 0); Out.Ln;

    x := pi * pi;
    Out.Real(x, 0); Out.Ln;

    x := pi / 2;
    Out.Real(x, 0); Out.Ln;

    x := pi * 0;
    Out.Real(x, 0); Out.Ln;

    x := pi / 0;
    Out.Real(x, 0); Out.Ln;
    RETURN 0;
END real02.