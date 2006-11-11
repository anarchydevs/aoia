// demonstrate how madCodeHook can hook (almost) any API under any win32 OS
// a madCodeHook is normally only process wide
// look at the systemAPI demo for infos about system wide hooks
// (note, that you can even hook so-called shared system APIs under win9x)

#include <windows.h>
#include "madCodeHookLib.h"

// variable for the "next hook", which we then call in the callback function
// it must have *exactly* the same parameters and calling convention as the
// original function
// besides, it's also the parameter that you need to undo the code hook again
UINT (WINAPI *WinExecNextHook)(LPCSTR lpCmdLine, UINT uCmdShow);

// this function is our hook callback function, which will receive
// all calls to the original SomeFunc function, as soon as we've hooked it
// the hook function must have *exactly* the same parameters and calling
// convention as the original function
UINT WINAPI WinExecHookProc(LPCSTR lpCmdLine, UINT uCmdShow)
{
  UINT result;

  // check the input parameters and ask whether the call should be executed
  if (MessageBox(0, lpCmdLine, "Execute?", MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
    // now call the original function, but in minimized form (just for fun :-)
    result = WinExecNextHook(lpCmdLine, SW_SHOWMINIMIZED);
  } else
    // if we didn't execute the call, we should at least return a valid value
    result = ERROR_FILE_NOT_FOUND;
  return result;
}

int pascal WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // we install our hook on the API...
  // alternatively to the call below you can also use this one:
  // HookAPI('kernel32.dll', 'WinExec', @WinExecHookProc, @WinExecNextHook);
  HookCode(WinExec, WinExecHookProc, (PVOID*) &WinExecNextHook);
  // now call the original (but hooked) API
  // as a result of the hook the user will receive our messageBox etc
  WinExec("notepad.exe", SW_SHOWNORMAL);
  // *PLEASE* be cautious when you hook APIs in win9x that are in the shared area
  // e.g. kernel32.dll and user32.dll are in the shared area
  // each dll with GetModuleHandle >= $80000000 is in the shared area
  // with madCodeHook you can hook such "shared APIs" like any other
  // but if you don't unhook them, rests of your hooks will remain installed
  // even after your application closes
  // that doesn't impact system stability, but it's not good for performance
  // (under winNT/2000 you don't need to care about unhooking)
  UnhookCode((PVOID*) &WinExecNextHook);

  return 0;
}
