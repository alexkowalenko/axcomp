MODULE enum_incdec;
IMPORT Out;
TYPE color = (red, green, blue, yellow);
VAR c: color;

BEGIN
    c := red;
    c := INC(c);
    Out.Int(ORD(c), 0); Out.Ln;
    INC(c);
    Out.Int(ORD(c), 0); Out.Ln;
    c := DEC(c);
    Out.Int(ORD(c), 0); Out.Ln;
    DEC(c);
    Out.Int(ORD(c), 0); Out.Ln;
END enum_incdec.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 1
CHECK-NEXT: 0
*)
