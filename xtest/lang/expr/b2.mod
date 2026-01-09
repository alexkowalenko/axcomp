(*
RUN: %comp %s | filecheck %s
CHECK: 2
*)

(* add integer *)
MODULE b2;
BEGIN
    RETURN 1 + 1
END b2.