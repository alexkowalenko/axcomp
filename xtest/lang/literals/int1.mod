(*
RUN: %comp %s | filecheck %s
CHECK: 7
*)

MODULE int1;
BEGIN
    RETURN 7;
END int1.