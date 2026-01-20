MODULE char3;
TYPE characterArray = ARRAY 3 OF CHAR;
VAR x : characterArray;
BEGIN
    x[0] := 'A';
    RETURN x[0]
END char3.


(*
RUN: %comp %s | filecheck %s
CHECK: 65
*)