(*
RUN: %comp %s | filecheck %s
*)

module parens2;
begin
    return (-(17));
end parens2.

(* -17
CHECK: 239
*)
