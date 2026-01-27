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

(*
RUN: %comp %s | filecheck %s
CHECK:alpha  6
CHECK-NEXT:  beta  5
CHECK-NEXT:  alpha beta  11
CHECK-NEXT:  alpha  6
CHECK-NEXT:  beta  5
CHECK-NEXT:  alpha beta gamma  17
CHECK-NEXT:  alpha beta gamma gamma  23
CHECK-NEXT:  alpha A 7
CHECK-NEXT:  0alpha  7
CHECK-NEXT:  0
*)