(*
RUN: %comp %s | filecheck %s
CHECK: A
CHECK: 四
CHECK: ξ
CHECK: 👾
*)

<* MAIN+ *>

MODULE proc_char_1;
IMPORT Out;
CONST
    A = '四';
    xi = 'ξ';
    invader = '👾';
VAR
    c : CHAR;

PROCEDURE Write(c: CHAR);
BEGIN
     Out.Char(c); Out.Ln;
END Write;


BEGIN
    c := 'A';
    Write(c);
    Write(A); 
    Write(xi); 
    Write(invader);
END proc_char_1.
