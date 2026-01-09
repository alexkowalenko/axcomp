(*
RUN: %comp %s | filecheck %s
CHECK: 65
*)

MODULE char2;
VAR x : CHAR;
BEGIN
    x := 'A';
    RETURN x
END char2.