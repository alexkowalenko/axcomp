MODULE string11; (* STRING comparison*)
IMPORT Out, Strings;
CONST a = "alpha";
      b = "beta";

PROCEDURE print(s1, s2: STRING);
BEGIN
    Out.String(s1); Out.Char(' '); Out.String(s2); Out.Char(' ');
    Out.Bool(s1 = s2);  Out.Char(' ');
    Out.Bool(s1 # s2);  Out.Char(' ');
    Out.Bool(s1 < s2);  Out.Char(' ');
    Out.Bool(s1 <= s2);  Out.Char(' ');
    Out.Bool(s1 > s2);  Out.Char(' ');
    Out.Bool(s1 >= s2);  Out.Char(' ');
    Out.Ln;
END print;

BEGIN
    print(a, a);
    print(a, b);
    print(b, a);
    print(b, b);
    RETURN 0
END string11.

(*
RUN: %comp %s | filecheck %s
CHECK:alpha alpha 1 0 0 1 0 1
CHECK-NEXT: alpha beta 0 1 1 1 0 0
CHECK-NEXT: beta alpha 0 1 0 0 1 1
CHECK-NEXT: beta beta 1 0 0 1 0 1
CHECK-NEXT: 0
*)