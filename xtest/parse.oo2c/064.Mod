(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
TYPE T = RECORD a: CHAR; b: INTEGER; END;
END M.
