MODULE stdlib01;  (* Test the stand library output routines *)

IMPORT Out;
CONST x = 12;

BEGIN
    Out.Open;
    Out.Int(x, 0); Out.Ln;
    Out.Int(0cafebabeH, 0); Out.Ln;
    Out.Hex(0cafebabeH, 0); Out.Ln;
    Out.Bool(TRUE); Out.Ln;
    Out.Flush;
END stdlib01.