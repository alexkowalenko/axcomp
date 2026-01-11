(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE Short1;

PROCEDURE Test* ();
  VAR
    i8: SHORTINT; i16: INTEGER; i32: LONGINT;
    r64: LONGREAL; r32: REAL;
  BEGIN
    i32 := 1;
    i16 := SHORT(i32);
    i8 := SHORT(i16);

    r64 := 1.0;
    r32 := SHORT(r64);

    i8 := SHORT(128-128);
  END Test;

BEGIN
  Test
END Short1.
