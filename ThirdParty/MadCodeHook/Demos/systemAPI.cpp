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

#include <windows.h>
#include "madCodeHookLib.h"

// variable for the "next hook", which we then call in the callback function
// it must have *exactly* the same parameters and calling convention as the
// original function
// besides, it's also the parameter that you need to undo the code hook again
BOOL (WINAPI *CreateProcessNextHook)(LPCTSTR               lpApplicationName,
                                     LPTSTR                lpCommandLine,
                                     LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                     LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                     BOOL                  bInheritHandles,
                                     DWORD                 dwCreationFlags,
                                     LPVOID                lpEnvironment,
                                     LPCTSTR               lpCurrentDirectory,
                                     LPSTARTUPINFO         lpStartupInfo,
                                     LPPROCESS_INFORMATION lpProcessInformation);

// this function is our hook callback function, which will (system wide!) receive
// all calls to the original CreateProcess function, as soon as we've hooked it
// the hook function must have *exactly* the same parameters and calling
// convention as the original function
BOOL WINAPI CreateProcessHookProc(LPCTSTR               lpApplicationName,
                                  LPTSTR                lpCommandLine,
                                  LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                  LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                  BOOL                  bInheritHandles,
                                  DWORD                 dwCreationFlags,
                                  LPVOID                lpEnvironment,
                                  LPCTSTR               lpCurrentDirectory,
                                  LPSTARTUPINFO         lpStartupInfo,
                                  LPPROCESS_INFORMATION lpProcessInformation)
{
  // this function will be called from several processes, so we can't use
  // any variables, functions or constants that are only accessible in our own process
  // so we have to set up our temporary string byte per byte, painful...
  char question[9];

  question[0] = 'E';
  question[1] = 'x';
  question[2] = 'e';
  question[3] = 'c';
  question[4] = 'u';
  question[5] = 't';
  question[6] = 'e';
  question[7] = '?';
  question[8] = 0;
  if (MessageBox(0, lpCommandLine, question, MB_YESNO | MB_ICONQUESTION) == IDYES)
  {  
    return CreateProcessNextHook(lpApplicationName, lpCommandLine,
		                         lpProcessAttributes, lpThreadAttributes,
								 bInheritHandles, dwCreationFlags,
								 lpEnvironment, lpCurrentDirectory,
								 lpStartupInfo, lpProcessInformation);
  } else {
    SetLastError(ERROR_ACCESS_DENIED);
	return false;
  }
}

int pascal WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // we install our hook on the API with special parameters (= system wide)...
  // alternatively to the call below you can also use this one:
  // HookAPI('kernel32.dll', 'CreateProcessA', @CreateProcessHookProc, @CreateProcessNextHook);
  HookCode(CreateProcess, CreateProcessHookProc, (PVOID*) &CreateProcessNextHook, NULL, NULL, TRUE);
  // now let's show a message box
  // while this message box is shown, you can start programs from the shell
  // to check whether our system wide hook is *really* system wide
  MessageBox(0, "Hook installed!", "info", MB_OK | MB_ICONINFORMATION);
  // now we can decide: either we unhook again or not
  // if we don't the hook remains installed (and keeps working!!) even after
  // our program has terminated
  // but in this demo we want to unhook again, so let's do it:
  UnhookCode((PVOID*) &CreateProcessNextHook);

  return 0;
}
