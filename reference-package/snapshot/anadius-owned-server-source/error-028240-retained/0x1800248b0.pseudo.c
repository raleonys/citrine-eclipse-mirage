/* 0x1800248b0 FUN_1800248b0; Evidence ev_b2bf6baada4cd16a73cc61d2de01545aa74f09abc92379812bd927dd62ffb425 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 ***** FUN_1800248b0(undefined8 *****param_1,undefined8 *param_2,undefined8 *param_3)

{
  LPVOID pvVar1;
  ulonglong uVar2;
  undefined1 auStack_68 [32];
  undefined8 *****local_48;
  undefined8 uStack_40;
  LPVOID local_38;
  undefined8 uStack_30;
  undefined8 local_28;
  ulonglong uStack_20;
  ulonglong local_18;
  
  local_18 = DAT_1800a1580 ^ (ulonglong)auStack_68;
  local_38 = (LPVOID)0x0;
  uStack_30 = 0;
  local_28 = 0;
  uStack_20 = 0;
  uVar2 = 0xffffffffffffffff;
  do {
    uVar2 = uVar2 + 1;
  } while (*(char *)((longlong)param_3 + uVar2) != '\0');
  local_48 = param_1;
  FUN_180006b30(&local_38,param_3,uVar2);
  local_48 = (undefined8 *****)*param_2;
  uStack_40 = param_2[1];
  FUN_18000a270(param_1,(ulonglong *)&local_48,&local_38);
  if (0xf < uStack_20) {
    pvVar1 = local_38;
    if ((0xfff < uStack_20 + 1) &&
       (pvVar1 = *(LPVOID *)((longlong)local_38 + -8),
       0x1f < (ulonglong)((longlong)local_38 + (-8 - (longlong)pvVar1)))) {
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(pvVar1);
  }
  *param_1 = (undefined8 ****)std::system_error::vftable;
  return param_1;
}



