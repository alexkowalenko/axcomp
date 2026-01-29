
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_arithmetic;
BEGIN
    RETURN 2 * (-2) = 1 + 5;
END compare_arithmetic.

(*
CHECK: 0
*)
