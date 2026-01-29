(*
RUN: %comp %s | filecheck %s
*)

MODULE var_real_1;
IMPORT Out;
VAR
    x : REAL;
BEGIN
    x := 3.14159 + (- 2.0);
    Out.Real(x, 6); Out.Ln;
    RETURN 0;
END var_real_1.

(*
CHECK: 1.14159
*)
