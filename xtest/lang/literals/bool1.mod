(*
RUN: %comp %s | filecheck %s
CHECK: 1
*)

MODULE bool1;
BEGIN
    RETURN TRUE;
END bool1.