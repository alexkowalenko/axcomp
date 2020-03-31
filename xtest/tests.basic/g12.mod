MODULE g12; (* RECORD parameters and return value *)
        
VAR pt : RECORD x, y : INTEGER; END;

PROCEDURE identity(a : RECORD x, y : INTEGER END) : RECORD x, y : INTEGER END;
BEGIN
    RETURN a
END identity;

PROCEDURE sum(a : RECORD x, y : INTEGER END) : INTEGER;
VAR total : INTEGER;
BEGIN
    RETURN a.x + a.y
END sum;

BEGIN
    pt.x := 12;
    pt.y := 24;
    RETURN sum(identity(pt))
END g12.