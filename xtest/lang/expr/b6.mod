(*
RUN: %comp %s | filecheck %s
CHECK: 47
*)

(* multiple integer *)
MODULE b6;
BEGIN
    RETURN 2115 DIV 45
END b6.