
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_and_3;
BEGIN
    RETURN TRUE & FALSE;
END logical_and_3.

(*
CHECK: 0
*)
