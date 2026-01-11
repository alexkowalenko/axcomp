(*
RUN: %comp %s | filecheck %s
CHECK: 220
*)

MODULE c06; (* vars *)
CONST
    alpha = 24;
VAR
    x : INTEGER;
    y : INTEGER;
BEGIN
    x := 3;
    y := x + 987;
    RETURN y (* 990 *)
END c06.