(*
RUN:  %comp %s | filecheck %s
*)

<* MAIN+ *>

module case_9;
var x: INTEGER;
begin
    x := 10;
    case x of
      1: WriteInt(1); WriteLn;
      | 3..7+1:  WriteInt(4); WriteLn;
      | 10 : WriteInt(10); WriteLn;
    else
      WriteInt(999); WriteLn;
    end;
end case_9.

(*
CHECK: 10
*)

