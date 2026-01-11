(*
RUN: %comp %s | filecheck %s
CHECK: 4
*)

(* constants *)
MODULE c02;
CONST
    x = 1;
    y = 2;
BEGIN
    RETURN y + 2
END c02.