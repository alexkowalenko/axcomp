(*
RUN: %comp %s | filecheck %s
*)

MODULE const_real_1;
IMPORT Out;
CONST
    pi = 3.141592;
VAR
    r : REAL;
    area : REAL;
BEGIN
    r := 4.0;
    area := pi * r * r;
    Out.Real(area, 6); WriteLn;
END const_real_1.

(*
CHECK: 50.2655
*)
