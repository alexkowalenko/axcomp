MODULE Halt4;

BEGIN
  HALT(-1);
END Halt4.
(*
RUN: %comp %s | filecheck %s
CHECK: 255
*)
