(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module real_2;
import Out;

begin
    Out.Real(1.2E-2, 3); Out.Ln;
end real_2.

(*
CHECK: 0.012
*)
