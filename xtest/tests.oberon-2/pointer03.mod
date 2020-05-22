MODULE pointer03; (* pointers *)
IMPORT Out;
TYPE 
    ListPtr = POINTER TO List;
    List = RECORD
        value : INTEGER;
        (* next : ListPtr; (* recursive pointers *)  *)
    END;
VAR start : ListPtr;
BEGIN
    start := NIL;
    NEW(start);    
    start^.value := 5;
    Out.Int(start^.value); Out.Ln;
    RETURN 0;
END pointer03.