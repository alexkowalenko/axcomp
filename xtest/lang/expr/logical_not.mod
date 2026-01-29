
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_not;
BEGIN
    RETURN ~TRUE;
END logical_not.

(*
CHECK: 0
*)
