(*
RUN: %comp %s | filecheck %s
CHECK: 97
*)

MODULE char5;
VAR x : CHAR;

BEGIN
       x := 'a';
       IF x = 'a' THEN
              RETURN x
       END
       RETURN 0
END char5.