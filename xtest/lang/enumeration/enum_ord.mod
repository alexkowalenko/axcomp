MODULE enum_ord;
IMPORT Out;
TYPE color = (red, green, blue);
VAR i: INTEGER;
    c: color;
BEGIN
    i := ORD(green);
    Out.Int(i, 0); Out.Ln;
    c := blue;
    Out.Int(ORD(c),0); Out.Ln;
END enum_ord.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 0
*)
