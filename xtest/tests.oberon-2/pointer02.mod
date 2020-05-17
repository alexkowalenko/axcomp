MODULE pointer02; (* pointers *)
IMPORT Out;
VAR x : POINTER TO INTEGER;
    y : INTEGER;
BEGIN
    NEW(x);
    x^ := 5;
    Out.Int(x^); Out.Ln;
    y := 5;
    x^ := x^ + y;
    Out.Int(x^); Out.Ln;
    Out.Int(SIZE(x^)); Out.Ln;
    RETURN 0;
END pointer02.