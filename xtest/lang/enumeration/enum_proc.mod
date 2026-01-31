MODULE enum_proc;
IMPORT
    Out;
TYPE
    color = (red, green, blue);
VAR
    c1, c2 : color;

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
    c2 := blue;
    print(c2);
    print(green);
END enum_proc.

(*
RUN: %comp %s | filecheck %s
CHECK: red
CHECK-NEXT: not red
CHECK-NEXT: not red
CHECK-NEXT: 0
*)
