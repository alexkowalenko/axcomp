MODULE g07; (* Records *)

VAR pt : RECORD 
        x, y, z : INTEGER;
    END;

BEGIN
    pt.x := 3;
    RETURN pt.x
END g07.