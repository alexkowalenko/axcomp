MODULE enum1;
TYPE color = (red, green, blue);
BEGIN
END enum1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
