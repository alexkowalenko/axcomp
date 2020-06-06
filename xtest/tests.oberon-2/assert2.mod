MODULE assert2; (* ASSERT *)
VAR x: INTEGER;
BEGIN
    ASSERT(x = 0, 3);
    ASSERT(x # 0, 2);
END assert2.