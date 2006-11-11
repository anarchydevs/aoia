// demonstrate how to use CopyFunction, AllocMemEx and CreateRemoteThread to
// execute a function in the context of another process

// this demo shows us the command line of the explorer, which normally is not
// possible when using normal win32 APIs

program RemoteCmdLine;

uses Windows, madRemote;

// this is our thread function, which will be copied to and then executed in the
// context of any desired process
function GetCmdLineThread(buffer: pchar) : integer; stdcall;
var cl : pchar;
begin
  // first let's get the command line of the current process
  cl := GetCommandLine;

  // now let's copy the characters to the specified buffer
  // we can't use StrPCopy, because that's not available in other processes
  // we use "result" as the loop variable, so the length of the copied
  // command line is automatically the exit code of the thread
  for result := 0 to MAX_PATH - 1 do begin
    buffer[result] := cl[result];
    if buffer[result] = #0 then
      break;
  end;
end;

// this function can give us the command line of any specified 32bit process
function GetProcessCmdLine(processHandle: dword) : string;
var th, tid    : dword;
    entryPoint : pointer;
    buffer     : pointer;
    params     : pointer;
    len        : dword;
begin
  // first of all we copy the function to the specified process
  entryPoint := CopyFunction(@GetCmdLineThread, processHandle, false, @buffer);

  // now we allocate a temporare buffer in the context of the specified process
  params := AllocMemEx(MAX_PATH, processHandle);

  // finally we create the remote thread and give in the entryPoint of the
  // copied fuction and the temporare buffer as the parameter
  // both addresses are valid in the context of the destination process
  th := CreateRemoteThread(processHandle, nil, 0, entryPoint, params, 0, tid);

  // let's wait until the remote thread has done it's work
  WaitForSingleObject(th, INFINITE);

  // the exit code of the remote thread is the length of the command line
  GetExitCodeThread(th, len);
  SetLength(result, len);

  // finally let's fill the result string from the temporare buffer
  ReadProcessMemory(processHandle, params, pointer(result), len, len);

  // never forget to close handles
  CloseHandle(th);

  // also don't forget to free whatever we allocated in the other process
  FreeMemEx(params, processHandle);
  FreeMemEx(buffer, processHandle);
end;

// now we demonstrate the functionality by showing the explorer's command line
var wnd, pid, ph : dword;
begin
  // first we ask the taskbar's window handle and the corresponding process ID
  wnd := FindWindow('Shell_TrayWnd', '');
  GetWindowThreadProcessID(wnd, @pid);

  // then we open the process, which is the explorer, of course
  ph := OpenProcess(PROCESS_ALL_ACCESS, false, pid);

  // and finally show it's command line
  MessageBox(0, pchar('"' + GetProcessCmdLine(ph) + '"'),
             'explorer''s command line...', MB_ICONINFORMATION);

  // again: don't forget to close the handles
  CloseHandle(ph);
end.