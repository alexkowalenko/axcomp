MODULE g08; (* Records *)

VAR pt : RECORD
        x, y: INTEGER;
        z :RECORD
            x : INTEGER;
            y : INTEGER
        END;
        a : BOOLEAN;
    END;

BEGIN
    pt.x := 1;
    pt.y := 2;
    pt.z.x := 1;
    pt.z.y := 1;
    pt.a := TRUE;
    RETURN pt.x + pt.y + pt.z.x + pt.z.y
END g08.


(*
RUN: %comp %s | filecheck %s
CHECK: 5
*)