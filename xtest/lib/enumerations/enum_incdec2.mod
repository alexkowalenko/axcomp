MODULE enum_incdec2;
IMPORT Out;
TYPE color = (red, green, blue, yellow);
VAR c: color;

PROCEDURE print (c : color);
BEGIN
    CASE c OF
        red : Out.String("red"); Out.Ln;
        | green : Out.String("green"); Out.Ln;
        | blue : Out.String("blue"); Out.Ln;
        | yellow : Out.String("yellow"); Out.Ln;
    END;
END print;

BEGIN
    c := red;
    print(c);
    c := INC(c);
    print(c);
    INC(c);
    print(c);
    INC(c);
    print(c);

    c := DEC(c);
    print(c);
    DEC(c);
    print(c);
    DEC(c);
    print(c);
END enum_incdec2.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK-NEXT: green
CHECK-NEXT: blue
CHECK-NEXT: yellow
CHECK-NEXT: blue
CHECK-NEXT: green
CHECK-NEXT: red
CHECK-NEXT: 0
*)
