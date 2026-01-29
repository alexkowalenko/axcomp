
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_or_3;
BEGIN
    RETURN TRUE OR FALSE;
END logical_or_3.

(*
CHECK: 1
*)
