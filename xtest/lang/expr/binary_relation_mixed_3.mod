(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_relation_mixed_3;

begin
     WriteBoolean(  1.0 = 1); WriteLn;
     WriteBoolean(  1 # 1.0); WriteLn;
     WriteBoolean(  1.0 > 1); WriteLn;
     WriteBoolean(  1 >= 1.0); WriteLn;
     WriteBoolean(  1.0 < 1); WriteLn;
     WriteBoolean(  1 <= 1.0); WriteLn;
end binary_relation_mixed_3.

(*
CHECK: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
*)
