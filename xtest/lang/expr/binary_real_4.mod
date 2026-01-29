(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_real_4;

import Out;

begin
    Out.Real(  3.1415927 / 1.2, 6); Out.Ln;
end binary_real_4.

(*
CHECK: 2.61799
*)
