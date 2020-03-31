MODULE g07; (* Records *)

VAR pt : RECORD 
        x, y, z : INTEGER;
    END;

BEGIN
    pt.x := 3;
    pt.y := 2;
    pt.z := 5;
    RETURN pt.x + pt.y + pt.z
END g07.