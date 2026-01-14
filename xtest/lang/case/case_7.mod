MODULE case_7;
IMPORT Out;
VAR i: INTEGER;

BEGIN
    FOR i := 1 TO 15 DO
        Out.Int(i, 0); Out.Char(' ');
        CASE i OF
            1 : Out.String("x 1"); Out.Ln;
            | 2..4 : Out.String("x 2..4"); Out.Ln;
            | 5, 8 :  Out.String("x 5, 8"); Out.Ln;
            | 10, 12..14 : Out.String("x 10, 12..14"); Out.Ln;
        END
    END
    RETURN 0
END case_7.

(*
RUN: %comp %s | filecheck %s
CHECK: 1 x 1
CHECK-NEXT: 2 x 2..4
CHECK-NEXT: 3 x 2..4
CHECK-NEXT: 4 x 2..4
CHECK-NEXT: 5 x 5, 8
CHECK-NEXT: 6 7 8 x 5, 8
CHECK-NEXT: 9 10 x 10, 12..14
CHECK-NEXT: 11 12 x 10, 12..14
CHECK-NEXT: 13 x 10, 12..14
CHECK-NEXT: 14 x 10, 12..14
CHECK-NEXT: 15 0
*)
