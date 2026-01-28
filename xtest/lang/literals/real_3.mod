(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module real_3;
import Out;

begin
    Out.Real(5.3e5, 3); Out.Ln;
end real_3.

(*
CHECK: 5.3E+05
*)
