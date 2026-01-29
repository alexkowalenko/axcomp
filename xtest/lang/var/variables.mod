(*
RUN: %comp %s | filecheck %s
*)

MODULE variables;
VAR
    x, y : INTEGER;
    z : BOOLEAN;
BEGIN
    RETURN 0;
END variables.

(*
CHECK: 0
*)
