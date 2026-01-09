(*
RUN: %comp %s | filecheck %s
CHECK: 25
*)

(* parentheses *)
MODULE b10;
BEGIN
    RETURN (3 * 3) + ((2+ (1+1)) * 4)
END b10.