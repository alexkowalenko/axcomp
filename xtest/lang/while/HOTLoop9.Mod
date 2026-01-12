MODULE HOTLoop9;

PROCEDURE Test* ();
  BEGIN
    WHILE TRUE DO
      EXIT
    END;
  END Test;

BEGIN
  Test
END HOTLoop9.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

