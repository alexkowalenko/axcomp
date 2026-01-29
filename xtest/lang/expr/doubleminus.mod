(*
RUN: %comp %s | filecheck %s
*)

module doubleminus;
begin
    return -(-10);
end doubleminus.

(*
CHECK: 10
*)
