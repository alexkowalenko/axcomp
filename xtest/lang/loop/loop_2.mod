(*
RUN:  %comp %s | filecheck %s
*)

MODULE loop_2;
const
    max_x = 5;
    max_y = 3;
VAR 
    x, y : INTEGER;
BEGIN
    x := 1;
    y := 1;
    LOOP
         WriteInt(x); WriteLn;
         x := x + 1;
         y := 0;
        LOOP
            y := y + 1;
            if y > max_y then
                exit;
            end;
            WriteInt(y); WriteLn;
        end;
        if x > max_x then
            exit
        end
    END;
    WriteBoolean(TRUE); WriteLn;
END loop_2.

(*
CHECK: 1
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 2
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 3
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 4
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 5
CHECK-NEXT: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 1
*)
