
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_and_2;
BEGIN
    RETURN FALSE & TRUE;
END logical_and_2.

(*
CHECK: 0
*)
