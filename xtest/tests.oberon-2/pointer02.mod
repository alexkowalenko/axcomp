MODULE pointer02; (* pointers *)
IMPORT Out;
VAR x : POINTER TO INTEGER;
    y : INTEGER;
BEGIN
    NEW(x);
    x^ := 5;
    Out.Int(x^, 0); Out.Ln;
    y := 5;
    x^ := x^ + y;
    Out.Int(x^, 0); Out.Ln;
    Out.Int(SIZE(x^), 0); Out.Ln;
    RETURN 0;
END pointer02.