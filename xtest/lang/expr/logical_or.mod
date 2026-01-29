
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_or;
BEGIN
    RETURN TRUE OR TRUE;
END logical_or.

(*
CHECK: 1
*)
