
(* Test generation of type conversion operators *)

MODULE Fold2;

VAR
  i1, i2 : INTEGER;
  s1, s2 : SHORTINT;
BEGIN
  WriteInt(i1); WriteLn;
  WriteInt(i1 + i2); WriteLn;
  WriteInt(s1 + i1); WriteLn;
  WriteInt(i1 + s1); WriteLn;
  WriteInt(s1 + s2); WriteLn;
END Fold2.
