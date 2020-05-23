MODULE string3; (* STRING LEN *)
IMPORT Out;
CONST a = "More constants";
VAR   x: STRING;

BEGIN
   x := 'Hello';
   Out.String("Hello World!"); Out.Char(' '); Out.Int(LEN("Hello World!"), 0); Out.Ln;
   Out.String(x); Out.Char(' '); Out.Int(LEN(x), 0); Out.Ln;
   Out.String(a); Out.Char(' '); Out.Int(LEN(a), 0); Out.Ln;
   RETURN 0;
END string3.