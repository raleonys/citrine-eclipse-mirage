/* 0x18006ccc8 FUN_18006ccc8; Evidence ev_b20fc9636a2c56a1d955fa71efd50f6472b9937c3a169ae04125c9b14510bf03 */

void FUN_18006ccc8(UINT param_1,DWORD param_2,LPCSTR param_3,int param_4,LPWSTR param_5,int param_6)

{
  bool bVar1;
  
  if (param_1 < 0xdead) {
    if (param_1 != 0xdeac) {
      if (param_1 < 0xc434) {
        if ((((param_1 != 0xc433) && (param_1 != 0x2a)) && (param_1 != 0xc42c)) &&
           ((param_1 != 0xc42d && (param_1 != 0xc42e)))) {
          bVar1 = param_1 == 0xc431;
LAB_18006cd15:
          if (!bVar1) goto LAB_18006cd19;
        }
      }
      else if (param_1 != 0xc435) {
        if (param_1 == 0xd698) goto LAB_18006cd53;
        if (param_1 != 0xdeaa) {
          bVar1 = param_1 == 0xdeab;
          goto LAB_18006cd15;
        }
      }
    }
  }
  else if ((((param_1 != 0xdead) && (param_1 != 0xdeae)) && (param_1 != 0xdeaf)) &&
          (((param_1 != 0xdeb0 && (param_1 != 0xdeb1)) &&
           ((param_1 != 0xdeb2 && ((param_1 != 0xdeb3 && (param_1 != 65000)))))))) {
    if (param_1 != 0xfde9) goto LAB_18006cd19;
LAB_18006cd53:
    param_2 = param_2 & 8;
    goto LAB_18006cd19;
  }
  param_2 = 0;
LAB_18006cd19:
                    /* WARNING: Could not recover jumptable at 0x00018006cd19. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  MultiByteToWideChar(param_1,param_2,param_3,param_4,param_5,param_6);
  return;
}




/* 0x18006cd58 FUN_18006cd58; Evidence ev_199d5128e65e855a843901c44a214968af6d9232730c107b76dc582ac07ccad1 */

void FUN_18006cd58(uint param_1,uint param_2,LPCWSTR param_3,int param_4,LPSTR param_5,int param_6,
                  LPBOOL param_7,LPBOOL param_8)

{
  LPBOOL lpUsedDefaultChar;
  int iVar1;
  DWORD dwFlags;
  LPBOOL pBVar2;
  bool bVar3;
  
  pBVar2 = (LPBOOL)0x0;
  dwFlags = 0;
  if (param_1 < 0xdead) {
    if (param_1 == 0xdeac) goto LAB_18006cdf8;
    if (0xc433 < param_1) {
      if ((param_1 == 0xc435) || (param_1 == 0xd698)) goto LAB_18006cdf8;
      iVar1 = param_1 - 0xdeaa;
      goto LAB_18006cde9;
    }
    if ((((param_1 == 0xc433) || (param_1 == 0x2a)) || (param_1 == 0xc42c)) ||
       ((param_1 == 0xc42d || (param_1 == 0xc42e)))) goto LAB_18006cdf8;
    bVar3 = param_1 == 0xc431;
  }
  else {
    if (((((param_1 == 0xdead) || (param_1 == 0xdeae)) || (param_1 == 0xdeaf)) ||
        ((param_1 == 0xdeb0 || (param_1 == 0xdeb1)))) ||
       ((param_1 == 0xdeb2 || (param_1 == 0xdeb3)))) goto LAB_18006cdf8;
    iVar1 = param_1 - 65000;
LAB_18006cde9:
    if (iVar1 == 0) goto LAB_18006cdf8;
    bVar3 = iVar1 == 1;
  }
  if (!bVar3) {
    dwFlags = param_2 & 0xffffff7f;
  }
LAB_18006cdf8:
  lpUsedDefaultChar = param_8;
  if ((param_1 - 65000 < 2) &&
     (param_7 = pBVar2, lpUsedDefaultChar = pBVar2, param_8 != (LPBOOL)0x0)) {
    *param_8 = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x00018006ce2f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  WideCharToMultiByte(param_1,dwFlags,param_3,param_4,param_5,param_6,(LPCSTR)param_7,
                      lpUsedDefaultChar);
  return;
}




/* 0x180066c1c FUN_180066c1c; Evidence ev_9eec0ac9cfd859ebdf7fadf5438403822349fa6b1ece7b56f77e74ccf70a9ca3 */

/* WARNING: Function: _guard_dispatch_icall replaced with injection: guard_dispatch_icall */
/* WARNING: Exceeded maximum restarts with more pending */

undefined8 FUN_180066c1c(void)

{
  FARPROC pFVar1;
  undefined8 uVar2;
  
  pFVar1 = FUN_1800669b0(0,"AreFileApisANSI",(uint *)&DAT_18008b8f8,(uint *)&DAT_18008b8fc);
  if (pFVar1 != (FARPROC)0x0) {
    uVar2 = (*pFVar1)();
    return uVar2;
  }
  return 1;
}



