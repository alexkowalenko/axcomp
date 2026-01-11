(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE Long1;

PROCEDURE Test* ();
  VAR
    i8: SHORTINT; i16: INTEGER; i32: LONGINT;
    r64: LONGREAL; r32: REAL;
  BEGIN
    i8 := 1;
    i16 := LONG(i8);
    i32 := LONG(i16);
    
    r32 := 1.0;
    r64 := LONG(r32);

    i32 := LONG(MAX(LONGINT));
  END Test;

BEGIN
  Test
END Long1.
