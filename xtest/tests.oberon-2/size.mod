MODULE size; (* LEN function*)
IMPORT Out;
TYPE arrayType = ARRAY 7 OF INTEGER;
VAR x : INTEGER;
    y : ARRAY 7 OF INTEGER;

PROCEDURE f(x : INTEGER) : INTEGER;
BEGIN
  RETURN 0
END f;

BEGIN
    x := SIZE(BOOLEAN);
    Out.Int(x); Out.Ln;
    x := SIZE(CHAR);
    Out.Int(x); Out.Ln;
     x := SIZE(INTEGER);
    Out.Int(x); Out.Ln;
    x := SIZE(arrayType);
    Out.Int(x); Out.Ln;
END size.