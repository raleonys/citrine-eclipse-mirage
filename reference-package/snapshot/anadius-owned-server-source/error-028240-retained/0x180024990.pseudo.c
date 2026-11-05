/* 0x180024990 FUN_180024990; Evidence ev_3a626f8100eb1bf7ba0b9677a33b1b1418e250dde536cb3a9f122c32790a3442 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 * FUN_180024990(undefined8 *param_1,undefined8 param_2,DWORD param_3,char param_4)

{
  LPVOID pvVar1;
  BOOL BVar2;
  DWORD DVar3;
  DWORDLONG dwlConditionMask;
  HANDLE pvVar4;
  longlong lVar5;
  undefined4 uVar6;
  undefined4 in_register_00000084;
  undefined1 auStack_128 [32];
  longlong local_108;
  undefined **local_100;
  undefined8 *local_f8;
  undefined **local_f0;
  undefined8 *local_e8;
  _OSVERSIONINFOEXA local_d8;
  ulonglong local_38;
  
  local_38 = DAT_1800a1580 ^ (ulonglong)auStack_128;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = param_2;
  param_1[4] = 0;
  *param_1 = asio::detail::win_iocp_io_context::vftable;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  local_d8.dwMinorVersion = 0;
  local_d8.dwBuildNumber = 0;
  local_d8.dwPlatformId = 0;
  local_d8.szCSDVersion[0] = '\0';
  local_d8.szCSDVersion[1] = '\0';
  local_d8.szCSDVersion[2] = '\0';
  local_d8.szCSDVersion[3] = '\0';
  local_d8.szCSDVersion[4] = '\0';
  local_d8.szCSDVersion[5] = '\0';
  local_d8.szCSDVersion[6] = '\0';
  local_d8.szCSDVersion[7] = '\0';
  local_d8.szCSDVersion[8] = '\0';
  local_d8.szCSDVersion[9] = '\0';
  local_d8.szCSDVersion[10] = '\0';
  local_d8.szCSDVersion[0xb] = '\0';
  local_d8.szCSDVersion[0xc] = '\0';
  local_d8.szCSDVersion[0xd] = '\0';
  local_d8.szCSDVersion[0xe] = '\0';
  local_d8.szCSDVersion[0xf] = '\0';
  local_d8.szCSDVersion[0x10] = '\0';
  local_d8.szCSDVersion[0x11] = '\0';
  local_d8.szCSDVersion[0x12] = '\0';
  local_d8.szCSDVersion[0x13] = '\0';
  local_d8.szCSDVersion[0x14] = '\0';
  local_d8.szCSDVersion[0x15] = '\0';
  local_d8.szCSDVersion[0x16] = '\0';
  local_d8.szCSDVersion[0x17] = '\0';
  local_d8.szCSDVersion[0x18] = '\0';
  local_d8.szCSDVersion[0x19] = '\0';
  local_d8.szCSDVersion[0x1a] = '\0';
  local_d8.szCSDVersion[0x1b] = '\0';
  local_d8.szCSDVersion[0x1c] = '\0';
  local_d8.szCSDVersion[0x1d] = '\0';
  local_d8.szCSDVersion[0x1e] = '\0';
  local_d8.szCSDVersion[0x1f] = '\0';
  local_d8.szCSDVersion[0x20] = '\0';
  local_d8.szCSDVersion[0x21] = '\0';
  local_d8.szCSDVersion[0x22] = '\0';
  local_d8.szCSDVersion[0x23] = '\0';
  local_d8.szCSDVersion[0x24] = '\0';
  local_d8.szCSDVersion[0x25] = '\0';
  local_d8.szCSDVersion[0x26] = '\0';
  local_d8.szCSDVersion[0x27] = '\0';
  local_d8.szCSDVersion[0x28] = '\0';
  local_d8.szCSDVersion[0x29] = '\0';
  local_d8.szCSDVersion[0x2a] = '\0';
  local_d8.szCSDVersion[0x2b] = '\0';
  local_d8.szCSDVersion[0x2c] = '\0';
  local_d8.szCSDVersion[0x2d] = '\0';
  local_d8.szCSDVersion[0x2e] = '\0';
  local_d8.szCSDVersion[0x2f] = '\0';
  local_d8.szCSDVersion[0x30] = '\0';
  local_d8.szCSDVersion[0x31] = '\0';
  local_d8.szCSDVersion[0x32] = '\0';
  local_d8.szCSDVersion[0x33] = '\0';
  local_d8.szCSDVersion[0x34] = '\0';
  local_d8.szCSDVersion[0x35] = '\0';
  local_d8.szCSDVersion[0x36] = '\0';
  local_d8.szCSDVersion[0x37] = '\0';
  local_d8.szCSDVersion[0x38] = '\0';
  local_d8.szCSDVersion[0x39] = '\0';
  local_d8.szCSDVersion[0x3a] = '\0';
  local_d8.szCSDVersion[0x3b] = '\0';
  local_d8.szCSDVersion[0x3c] = '\0';
  local_d8.szCSDVersion[0x3d] = '\0';
  local_d8.szCSDVersion[0x3e] = '\0';
  local_d8.szCSDVersion[0x3f] = '\0';
  local_d8.szCSDVersion[0x40] = '\0';
  local_d8.szCSDVersion[0x41] = '\0';
  local_d8.szCSDVersion[0x42] = '\0';
  local_d8.szCSDVersion[0x43] = '\0';
  local_d8.szCSDVersion[0x44] = '\0';
  local_d8.szCSDVersion[0x45] = '\0';
  local_d8.szCSDVersion[0x46] = '\0';
  local_d8.szCSDVersion[0x47] = '\0';
  local_d8.szCSDVersion[0x48] = '\0';
  local_d8.szCSDVersion[0x49] = '\0';
  local_d8.szCSDVersion[0x4a] = '\0';
  local_d8.szCSDVersion[0x4b] = '\0';
  local_d8.szCSDVersion[0x4c] = '\0';
  local_d8.szCSDVersion[0x4d] = '\0';
  local_d8.szCSDVersion[0x4e] = '\0';
  local_d8.szCSDVersion[0x4f] = '\0';
  local_d8.szCSDVersion[0x50] = '\0';
  local_d8.szCSDVersion[0x51] = '\0';
  local_d8.szCSDVersion[0x52] = '\0';
  local_d8.szCSDVersion[0x53] = '\0';
  local_d8.szCSDVersion[0x54] = '\0';
  local_d8.szCSDVersion[0x55] = '\0';
  local_d8.szCSDVersion[0x56] = '\0';
  local_d8.szCSDVersion[0x57] = '\0';
  local_d8.szCSDVersion[0x58] = '\0';
  local_d8.szCSDVersion[0x59] = '\0';
  local_d8.szCSDVersion[0x5a] = '\0';
  local_d8.szCSDVersion[0x5b] = '\0';
  local_d8.szCSDVersion[0x5c] = '\0';
  local_d8.szCSDVersion[0x5d] = '\0';
  local_d8.szCSDVersion[0x5e] = '\0';
  local_d8.szCSDVersion[0x5f] = '\0';
  local_d8.szCSDVersion[0x60] = '\0';
  local_d8.szCSDVersion[0x61] = '\0';
  local_d8.szCSDVersion[0x62] = '\0';
  local_d8.szCSDVersion[99] = '\0';
  local_d8.szCSDVersion[100] = '\0';
  local_d8.szCSDVersion[0x65] = '\0';
  local_d8.szCSDVersion[0x66] = '\0';
  local_d8.szCSDVersion[0x67] = '\0';
  local_d8.szCSDVersion[0x68] = '\0';
  local_d8.szCSDVersion[0x69] = '\0';
  local_d8.szCSDVersion[0x6a] = '\0';
  local_d8.szCSDVersion[0x6b] = '\0';
  local_d8.szCSDVersion[0x6c] = '\0';
  local_d8.szCSDVersion[0x6d] = '\0';
  local_d8.szCSDVersion[0x6e] = '\0';
  local_d8.szCSDVersion[0x6f] = '\0';
  local_d8.szCSDVersion[0x70] = '\0';
  local_d8.szCSDVersion[0x71] = '\0';
  local_d8.szCSDVersion[0x72] = '\0';
  local_d8.szCSDVersion[0x73] = '\0';
  local_d8.szCSDVersion[0x74] = '\0';
  local_d8.szCSDVersion[0x75] = '\0';
  local_d8.szCSDVersion[0x76] = '\0';
  local_d8.szCSDVersion[0x77] = '\0';
  local_d8.szCSDVersion[0x78] = '\0';
  local_d8.szCSDVersion[0x79] = '\0';
  local_d8.szCSDVersion[0x7a] = '\0';
  local_d8.szCSDVersion[0x7b] = '\0';
  local_d8.szCSDVersion[0x7c] = '\0';
  local_d8.szCSDVersion[0x7d] = '\0';
  local_d8.szCSDVersion[0x7e] = '\0';
  local_d8.szCSDVersion[0x7f] = '\0';
  local_d8.wServicePackMajor = 0;
  local_d8.wServicePackMinor = 0;
  local_d8.wSuiteMask = 0;
  local_d8.wProductType = '\0';
  local_d8.wReserved = '\0';
  local_d8.dwOSVersionInfoSize = 0x9c;
  local_d8.dwMajorVersion = 6;
  local_e8 = param_1;
  dwlConditionMask =
       VerSetConditionMask(0,2,CONCAT71((int7)(CONCAT44(in_register_00000084,param_3) >> 8),3));
  BVar2 = VerifyVersionInfoA(&local_d8,2,dwlConditionMask);
  uVar6 = 0xffffffff;
  if (BVar2 == 0) {
    uVar6 = 500;
  }
  *(undefined4 *)(param_1 + 8) = uVar6;
  param_1[9] = 0;
  param_1[10] = 0;
  *(undefined4 *)(param_1 + 0xb) = 0;
  DVar3 = FUN_180027be0((LPCRITICAL_SECTION)(param_1 + 0xc));
  local_f0 = FUN_180028620();
  local_f8 = (undefined8 *)CONCAT44(local_f8._4_4_,DVar3);
  if (DVar3 == 0) {
    param_1[0x11] = 0;
    param_1[0x12] = 0;
    param_1[0x13] = 0;
    *(DWORD *)(param_1 + 0x14) = param_3;
    param_1[0x15] = 0;
    DVar3 = 0xffffffff;
    if (-1 < (int)param_3) {
      DVar3 = param_3;
    }
    pvVar4 = CreateIoCompletionPort((HANDLE)0xffffffffffffffff,(HANDLE)0x0,0,DVar3);
    param_1[5] = pvVar4;
    if (pvVar4 == (HANDLE)0x0) {
      DVar3 = GetLastError();
      local_100 = FUN_180028620();
      local_108 = CONCAT44(local_108._4_4_,DVar3);
      if (DVar3 != 0) {
                    /* WARNING: Subroutine does not return */
        FUN_180028240(&local_108,(undefined8 *)&DAT_180084528);
      }
    }
    if (param_4 != '\0') {
      LOCK();
      *(int *)(param_1 + 6) = *(int *)(param_1 + 6) + 1;
      UNLOCK();
      lVar5 = FUN_18004b0bc(0x18);
      local_108 = lVar5;
      if (lVar5 == 0) {
        lVar5 = 0;
      }
      else {
        *(undefined8 *)(lVar5 + 8) = 0;
        *(undefined8 *)(lVar5 + 0x10) = 0;
        local_f8 = (undefined8 *)FUN_18004b0bc(0x20);
        if (local_f8 == (undefined8 *)0x0) {
          FUN_18002a000(lVar5,(undefined8 *)0x0,0);
        }
        else {
          *local_f8 = asio::detail::win_thread::
                      func<asio::detail::win_iocp_io_context::thread_function>::vftable;
          local_f8[3] = param_1;
          FUN_18002a000(lVar5,local_f8,0);
        }
      }
      pvVar1 = (LPVOID)param_1[0x15];
      if (pvVar1 != (LPVOID)0x0) {
        CloseHandle(*(HANDLE *)((longlong)pvVar1 + 8));
        thunk_FUN_18005caec(pvVar1);
      }
      param_1[0x15] = lVar5;
    }
    return param_1;
  }
                    /* WARNING: Subroutine does not return */
  FUN_180028240(&local_f8,(undefined8 *)"mutex");
}



