MODULE stdlib01;  (* Test the stand library output routines *)

IMPORT Out;
CONST x = 12;

BEGIN
    Out.Open;
    Out.Int(x); Out.Ln;
    Out.Int(0cafebabeH); Out.Ln;
    Out.Hex(0cafebabeH); Out.Ln;
    Out.Bool(TRUE); Out.Ln;
    Out.Flush;
END stdlib01.