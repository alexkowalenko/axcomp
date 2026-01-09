(*
RUN: %comp %s | filecheck %s
CHECK: 8
*)

(* multiple integer *)
MODULE b7;
BEGIN
    RETURN 56 MOD 16
END b7.