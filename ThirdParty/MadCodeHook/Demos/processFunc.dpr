// demonstrate how madCodeHook can hook (almost) any function under any win32 OS
// a madCodeHook is normally only process wide
// look at the systemAPI demo for infos about system wide hooks
// the to-be-hooked function must fulfill 2 rules to be hook-able by madCodeHook
// (1) the asm code it must be at least 6 bytes long
// (2) there must not be a jump into the 2-6th byte anywhere in the code
// if these rules are not fulfilled the hook is not installed
// because otherwise we would risk "wild" crashes

program processFunc;

uses Windows, madCodeHookLib;

// SomeFunc appends the 2 string parameters and returns the result
function SomeFunc(str1, str2: string) : string;
begin
  result := str1 + str2;
end;

// variable for the "next hook", which we then call in the callback function
// it must have *exactly* the same parameters and calling convention as the
// original function
// besides, it's also the parameter that you need to undo the code hook again
var SomeFuncNextHook : function (str1, str2: string) : string;

// this function is our hook callback function, which will receive
// all calls to the original SomeFunc function, as soon as we've hooked it
// the hook function must have *exactly* the same parameters and calling
// convention as the original function
function SomeFuncHookProc(str1, str2: string) : string;
var i1 : integer;
begin
  // manipulate the input parameters
  str1 := 'blabla';
  for i1 := 1 to Length(str2) do  // strupr
    if str2[i1] in ['a'..'z', 'ü', 'ö', 'ä'] then
      str2[i1] := chr(byte(str2[i1]) - 32);
  // now call the original function
  result := SomeFuncNextHook(str1, str2);
  // now we can manipulate the result
  Delete(result, 1, 3);
end;

begin
  // call the original unhooked function and display the result
  MessageBox(0, pchar(SomeFunc('str1', 'str2')), '"str1" + "str2"', 0);
  // now we install our hook on the function ...
  HookCode(@SomeFunc, @SomeFuncHookProc, @SomeFuncNextHook);
  // now call the original (but in the meanwhile hooked) function again
  // the displayed result will be different because in our hook function
  // we manipulated the input parameters (and also the result)
  MessageBox(0, pchar(SomeFunc('str1', 'str2')), '"str1" + "str2"', 0);
  // unhook again, not really needed here, but - hey - it can't do any harm  :-)
  UnhookCode(@SomeFuncNextHook);
end.
