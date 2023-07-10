MODULE record16; 
IMPORT Out;
TYPE pt = RECORD x, y : INTEGER; END;
VAR x: pt;

PROCEDURE f(VAR a : pt);
BEGIN
    a.x := 1;
END f;

BEGIN
    Out.Int(x.x, 0); Out.Ln;
    f(x); 
    Out.Int(x.x, 0); Out.Ln;
END record16.