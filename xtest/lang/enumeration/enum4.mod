MODULE enum4;
IMPORT
    Out;
TYPE
    color = (red, green, blue);
VAR
    c1, c2 : color;
BEGIN
    c1 := red;
    c2 := c1;
    IF c1 = red THEN
        Out.String("red"); Out.Ln;
    ELSE
        Out.String("not red"); Out.Ln;
    END;
    IF c2 # red THEN
        Out.String("not red"); Out.Ln;
    ELSE
        Out.String("red"); Out.Ln;
    END;

END enum4.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK-NEXT: red
CHECK-NEXT: 0
*)
