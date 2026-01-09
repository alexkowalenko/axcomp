(*
RUN: %comp %s | filecheck %s
CHECK: 23
*)

(* multiple integer *)
MODULE b8;
BEGIN
    RETURN 3 + 5 * 4 (* test precedence, should be 23, not 24 *)
END b8.