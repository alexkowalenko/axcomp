MODULE g10; (* Mix ARRAY and RECORD *)
        
VAR pt : ARRAY 3 OF RECORD
        x, y: INTEGER;
    END;
    i: INTEGER;

BEGIN
    FOR i := 0 TO 2 DO
        pt[i].x := i;
        pt[i].y := i * 3
    END;
    RETURN pt[1].x + pt[1].y
END g10.

(*
RUN: %comp %s | filecheck %s
CHECK: 4
*)