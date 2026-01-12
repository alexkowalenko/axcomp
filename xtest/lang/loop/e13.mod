MODULE e13;
IMPORT Out;
VAR x: INTEGER;

BEGIN
    LOOP
        x := x + 1;
        Out.Int(x, 0); Out.Ln;
        IF x # 10 THEN
            x := x;
        ELSE
            EXIT;
        END
    END
    RETURN 0
END e13.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 5
CHECK-NEXT: 6
CHECK-NEXT: 7
CHECK-NEXT: 8
CHECK-NEXT: 9
CHECK-NEXT: 10
*)
