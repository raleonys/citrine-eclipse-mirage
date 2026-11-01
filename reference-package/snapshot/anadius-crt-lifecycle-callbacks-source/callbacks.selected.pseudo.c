/* 0x18006447c FUN_18006447c; Evidence ev_128dcdbd118a6380a4750ff1e54271f936ddb311b7b3557f4bfaca02a8a8e7d5 */

undefined8 FUN_18006447c(void)

{
  undefined8 uVar1;
  
  _initialize_onexit_table((longlong *)&DAT_1800a52a8);
  uVar1 = _initialize_onexit_table((longlong *)&DAT_1800a52c0);
  return CONCAT71((int7)((ulonglong)uVar1 >> 8),1);
}




/* 0x1800644f8 FUN_1800644f8; Evidence ev_82ed5c1b2273a1b6458c896c47a7f737a42930c9a261f3be55551ef4e056625a */

undefined8 FUN_1800644f8(void)

{
  uint uVar1;
  ulonglong uVar2;
  
  LOCK();
  uVar1 = *DAT_1800a59d0;
  uVar2 = (ulonglong)uVar1;
  *DAT_1800a59d0 = *DAT_1800a59d0 - 1;
  UNLOCK();
  if ((uVar1 == 1) && (DAT_1800a59d0 != (uint *)&DAT_1800a1c70)) {
    uVar2 = FUN_1800668c0(DAT_1800a59d0);
    DAT_1800a59d0 = (uint *)&DAT_1800a1c70;
  }
  return CONCAT71((int7)(uVar2 >> 8),1);
}



