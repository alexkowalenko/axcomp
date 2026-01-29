
(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN+ *>

module caser_3;
var x: INTEGER;
begin
    x := 7;
    case x of
      1: WriteInt(1); WriteLn;
      | 2: WriteInt(2); WriteLn;
      | 3: WriteInt(3); WriteLn;
      | 4..5:  WriteInt(4); WriteLn;
    else
       WriteInt(999); WriteLn;
    end;
end caser_3.


(*
CHECK: 999
*)


