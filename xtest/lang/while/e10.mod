MODULE e10; (* WHILE block *)

VAR x : INTEGER;
VAR y : INTEGER;

BEGIN
    x := 0;
    WHILE x <= 100 DO
        y := 0;
        WHILE y < 10 DO
            x := x + 1;
            y := y + 1
        END
    END;
    RETURN x 

END e10.

(*
RUN: %comp %s | filecheck %s
CHECK: 110
*)

