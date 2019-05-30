# Windows-Rootkits
## CheckKernelEATHook
Reference [3600safeOpenSource](https://bbs.pediy.com/thread-150599.htm)

## GetKernel32Addressx64
x64 use asm

## HideProcess
HideProcess by Remove ProcessList in EPROCESS struct.

## HookDeviceIocontrlFile
Reference [IOCTL Fuzzer v1.2](https://github.com/Cr4sh/ioctlfuzzer/tree/master/src)

## Inject
CreateRemoteThread to Inject
[Process-Hollowing](https://github.com/m0n0ph1/Process-Hollowing)<br> 
[ReflectiveDLLInjection](https://github.com/stephenfewer/ReflectiveDLLInjection)<br> 
SetThreadContext to Inject support x64 and x86<br> 
SetWindowsHookEx to Inject<br> 
UserApcInject

## LoadImageCallBack
Scan PE's IAT in PsSetLoadImageNotifyRoutine's callback

## ProcessManager
Enum Process By PsLookupProcessByProcessId/travel Active List/PspCidTable<br> 
Hdie Process By Process Active List/PspCidTable<br> 
Monitor Process CreateInformation By PsSetCreateProcessNotifyRoutineEx<br> 

## ProtectFilex64
Protect File in Windows 7 by ObRegisterCallbacks

## ProtectFilex86
Hook NtSetInformationFile to change target file<br> 
Hook NtWriteFile to write the target file<br> 
Hook NtDeleteFile to delete the target file<br> 
bind keyboard Filter Driver to avoid "ctrl+c" copy the content

## ProtectProcessx64
Protect Process in Windows 7 by ObRegisterCallbacks

## ReloadKernel-XP
Reference [3600safeOpenSource](https://bbs.pediy.com/thread-150599.htm)

## SSDT-SSSDT-Manager
Check SSDT/ShadowSSDT Hook/InlineHook<br> 
Resume SSDT/ShadowSSDT Hook/InlineHook

## ZwQueryVirtualMemory
Enum Module By NtQueryVirtualMemory
