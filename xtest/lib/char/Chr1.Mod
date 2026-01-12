MODULE Chr1;

IMPORT Out;

PROCEDURE Chr (x: INTEGER; ch: CHAR);
  BEGIN
    Out.Int(x, 4);
    Out.Char(ch);
    Out.Char(CHR(x));
    Out.Ln;
  END Chr;

PROCEDURE Test* ();
  BEGIN
    Chr (65, "A");
    Chr (97, "a");
    Chr (255, 0FFX);
  END Test;

BEGIN
  Test
END Chr1.

(*
RUN: %comp %s | filecheck %s
CHECK: 65AA
CHECK-NEXT: 97aa
CHECK-NEXT: 255ÿÿ
*)
