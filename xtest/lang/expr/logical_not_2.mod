
(*
RUN: %comp %s | filecheck %s
*)

MODULE logical_not_2;
BEGIN
    RETURN ~FALSE;
END logical_not_2.

(*
CHECK: 1
*)
