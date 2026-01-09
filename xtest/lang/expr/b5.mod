(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

(* multiple integer *)
MODULE b5;
BEGIN
    RETURN 1 * 2 * 3 * 4 * 5
END b5.