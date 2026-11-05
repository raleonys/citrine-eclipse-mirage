/* 0x180029ea0 FUN_180029ea0; Evidence ev_6cf0254d6c71c9d2e74fabc14f751e84a071e29cca026bf32d866351d4cf2705 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

void FUN_180029ea0(void)

{
  longlong *plVar1;
  undefined8 *puVar2;
  code *pcVar3;
  int iVar4;
  LPCRITICAL_SECTION p_Var5;
  undefined8 *puVar6;
  bool bVar7;
  undefined1 auStack_1f8 [112];
  undefined1 local_188 [64];
  longlong local_148 [19];
  longlong local_b0 [19];
  ulonglong local_18;
  
  local_18 = DAT_1800a1580 ^ (ulonglong)auStack_1f8;
  FUN_1800241b0((PRTL_CRITICAL_SECTION_DEBUG)local_188);
  FUN_180024480(local_148,(undefined8 *)local_188,DAT_1800a3c90);
  local_188._32_4_ = 0;
  local_188._40_8_ = &PTR_vftable_1800a10f8;
  FUN_1800298b0((longlong *)local_188._16_8_,(longlong *)(local_188 + 0x20));
  if (local_188._32_4_ == 0) {
    FUN_180024db0(local_b0);
    FUN_180024db0(local_148);
    LOCK();
    iVar4 = DAT_1800a3900 + -1;
    UNLOCK();
    bVar7 = DAT_1800a3900 == 1;
    DAT_1800a3900 = iVar4;
    if (bVar7) {
      WSACleanup();
    }
    p_Var5 = (LPCRITICAL_SECTION)local_188._0_8_;
    for (plVar1 = *(longlong **)&((LPCRITICAL_SECTION)(local_188._0_8_ + 0x28))->LockCount;
        local_188._0_8_ = p_Var5, plVar1 != (longlong *)0x0; plVar1 = (longlong *)plVar1[4]) {
      (**(code **)(*plVar1 + 8))(plVar1);
      p_Var5 = (LPCRITICAL_SECTION)local_188._0_8_;
    }
    puVar6 = *(undefined8 **)&p_Var5[1].LockCount;
    while (puVar6 != (undefined8 *)0x0) {
      puVar2 = (undefined8 *)puVar6[4];
      (**(code **)*puVar6)(puVar6,1);
      *(undefined8 **)&p_Var5[1].LockCount = puVar2;
      puVar6 = puVar2;
    }
    if ((LPCRITICAL_SECTION)local_188._0_8_ != (LPCRITICAL_SECTION)0x0) {
      DeleteCriticalSection((LPCRITICAL_SECTION)local_188._0_8_);
      thunk_FUN_18005caec((LPVOID)local_188._0_8_);
    }
    return;
  }
  FUN_180028210((undefined8 *)(local_188 + 0x20));
  pcVar3 = (code *)swi(3);
  (*pcVar3)();
  return;
}



