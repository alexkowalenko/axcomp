MODULE NestedProc1;

IMPORT Out;
  
PROCEDURE Test* ();
  PROCEDURE Inner ();
    BEGIN
      Out.String ("Hello World"); Out.Ln;
    END Inner;
  
  BEGIN
    Inner;
  END Test;

BEGIN
  Test
END NestedProc1.

(*
RUN: %comp %s | filecheck %s
CHECK: Hello World
CHECK-NEXT: 0
*)
