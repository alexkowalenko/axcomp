MODULE record13; (* records *)
TYPE 
    List = RECORD
        value : INTEGER;
        next : INTEGER;
    END;
VAR current : List;
BEGIN
    current.value := 5;
    RETURN current.value;
END record13.