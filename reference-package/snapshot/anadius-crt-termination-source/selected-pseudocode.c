/* 0x18005cb00 operator()<>; Evidence ev_420a1e96d05180f4259482d3554df390f84cdf4ea4fc9173d88795435d1127a8 */

/* Library Function - Multiple Matches With Same Base Name
    public: void __cdecl __crt_seh_guarded_call<void>::operator()<class
   <lambda_99476a1ad63dd22509b5d3e65b0ffc95>,class <lambda_ad1ced32f4ac17aa236e5ef05d6b3b7c> &
   __ptr64,class <lambda_f7424dd8d45958661754dc4f2697e9c3> >(class
   <lambda_99476a1ad63dd22509b5d3e65b0ffc95> && __ptr64,class
   <lambda_ad1ced32f4ac17aa236e5ef05d6b3b7c> & __ptr64,class
   <lambda_f7424dd8d45958661754dc4f2697e9c3> && __ptr64) __ptr64
    public: void __cdecl __crt_seh_guarded_call<void>::operator()<class
   <lambda_d80eeec6fff315bfe5c115232f3240e3>,class <lambda_6e4b09c48022b2350581041d5f6b0c4c> &
   __ptr64,class <lambda_2358e3775559c9db80273638284d5e45> >(class
   <lambda_d80eeec6fff315bfe5c115232f3240e3> && __ptr64,class
   <lambda_6e4b09c48022b2350581041d5f6b0c4c> & __ptr64,class
   <lambda_2358e3775559c9db80273638284d5e45> && __ptr64) __ptr64
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void operator()<>(undefined8 param_1,int *param_2,undefined8 *param_3,int *param_4)

{
  __acrt_lock(*param_2);
  FUN_18005cb38(param_3);
  __acrt_unlock(*param_4);
  return;
}

/* 0x18005cb38 FUN_18005cb38; Evidence ev_d1e199fcbb28ad599e5d05dd9e99b003e173af04f4f4cff1ffbf682311bcd055 */

/* WARNING: Function: _guard_dispatch_icall replaced with injection: guard_dispatch_icall */

void FUN_18005cb38(undefined8 *param_1)

{
  byte bVar1;
  undefined *puVar2;
  
  if (DAT_1800a4f18 != '\0') {
    return;
  }
  LOCK();
  DAT_1800a4f08 = 1;
  UNLOCK();
  if (*(int *)*param_1 == 0) {
    if (DAT_1800a4f10 != DAT_1800a1580) {
      bVar1 = (byte)DAT_1800a1580 & 0x3f;
      (*(code *)((ulonglong)(DAT_1800a1580 ^ DAT_1800a4f10) >> bVar1 |
                (DAT_1800a1580 ^ DAT_1800a4f10) << 0x40 - bVar1))(0,0,0);
    }
    puVar2 = &DAT_1800a52a8;
  }
  else {
    if (*(int *)*param_1 != 1) goto LAB_18005cbb2;
    puVar2 = &DAT_1800a52c0;
  }
  FUN_1800643b4(puVar2);
LAB_18005cbb2:
  if (*(int *)*param_1 == 0) {
    FUN_180064614((undefined8 *)&DAT_18007f728,(undefined8 *)&DAT_18007f748);
  }
  FUN_180064614((undefined8 *)&DAT_18007f750,(undefined8 *)&DAT_18007f758);
  if (*(int *)param_1[1] == 0) {
    DAT_1800a4f18 = '\x01';
    *(undefined1 *)param_1[2] = 1;
  }
  return;
}

/* 0x18005cc04 FUN_18005cc04; Evidence ev_7a349832acd7de0edd5058b95ded2252a6787a6807b783c7bd1f999bc4951033 */

void FUN_18005cc04(UINT param_1,undefined4 param_2,int param_3)

