
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_and_4;
BEGIN
    RETURN FALSE & FALSE;
END logical_and_4.

(*
CHECK: 0
*)
