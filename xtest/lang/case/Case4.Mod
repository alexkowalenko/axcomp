MODULE Case4;

PROCEDURE P (i, j: INTEGER);
  BEGIN
    CASE i OF
    | 1..j: (* nothing *)
    END;
  END P;

BEGIN
  P(1, 3)
END Case4.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
