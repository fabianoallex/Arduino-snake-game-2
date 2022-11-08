MODE COM6 BAUD=9600 PARITY=n DATA=8

:LOOP
  
  CHOICE /C:1235 /M "1: liga; 2: desliga; 3: pisca; 4: sair " 
  IF errorlevel 4 GOTO TOP
  IF errorlevel 3 GOTO RIGHT
  IF errorlevel 2 GOTO BOTTOM
  IF errorlevel 1 GOTO LEFT

  :RIGHT  
  ECHO b > COM6 
  GOTO END

  :LEFT
  ECHO a > COM6  
  GOTO END

  :TOP
  ECHO c > COM6  
  GOTO END

  :BOTTOM
  ECHO d > COM6  
  
  :END
  CLS
GOTO LOOP 

:SAIR
