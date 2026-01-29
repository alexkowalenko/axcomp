
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_or_4;
BEGIN
    RETURN FALSE OR FALSE;
END logical_or_4.

(*
CHECK: 0
*)
