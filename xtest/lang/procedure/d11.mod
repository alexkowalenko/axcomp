MODULE d11; (* Test call args *)

  PROCEDURE eq(x : INTEGER; y : INTEGER): BOOLEAN;
  BEGIN
      RETURN x = y
  END eq;

  PROCEDURE neq(x : INTEGER; y : INTEGER): BOOLEAN;
  BEGIN
      RETURN x # y
  END neq;

BEGIN
    RETURN ~(eq(1, 1) & neq(1, 1))
END d11.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
*)
