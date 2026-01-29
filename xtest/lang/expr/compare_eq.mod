
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_eq;
BEGIN
    RETURN 1 = 2 + 1; 
END compare_eq.

(*
CHECK: 0
*)
