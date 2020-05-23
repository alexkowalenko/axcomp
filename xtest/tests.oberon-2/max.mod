MODULE max; (* MAX MIN function*)
IMPORT Out;
VAR x: INTEGER;
  b: BOOLEAN;
BEGIN
    x := MAX(INTEGER);
    Out.Int(x, 0);  Out.Ln;
    b := MIN(BOOLEAN);
    Out.Bool(b); Out.Ln;

    Out.Bool(MAX(BOOLEAN)); Out.Char(' '); Out.Bool(MIN(BOOLEAN)); Out.Ln;
    Out.Int(MAX(INTEGER), 0); Out.Char(' '); Out.Int(MIN(INTEGER), 0); Out.Ln;
END max.