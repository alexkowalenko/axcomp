(*
RUN: %comp %s | filecheck %s
CHECK: 232
*)

MODULE c05; (* vars *)
CONST
    alpha = 24;
VAR
    x : INTEGER;
    y : INTEGER;
BEGIN
    RETURN x - alpha (* -24 *)
END c05.


END c05.