MODULE enum_case;
IMPORT
    Out;
TYPE
    color = (red, green, blue);
VAR
    c1, c2 : color;

PROCEDURE print (c : color);
BEGIN
    CASE c OF
    red : Out.String("red"); Out.Ln;
    | green : Out.String("green"); Out.Ln;
    | blue : Out.String("blue"); Out.Ln;
    END;
END print;

BEGIN
    c1 := red;
    print(c1);
    c2 := blue;
    print(c2);
    print(green);
END enum_case.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK-NEXT: blue
CHECK-NEXT: green
CHECK-NEXT: 0
*)
