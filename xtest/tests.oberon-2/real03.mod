MODULE real03; (* REAL *)
IMPORT Out;
VAR x : REAL;
BEGIN
    Out.String("Min: "); Out.Real(MIN(REAL)); Out.Ln;
    Out.String("Max: "); Out.Real(MAX(REAL)); Out.Ln;
    Out.String("Abs: "); Out.Int(ABS(-1), 0); Out.Ln;
    x := -1.2;
    Out.String("Abs: "); Out.Real(ABS(x)); Out.Ln;
    Out.String("Floor: "); Out.Int(FLOOR(x), 0); Out.Ln;
    Out.String("FLT: "); Out.Real(FLT(3)); Out.Ln;
    RETURN 0;
END real03.