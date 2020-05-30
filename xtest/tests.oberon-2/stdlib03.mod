MODULE stdlib03; (* Stdlib *)
IMPORT Out, Strings;
CONST a = "Hello";
    b = " World";
VAR
    c : STRING;

PROCEDURE print(s: STRING);
BEGIN
    Out.String(s); Out.Char(' '); Out.Int(LEN(s), 0) Out.Ln;
END print;


BEGIN
    c := Strings.Concat(a, b);
    print(c);
    c := Strings.ConcatChar(c, '!');
    print(c);
    c := Strings.AppendChar('!', c);
    print(c);
    RETURN 0
END stdlib03.