MODULE string7; (* STRING type *)
IMPORT Out;
VAR   x: STRING;

BEGIN
    NEW(x, 7*SIZE(CHAR));

    x[0] := 'H'; x[1] := 'e'; x[2] := 'l'; x[3] := 'l'; x[4] := 'o'; x[5] := '!';
    Out.String(x); Out.Ln;
    RETURN 0;
END string7.