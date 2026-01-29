(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_real_3;

import Out;

begin
    Out.Real( 1.2 * 3.1415927, 6); Out.Ln;
end binary_real_3.

(*
CHECK: 3.76991
*)
