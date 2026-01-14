MODULE factorial;

PROCEDURE factor(x : INTEGER) : INTEGER;
BEGIN
     WriteInt(x); WriteLn;
    IF x <= 1 THEN
        RETURN 1
    END
    RETURN x * factor (x -1)
END factor;

BEGIN
    WriteInt( factor(10)); WriteLn;
    RETURN 0;
END factorial.

(*
RUN: %comp %s | filecheck %s
CHECK: 10
CHECK-NEXT: 9
CHECK-NEXT:8
CHECK-NEXT:7
CHECK-NEXT:6
CHECK-NEXT:5
CHECK-NEXT:4
CHECK-NEXT:3
CHECK-NEXT:2
CHECK-NEXT:1
CHECK-NEXT:3628800
CHECK-NEXT: 0
*)
