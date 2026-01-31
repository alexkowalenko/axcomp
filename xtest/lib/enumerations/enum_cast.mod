MODULE enum_cast;
IMPORT Out;
TYPE color = (red, green, blue);
VAR c: color;
BEGIN
    c := CAST(color, 2);
    IF c = blue THEN
        Out.String("blue"); Out.Ln;
    ELSE
        Out.String("not blue"); Out.Ln;
    END;

    IF CAST(color, ORD(blue)) = blue THEN
        Out.String("Equal by ORD"); Out.Ln;
    ELSE
        Out.String("Not equal by ORD"); Out.Ln;
    END;

END enum_cast.

(*
RUN: %comp %s | filecheck %s
CHECK: blue
CHECK-NEXT: Equal by ORD
CHECK-NEXT: 0
*)
