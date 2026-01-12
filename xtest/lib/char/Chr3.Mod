MODULE Chr3;

PROCEDURE Test;
  VAR
    x: CHAR;
  BEGIN
    x := CHR(256);
  END Test;

BEGIN
  Test;
END Chr3.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
