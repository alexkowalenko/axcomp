MODULE d01; (* function definitions *)
  PROCEDURE f: INTEGER;
  BEGIN
      RETURN 12
  END f;
BEGIN
    RETURN 0
END d01.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
