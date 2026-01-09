(*
RUN: %comp %s | filecheck %s
CHECK: 60
*)

MODULE c09; (* TYPE declarations *)

 TYPE time = INTEGER;
      spin = BOOLEAN;
 VAR seconds : time;
     orientation : spin;
    
BEGIN
      seconds := 60;
      orientation := TRUE;
      RETURN seconds
END c09.