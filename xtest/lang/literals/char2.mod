(*
RUN: %comp %s | filecheck %s
CHECK: 18
*)

MODULE char2;
BEGIN
    RETURN 12X;
END char2.