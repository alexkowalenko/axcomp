
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_eq_2;
BEGIN
    RETURN 2 + 1 = 2 + 1;
END compare_eq_2.

(*
CHECK: 1
*)
