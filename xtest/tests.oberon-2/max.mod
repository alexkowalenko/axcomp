MODULE max; (* MAX MIN function*)
IMPORT Out;
VAR x: INTEGER;
  b: BOOLEAN;
BEGIN
    x := MAX(INTEGER);
    Out.Int(x);  Out.Ln;
    b := MIN(BOOLEAN);
    Out.Bool(b); Out.Ln;

    Out.Bool(MAX(BOOLEAN)); Out.Char(' '); Out.Bool(MIN(BOOLEAN)); Out.Ln;
    Out.Int(MAX(INTEGER)); Out.Char(' '); Out.Int(MIN(INTEGER)); Out.Ln;
END max.