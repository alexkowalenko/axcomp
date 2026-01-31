MODULE enum_proc1;
IMPORT
    Out;
TYPE
    color = (red, green, blue);
VAR
    c1, c2 : color;

PROCEDURE change(VAR c : color);
BEGIN
    c := blue;
END change;

PROCEDURE print(c : color);
BEGIN
     IF c = red THEN
         Out.String("red"); Out.Ln;
     ELSE
         Out.String("not red"); Out.Ln;
     END;
END print;

BEGIN
    c1 := red;
    print(c1);
    change(c1);
    print(c1);
    RETURN;
END enum_proc1.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK: not red
CHECK-NEXT: 0
*)
