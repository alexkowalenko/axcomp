MODULE Case5;

PROCEDURE P (i: SHORTINT);
  BEGIN
    CASE i OF
    | 128: (* nothing *)
    END;
  END P;

BEGIN
  P(2)
END Case5.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
