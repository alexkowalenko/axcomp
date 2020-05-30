MODULE string10; (* String concatenation *)
IMPORT Out, Strings;
CONST a = "alpha ";
    b = "beta ";
    cc = "gamma ";
VAR
    c : STRING;
    d : STRING;
    e : CHAR;

PROCEDURE print(s: STRING);
BEGIN
    Out.String(s); Out.Char(' '); Out.Int(LEN(s), 0) Out.Ln;
END print;

BEGIN
    print(a);
    print(b);
    c := a + b;
    print(c);
    print(a);
    print(b);

    d := a + b + cc;
    print(d);

    d := d + cc;
    print(d);

    e := 'A';
    d := a + e;
    print(d);

    e := '0';
    d := e + a;
    print(d);

    RETURN 0
END string10.