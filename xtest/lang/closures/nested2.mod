MODULE nested2; (* Nested Procedures with closures *)
IMPORT Out;
VAR x: INTEGER;

PROCEDURE f();
    VAR y: INTEGER;

    PROCEDURE g();
    BEGIN 
        Out.String("g y = "); Out.Int(y, 0); Out.Ln;
        y := 3;
        Out.String("g y = "); Out.Int(y, 0); Out.Ln;
        RETURN
    END g; 

BEGIN
    y := 1;
    Out.String("f y = "); Out.Int(y, 0); Out.Ln;
    g();
    Out.String("f y = "); Out.Int(y, 0); Out.Ln;
    RETURN;
END f;

BEGIN
    f(); 
END nested2.