(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE OpExprCompat16;

VAR
i: HUGEINT;
BEGIN
i := 9223372036854775808+1;
END OpExprCompat16.
