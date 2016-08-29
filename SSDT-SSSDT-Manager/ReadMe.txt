1.send Io Control Code to Ring0 to get SSDT&SSSDT Information
include:functionIndex/Function Name/Current Address/Original Address/Is Hooked/the module belong
include:reload ntoskrnl.exe(SSDT)/win32k.sys(SSSDT) to compare

2.if current Address is not compare to  Original Address means the function is SSDT Hook
  if the code is not compare to the code in ntoskrnl.exe/win32k.sys in the first 32 bytes in the function,it is Inline Hook
  
3.Resume SSDT Hook/Inline Hook
