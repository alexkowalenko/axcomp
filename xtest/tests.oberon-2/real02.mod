MODULE real02; (* REAL *)
IMPORT Out;
CONST pi = 3.1415927;
VAR x : REAL;

BEGIN
    x := pi - 1;
    Out.Real(x); Out.Ln;

    x := pi + 1;
    Out.Real(x); Out.Ln;

    x := pi * pi;
    Out.Real(x); Out.Ln;

    x := pi / 2;
    Out.Real(x); Out.Ln;

    x := pi * 0;
    Out.Real(x); Out.Ln;

    x := pi / 0;
    Out.Real(x); Out.Ln;
    RETURN 0;
END real02.