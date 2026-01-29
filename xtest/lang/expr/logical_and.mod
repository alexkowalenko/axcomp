
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_and;
BEGIN
    RETURN TRUE & TRUE;
END logical_and.

(*
CHECK: 1
*)
