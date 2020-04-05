MODULE d13; (* Test VAR args *)

VAR a : INTEGER;

PROCEDURE f(x : INTEGER);
BEGIN
    WriteInt(x); WriteLn();
    x := 10;
    WriteInt(x); WriteLn()
END f;

BEGIN
    a := 1;
    f(a);
    WriteInt(a); WriteLn()
END d13.