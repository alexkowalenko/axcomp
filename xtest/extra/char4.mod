MODULE char4;
TYPE characterArray = ARRAY 3 OF CHAR;
VAR x : characterArray;
BEGIN
    x[0] := 'ξ';
    x[1] := '四';
    x[2] := '👾';
    RETURN x[2]
END char4.

(*
RUN: %comp %s | filecheck %s
CHECK: 126
*)