{
  code *pcVar1;
  HMODULE pHVar2;
  int *piVar3;
  undefined4 local_res10 [2];
  int local_res18 [2];
  undefined1 local_res20 [8];
  undefined1 local_38 [4];
  int local_34 [3];
  undefined8 local_28;
  undefined4 *local_20;
  int *local_18;
  undefined1 *local_10;
  
  local_28 = 0xfffffffffffffffe;
  local_res10[0] = param_2;
  local_res18[0] = param_3;
  if (param_3 == 0) {
    pHVar2 = GetModuleHandleW((LPCWSTR)0x0);
    if ((((pHVar2 != (HMODULE)0x0) && ((short)pHVar2->unused == 0x5a4d)) &&
        (piVar3 = (int *)((longlong)&pHVar2->unused + (longlong)pHVar2[0xf].unused),
        *piVar3 == 0x4550)) &&
       ((((short)piVar3[6] == 0x20b && (0xe < (uint)piVar3[0x21])) && (piVar3[0x3e] != 0)))) {
      FUN_18005cd24(param_1);
    }
  }
  local_res20[0] = 0;
  local_20 = local_res10;
  local_18 = local_res18;
  local_10 = local_res20;
  local_34[0] = 2;
  local_34[1] = 2;
  operator()<>(local_38,local_34 + 1,&local_20,local_34);
  if (local_res18[0] != 0) {
    return;
  }
  FUN_18005cccc(param_1);
  pcVar1 = (code *)swi(3);
  (*pcVar1)();
  return;
}

/* 0x18005cccc FUN_18005cccc; Evidence ev_f705ae67c18bf90dc48c21d4fdfdcc5988dae40324dcff736f3c7928ead796a5 */

void FUN_18005cccc(UINT param_1)

{
  bool bVar1;
  HANDLE hProcess;
  
  bVar1 = FUN_18005cd00();
  if (bVar1) {
    hProcess = GetCurrentProcess();
    TerminateProcess(hProcess,param_1);
  }
  FUN_18005cd24(param_1);
                    /* WARNING: Subroutine does not return */
  ExitProcess(param_1);
}

/* 0x18005cd00 FUN_18005cd00; Evidence ev_aae0f008304db15ce13a7a66b59578f7cf4ce8663d8248f617109ad76604b6ca */

bool FUN_18005cd00(void)

{
  bool bVar1;
  undefined3 extraout_var;
  uint uVar2;
  
  bVar1 = FUN_18006c110();
  if (CONCAT31(extraout_var,bVar1) == 1) {
    bVar1 = false;
  }
  else {
    uVar2 = FUN_18006c09c();
    bVar1 = (char)uVar2 == '\0';
  }
  return bVar1;
}

/* 0x18005cd24 FUN_18005cd24; Evidence ev_3908cdf401d4173b3a55da913b396be037c14830e00b494494eda59d68df96fa */

/* WARNING: Function: _guard_dispatch_icall replaced with injection: guard_dispatch_icall */
/* WARNING: Exceeded maximum restarts with more pending */

void FUN_18005cd24(undefined4 param_1)

{
  BOOL BVar1;
  FARPROC pFVar2;
  HMODULE local_res10 [3];
  
  local_res10[0] = (HMODULE)0x0;
  BVar1 = GetModuleHandleExW(0,L"mscoree.dll",local_res10);
  if ((BVar1 != 0) &&
     (pFVar2 = GetProcAddress(local_res10[0],"CorExitProcess"), pFVar2 != (FARPROC)0x0)) {
    (*pFVar2)(param_1);
  }
  if (local_res10[0] != (HMODULE)0x0) {
    FreeLibrary(local_res10[0]);
  }
  return;
}

/* 0x18005cd9c FUN_18005cd9c; Evidence ev_a8a2e2a3da033fefa810538a6e3943573c34ae8485b608043ba5bcc131f8765f */

void FUN_18005cd9c(void)

{
  FUN_18005cc04(0,0,1);
  return;
}

/* 0x18005cdac FUN_18005cdac; Evidence ev_36d5cf22008de36a8eb1c8e9cb8a834a8edf44349f629ede68abef82fe2192f1 */

void FUN_18005cdac(UINT param_1)

{
  FUN_18005cc04(param_1,2,0);
  return;
}

/* 0x18005cdb8 FUN_18005cdb8; Evidence ev_55158cc01d6278f6efc44c7a45e24421786674f168e77f6cae67bffef40c696c */

undefined4 FUN_18005cdb8(void)

{
  return DAT_1800a4f08;
}

/* 0x18005cdc0 FUN_18005cdc0; Evidence ev_4da3d1a6508c073e95fc5f849944d03628a1fcd5b362f56c5b16b259f6ad8d84 */

void FUN_18005cdc0(UINT param_1)

{
  FUN_18005cc04(param_1,0,0);
  return;
}