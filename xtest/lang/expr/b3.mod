(*
RUN: %comp %s | filecheck %s
CHECK: 98
*)

(* add/subtract integer *)
MODULE b3;
BEGIN
    RETURN -1 + 12 - 123 + 1234
END b3.