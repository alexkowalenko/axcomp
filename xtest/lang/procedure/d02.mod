MODULE d02; (* function definitions *)
  PROCEDURE f;
  BEGIN
      RETURN
  END f;
  
  PROCEDURE g : INTEGER;
  BEGIN
      RETURN 24
  END g;
 
  PROCEDURE h() : INTEGER;
  BEGIN
      RETURN 36
  END h;

BEGIN
    RETURN 0
END d02.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
