(*
RUN:  %comp %s | filecheck %s
*)

MODULE var_real_2;
IMPORT Out;
VAR
    x, y, z : REAL;
BEGIN
    x := 3.14159265;
    y := 2.71828183;
    z := x + y; 
    Out.Real(z, 6); Out.Ln;
    RETURN 0;
END var_real_2.

(*
CHECK: 5.85987
*)
