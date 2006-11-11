// this demo works only in win9x (system wide)

// demonstrate how madCodeHook can hook "shared APIs" under win9x system wide

// the method used in this demo works only in win9x and only for "shared APIs"
// shared APIs are all exported APIs of dlls whose module handle is >= $80000000

// in winNT you can hook APIs only process wide
// if you want to have system wide API hooking there you must inject code into
// every running process and hook the API in the context of each process seperately
// injecting code into every running process is another (difficult) story
// which can be solved by different methods, e.g.
// - injecting a little dll into all GUI processes by using SetWindowsHookEx
//   (this way you will miss some processes)
// - using VirtualAllocEx + WriteProcessMemory + CreateRemoteThread

program systemAPI;

uses Windows, madCodeHookLib;

// variable for the "next hook", which we then call in the callback function
// it must have *exactly* the same parameters and calling convention as the
// original function
// besides, it's also the parameter that you need to undo the code hook again
var CreateProcessNextHook : function (applicationName   : pchar;
                                      commandLine       : pchar;
                                      processAttr       : PSecurityAttributes;
                                      threadAttr        : PSecurityAttributes;
                                      inheritHandles    : bool;
                                      creationFlags     : dword;
                                      environment       : pointer;
                                      currentDirectory  : pchar;
                                      const startupInfo : TStartupInfo;
                                      var processInfo   : TProcessInformation) : bool; stdcall;

// this function is our hook callback function, which will (system wide!) receive
// all calls to the original CreateProcess API, as soon as we've hooked it
// the hook function must have *exactly* the same parameters and calling
// convention as the original API
function CreateProcessHookProc(applicationName   : pchar;
                               commandLine       : pchar;
                               processAttr       : PSecurityAttributes;
                               threadAttr        : PSecurityAttributes;
                               inheritHandles    : bool;
                               creationFlags     : dword;
                               environment       : pointer;
                               currentDirectory  : pchar;
                               const startupInfo : TStartupInfo;
                               var processInfo   : TProcessInformation) : bool; stdcall;
// sometimes the compiled Delphi code is not clean enough for being copied to shared memory
// e.g. Delphi sometimes doesn't restore all registers 100% correctly
// so since system wide hooking is quite dangerous we better write assembler code
// to have full power over what registers are used and restored
// a bit ugly, but - hey - after all system wide API hooking is a complicated task...
asm
  // this function will be called from several processes, so we can't use
  // any variables, functions or constants that are only accessible in our own process
  // so we're reserving space on the stack to build up a string, painful...
  add esp, -$14
  mov byte ptr [ebp - $14], 'E'
  mov byte ptr [ebp - $13], 'x'
  mov byte ptr [ebp - $12], 'e'
  mov byte ptr [ebp - $11], 'c'
  mov byte ptr [ebp - $10], 'u'
  mov byte ptr [ebp - $0F], 't'
  mov byte ptr [ebp - $0E], 'e'
  mov byte ptr [ebp - $0D], '?'
  mov byte ptr [ebp - $0C], 0
  lea eax, [ebp - $14]
  // now we set up the parameters for MessageBox...
  push MB_YESNO or MB_ICONQUESTION
  push eax
  push commandLine
  push 0
  // ... and call it
  // CAUTION!! Normally you should really not do this!!
  // this function will be called from every process that calls CreateProcess
  // some processes might have imported kernel32.dll, but NOT user32.dll
  // and in such a process we can't call MessageBox, because it's in user32.dll
  // as a result that means, you should only call APIs of kernel32.dll in your
  // callback function, because only this dll is loaded in really *every* process
  // but for our demo we are ignorant and do what we should not do namely call a
  // non-kernel32.dll API
  call MessageBox
  cmp eax, IDYES
  jz @@callNextHook
  // well, the user decided that the CreateProcess call should not be executed
  // so we need to set up a correct LastError value:
  mov eax, ERROR_ACCESS_DENIED
  push eax
  call SetLastError
  // and return "false" as the function result (in EAX):
  mov eax, 0
  jmp @@end
 @@callNextHook:
   // the user decided to let the CreateProcess call happen, so we do it:
  push processInfo
  push startupInfo
  push currentDirectory
  push environment
  push creationFlags
  push inheritHandles
  push threadAttr
  push processAttr
  push commandLine
  push applicationName
  // if we would call CreateProcess here, we would end up calling ourselves again
  // so we call "CreateProcessNextHook" of course
  call CreateProcessNextHook
 @@end:
  mov esp, ebp
end;

begin
  // we install our hook on the API with special parameters (= system wide)...
  // alternatively to the call below you can also use one of the following:
  // HookAPI(kernel32, 'CreateProcessA', @CreateProcessHookProc, @CreateProcessNextHook);
  // HookAPI(GetProcAddress(GetModuleHandle(kernel32), 'CreateProcessA'), 
  //         @CreateProcessHookProc, @CreateProcessNextHook);
  HookCode(@CreateProcess, @CreateProcessHookProc, @CreateProcessNextHook, nil, nil, true);
  // now let's show a message box
  // while this message box is shown, you can start programs from the shell
  // to check whether our system wide hook is *really* system wide
  MessageBox(0, 'Hook installed!', 'info', MB_OK or MB_ICONINFORMATION);
  // now we can decide: either we unhook again or not
  // if we don't the hook remains installed (and keeps working!!) even after
  // our program has terminated
  // but in this demo we want to unhook again, so let's do it:
  UnhookCode(@CreateProcessNextHook);
end.
