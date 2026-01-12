MODULE Return3;

PROCEDURE Test*(): SHORTINT;
  BEGIN
    RETURN 128;
  END Test;
  
END Return3.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
