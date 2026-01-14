MODULE Case8;

PROCEDURE P (i: CHAR);
  BEGIN
    CASE i OF
    | 1X, 1X:
    END;
  END P;

BEGIN
  P(1X)
END Case8.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
