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

PROCEDURE printCompare(s1, s2: STRING);
BEGIN
    Out.String(s1); Out.Char(' ');  Out.String(s2); Out.String(" -> "); 
    Out.Int(Strings.Compare(s1, s2), 0) Out.Ln;
END printCompare;

BEGIN
    c := Strings.Concat(a, b);
    print(c);
    c := Strings.ConcatChar(c, '!');
    print(c);
    c := Strings.AppendChar('!', c);
    print(c);
    Out.Ln;

    printCompare("a", "a");
    printCompare("a", "b");
    printCompare("b", "a");

    RETURN 0
END stdlib03.