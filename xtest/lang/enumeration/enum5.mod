MODULE enum5;
IMPORT Out;
TYPE color = (red, green, blue);
VAR c1, c2: color;
BEGIN
    c1 := red;
    c2 := green;
    IF c1 = c2 THEN
        Out.String("eq");
    ELSE
        Out.String("ne");
    END;
    Out.Ln;
    IF c1 # c2 THEN
        Out.String("ne");
    ELSE
        Out.String("eq");
    END;
    Out.Ln;
END enum5.

(*
RUN: %comp %s | filecheck %s
CHECK: ne
CHECK-NEXT: ne
CHECK-NEXT: 0
*)
