MODULE string3; (* STRING LEN *)
IMPORT Out;
CONST a = "More constants";
VAR   x: STRING;

BEGIN
   x := 'Hello';
   Out.String("Hello World!"); Out.Char(' '); Out.Int(LEN("Hello World!")); Out.Ln;
   Out.String(x); Out.Char(' '); Out.Int(LEN(x)); Out.Ln;
   Out.String(a); Out.Char(' '); Out.Int(LEN(a)); Out.Ln;
   RETURN 0;
END string3.