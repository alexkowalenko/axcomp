MODULE enum_minmax;
IMPORT Out;
TYPE color = (red, green, blue);

PROCEDURE print (c : color);
BEGIN
    CASE c OF
    red : Out.String("red"); Out.Ln;
    | green : Out.String("green"); Out.Ln;
    | blue : Out.String("blue"); Out.Ln;
    END;
END print;

BEGIN
    print(MIN(color));
    print(MAX(color));

    Out.Int(ORD(MIN(color)), 0); Out.Ln;
    Out.Int(ORD(MAX(color)), 0); Out.Ln;
END enum_minmax.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK-NEXT: blue
CHECK-NEXT: 0
CHECK-NEXT: 2
CHECK-NEXT: 0
*)
