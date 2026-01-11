(*
RUN: %comp %s | filecheck %s
CHECK: 12
*)

(* constants *)
MODULE c01;
CONST
    x = 1;
    y = 2;
BEGIN
    RETURN 12
END c01.