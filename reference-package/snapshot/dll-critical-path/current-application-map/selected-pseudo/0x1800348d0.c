/* 0x1800348d0 FUN_1800348d0; Evidence ev_c8956389f06090304d78bbbaeb7ba946fd58733924c38886a9cab4869d2c9cd2 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 * FUN_1800348d0(undefined8 *param_1,uint param_2)

{
  uint uVar1;
  char *pcVar2;
  undefined1 auStack_58 [40];
  undefined8 *local_30;
  char local_15 [5];
  ulonglong local_10;
  char *pcVar3;
  
  local_10 = DAT_1800a1580 ^ (ulonglong)auStack_58;
  pcVar2 = local_15 + 2;
  if ((int)param_2 < 0) {
    uVar1 = -param_2;
    do {
      pcVar3 = pcVar2;
      pcVar2 = pcVar3 + -1;
      *pcVar2 = (char)uVar1 + (char)((ulonglong)uVar1 / 10) * -10 + '0';
      uVar1 = (uint)((ulonglong)uVar1 / 10);
    } while (uVar1 != 0);
    pcVar2 = pcVar3 + -2;
    *pcVar2 = '-';
  }
  else {
    do {
      pcVar2 = pcVar2 + -1;
      *pcVar2 = (char)param_2 + (char)((ulonglong)param_2 / 10) * -10 + '0';
      param_2 = (uint)((ulonglong)param_2 / 10);
    } while (param_2 != 0);
  }
  local_30 = param_1;
  FUN_18001e6e0(param_1,(undefined8 *)pcVar2,(undefined8 *)(local_15 + 2));
  return param_1;
}



