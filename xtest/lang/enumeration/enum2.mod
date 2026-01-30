MODULE enum2;
TYPE day = (Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday);
BEGIN
END enum2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
