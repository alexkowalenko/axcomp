MODULE nested3; (* Nested Procedures *)
IMPORT Out;
VAR x: INTEGER;

PROCEDURE f();
    VAR y: INTEGER;

    PROCEDURE g(a : INTEGER);
    BEGIN 
        Out.String("g a = "); Out.Int(a, 0); Out.Ln;
        Out.String("g y = "); Out.Int(y, 0); Out.Ln;
        y := 3;
        Out.String("g y = "); Out.Int(y, 0); Out.Ln;
        RETURN
    END g; 

BEGIN
    y := 1;
    Out.String("f y = "); Out.Int(y, 0); Out.Ln;
    g(7);
    Out.String("f y = "); Out.Int(y, 0); Out.Ln;
    RETURN;
END f;

BEGIN
    f(); 
END nested3.