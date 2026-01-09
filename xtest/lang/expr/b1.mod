(*
RUN: %comp %s | filecheck %s
CHECK: 244
*)

(* negate integer *)
MODULE b1;
BEGIN
    RETURN - 12
END b1.