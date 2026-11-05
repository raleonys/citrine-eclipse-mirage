/* 0x180024480 FUN_180024480; Evidence ev_20e549a27321ba1d7651fc94fdf90ec98fec21fff58fbc1d4561e3b5a5828d5c */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

longlong * FUN_180024480(longlong *param_1,undefined8 *param_2,u_short param_3)

{
  code *pcVar1;
  ulonglong uVar2;
  LPVOID pvVar3;
  bool bVar4;
  u_short uVar5;
  u_long uVar6;
  int iVar7;
  undefined8 uVar8;
  undefined **ppuVar9;
  longlong *plVar10;
  LPVOID pvVar11;
  int *lpValue;
  uint uVar12;
  uint uVar13;
  ulonglong uVar14;
  char *pcVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  undefined1 auStackY_108 [32];
  longlong *local_d8 [2];
  longlong *local_c8;
  int local_b8;
  undefined4 uStack_b4;
  undefined **ppuStack_b0;
  ulonglong local_a0;
  sockaddr local_98;
  undefined8 uStack_88;
  uint local_80;
  undefined8 local_78;
  undefined8 uStack_70;
  undefined1 local_68 [8];
  undefined4 local_60;
  ulonglong local_58;
  
  local_58 = DAT_1800a1580 ^ (ulonglong)auStackY_108;
  uVar13 = 0;
  local_b8 = 0;
  ppuStack_b0 = &PTR_vftable_1800a10f8;
  local_d8[0] = (longlong *)((ulonglong)local_d8[0] & 0xffffffff00000000);
  local_c8 = param_1;
  uVar8 = FUN_180028690(0x17,"127.0.0.1",&local_78,(undefined4 *)local_d8,&local_b8);
  if ((int)uVar8 < 1) {
    uVar16 = 0;
    uVar17 = 0;
    uVar18 = 0;
    uVar19 = 0;
    uVar12 = 0;
  }
  else {
    uVar16 = (undefined4)local_78;
    uVar17 = local_78._4_4_;
    uVar18 = (undefined4)uStack_70;
    uVar19 = uStack_70._4_4_;
    uVar12 = (uint)local_d8[0];
  }
  if (local_b8 == 0) {
    bVar4 = true;
    uVar6 = 0;
  }
  else {
    uVar8 = FUN_180028690(2,"127.0.0.1",local_d8,(undefined4 *)0x0,&local_b8);
    uVar6 = (uint)local_d8[0];
    if ((int)uVar8 < 1) {
      uVar6 = 0;
    }
    if (local_b8 != 0) {
      FUN_180028210((undefined8 *)&local_b8);
      FUN_180024020(&local_78);
      FUN_1800238a0((longlong)&local_78);
      goto LAB_1800247f2;
    }
    bVar4 = false;
    uVar16 = 0;
    uVar17 = 0;
    uVar18 = 0;
    uVar19 = 0;
    uVar12 = uVar13;
  }
  local_98.sa_data[6] = '\0';
  local_98.sa_data[7] = '\0';
  local_98.sa_data[8] = '\0';
  local_98.sa_data[9] = '\0';
  local_98.sa_data[10] = '\0';
  local_98.sa_data[0xb] = '\0';
  local_98.sa_data[0xc] = '\0';
  local_98.sa_data[0xd] = '\0';
  uStack_88 = 0;
  local_80 = 0;
  if (bVar4) {
    local_98.sa_family = 0x17;
    uVar5 = htons(param_3);
    local_98.sa_data[0] = (char)uVar5;
    local_98.sa_data[1] = (char)(uVar5 >> 8);
    local_98.sa_data[2] = '\0';
    local_98.sa_data[3] = '\0';
    local_98.sa_data[4] = '\0';
    local_98.sa_data[5] = '\0';
    local_98.sa_data[10] = (char)uVar17;
    local_98.sa_data[0xb] = (char)((uint)uVar17 >> 8);
    local_98.sa_data[0xc] = (char)((uint)uVar17 >> 0x10);
    local_98.sa_data[0xd] = (char)((uint)uVar17 >> 0x18);
    local_98.sa_data[6] = (char)uVar16;
    local_98.sa_data[7] = (char)((uint)uVar16 >> 8);
    local_98.sa_data[8] = (char)((uint)uVar16 >> 0x10);
    local_98.sa_data[9] = (char)((uint)uVar16 >> 0x18);
    uStack_88 = CONCAT44(uVar19,uVar18);
    local_80 = uVar12;
  }
  else {
    local_98.sa_family = 2;
    uVar5 = htons(param_3);
    local_98.sa_data[0] = (char)uVar5;
    local_98.sa_data[1] = (char)(uVar5 >> 8);
    uVar6 = ntohl(uVar6);
    uVar6 = htonl(uVar6);
    local_98.sa_data[2] = (char)uVar6;
    local_98.sa_data[3] = (char)(uVar6 >> 8);
    local_98.sa_data[4] = (char)(uVar6 >> 0x10);
    local_98.sa_data[5] = (char)(uVar6 >> 0x18);
  }
  FUN_180020e20(param_1,param_2,&local_98,'\x01');
  FUN_1800211b0(param_1 + 0x13,0,0,param_2);
  local_98.sa_data[6] = '\0';
  local_98.sa_data[7] = '\0';
  local_98.sa_data[8] = '\0';
  local_98.sa_data[9] = '\0';
  local_98.sa_data[10] = '\0';
  local_98.sa_data[0xb] = '\0';
  local_98.sa_data[0xc] = '\0';
  local_98.sa_data[0xd] = '\0';
  uStack_88 = 0;
  local_80 = 0;
  local_98.sa_family = 2;
  local_98.sa_data[0] = '\0';
  local_98.sa_data[1] = '\0';
  local_98.sa_data[2] = '\0';
  local_98.sa_data[3] = '\0';
  local_98.sa_data[4] = '\0';
  local_98.sa_data[5] = '\0';
  if (param_1[1] == 0xffffffffffffffff) {
    ppuStack_b0 = FUN_180028620();
    local_b8 = 0x2719;
  }
  else {
    local_d8[0] = (longlong *)CONCAT44(local_d8[0]._4_4_,0x1c);
    iVar7 = getsockname(param_1[1],&local_98,(int *)local_d8);
    if (iVar7 == 0) {
      local_b8 = 0;
      ppuStack_b0 = &PTR_vftable_1800a10f8;
      if (0x80 < (uint)local_d8[0]) {
LAB_1800247f2:
        ppuStack_b0 = FUN_180028620();
        local_b8 = 0x2726;
        local_78 = CONCAT44(uStack_b4,0x2726);
        uStack_70 = ppuStack_b0;
        FUN_180028210(&local_78);
        pcVar1 = (code *)swi(3);
        plVar10 = (longlong *)(*pcVar1)();
        return plVar10;
      }
      local_68 = (undefined1  [8])uStack_88;
      local_60 = local_80;
      uVar13 = (uint)(ushort)local_98.sa_data._0_2_;
      goto LAB_18002466d;
    }
    ppuVar9 = FUN_180028620();
    local_b8 = WSAGetLastError();
    ppuStack_b0 = ppuVar9;
  }
  if (local_b8 != 0) {
                    /* WARNING: Subroutine does not return */
    FUN_180028240((undefined8 *)&local_b8,(undefined8 *)"local_endpoint");
  }
LAB_18002466d:
  DAT_1800a3c90 = ntohs((u_short)uVar13);
  pcVar15 = local_68 + 5;
  uVar14 = (ulonglong)DAT_1800a3c90;
  do {
    pcVar15 = pcVar15 + -1;
    uVar2 = uVar14 / 10;
    *pcVar15 = (char)uVar14 + (char)uVar2 * -10 + '0';
    uVar14 = uVar2;
  } while ((int)uVar2 != 0);
  FUN_18001e6e0((undefined8 *)&local_b8,(undefined8 *)pcVar15,(undefined8 *)(local_68 + 5));
  lpValue = &local_b8;
  if (0xf < local_a0) {
    lpValue = (int *)CONCAT44(uStack_b4,local_b8);
  }
  SetEnvironmentVariableA("EALsxPort",(LPCSTR)lpValue);
  if (0xf < local_a0) {
    pvVar3 = (LPVOID)CONCAT44(uStack_b4,local_b8);
    pvVar11 = pvVar3;
    if ((0xfff < local_a0 + 1) &&
       (pvVar11 = *(LPVOID *)((longlong)pvVar3 + -8),
       0x1f < (ulonglong)((longlong)pvVar3 + (-8 - (longlong)pvVar11)))) {
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(pvVar11);
  }
  FUN_180036820(&DAT_1800a3c3d);
  local_d8[0] = param_1;
  FUN_180021b00(*param_1,(SOCKET *)(param_1 + 1),param_1 + 0x13,0,(ULONG_PTR *)local_d8,
                (longlong)(param_1 + 0xd));
  return param_1;
}



