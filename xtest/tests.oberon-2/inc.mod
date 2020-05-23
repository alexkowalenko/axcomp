MODULE inc; (* INC()/DEC() *)
IMPORT Out;
VAR   x: INTEGER;
      y: INTEGER;

BEGIN
    x := 6; y := x;
    INC(x);
    Out.Int(x, 0); Out.Ln;
    DEC(x);
    Out.Int(x, 0); Out.Ln;
    IF x = y THEN
        Out.String("Same"); Out.Ln;
    ELSE
        Out.String("Different"); Out.Ln;
    END
    RETURN 0;
END inc.