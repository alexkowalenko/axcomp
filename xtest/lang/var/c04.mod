(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE c04; (* vars *)
VAR
    x : INTEGER;
    y : INTEGER;
BEGIN
    RETURN x + y
END c04.