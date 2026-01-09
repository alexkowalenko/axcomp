(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE bool2;
BEGIN
    RETURN FALSE;
END bool2.