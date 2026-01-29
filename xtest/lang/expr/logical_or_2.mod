
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_or_2;
BEGIN
    RETURN FALSE OR TRUE;
END logical_or_2.

(*
CHECK: 1
*)
