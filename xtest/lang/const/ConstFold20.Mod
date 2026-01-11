(*
RUN: %comp %s | filecheck %s
CHECK: TRUE
CHECK-NEXT: 0
*)

MODULE ConstFold20;
IMPORT Out;
BEGIN
  IF {1} = {1} THEN
    Out.String("TRUE"); Out.Ln;
  END;
END ConstFold20.
