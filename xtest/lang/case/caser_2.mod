(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN+ *>

module caser_2;
var x: INTEGER;
begin
    x := 3;
    case x of
      1: WriteInt(1); WriteLn;
      | 2: WriteInt(2); WriteLn;
      | 5,3: WriteInt(3); WriteLn;
      | 4:  WriteInt(4); WriteLn;
    end;
end caser_2.

(*
CHECK: 3
*)

