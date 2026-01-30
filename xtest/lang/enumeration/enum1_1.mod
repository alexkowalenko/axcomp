MODULE enum1_1;
TYPE color = (red, green, blue);
VAR c : color;
BEGIN
    c := red;
END enum1_1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
