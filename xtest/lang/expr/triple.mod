(*
RUN: %comp %s | filecheck %s
*)

module triple;
begin
    return -(+(-10));
end triple.

(*
CHECK: 10
*)
