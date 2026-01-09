(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE real1;
VAR f : REAL;
BEGIN
    f := 3.14;
    RETURN 0;
END real1.