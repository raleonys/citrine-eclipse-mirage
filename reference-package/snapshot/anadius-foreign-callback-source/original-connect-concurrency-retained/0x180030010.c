/* 0x180030010 FUN_180030010; Evidence ev_8e0c6e33949acd640bbdc56b56ceea86f8bb373eca849ee8c6a34d8a0ac1b175 */

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

undefined8 FUN_180030010(longlong param_1,longlong *param_2)

{
  uint *puVar1;
  char cVar2;
  ulonglong uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  longlong *plVar7;
  longlong lVar8;
  longlong *plVar9;
  longlong *plVar10;
  uint *puVar11;
  undefined8 *puVar12;
  byte *pbVar13;
  undefined8 uVar14;
  undefined8 *******pppppppuVar15;
  longlong *plVar16;
  undefined8 ******ppppppuVar17;
  longlong *plVar18;
  ulonglong uVar19;
  ulonglong uVar20;
  ulonglong uVar21;
  ulonglong uVar22;
  undefined1 auStack_3d8 [32];
  uint local_3b8;
  longlong *local_3a8;
  longlong lStack_3a0;
  longlong *local_398;
  undefined *local_390;
  undefined8 ******local_388;
  longlong lStack_380;
  ulonglong local_378;
  ulonglong uStack_370;
  longlong *local_368;
  undefined8 uStack_360;
  longlong *local_358;
  ulonglong uStack_350;
  uint local_348;
  undefined8 local_344;
  undefined8 *local_338;
  undefined8 uStack_330;
  undefined8 local_328;
  undefined8 uStack_320;
  longlong local_318;
  undefined8 uStack_310;
  undefined8 local_308;
  undefined8 uStack_300;
  longlong local_2f8;
  undefined8 uStack_2f0;
  undefined8 local_2e8;
  undefined8 uStack_2e0;
  longlong local_2d8;
  undefined **local_2c8 [2];
  undefined1 *local_2b8;
  undefined1 local_2b0 [248];
  undefined8 *local_1b8;
  undefined8 local_1b0 [5];
  longlong local_188 [34];
  undefined8 *local_78;
  ulonglong local_48;
  
  local_48 = DAT_1800a1580 ^ (ulonglong)auStack_3d8;
  local_398 = param_2;
  plVar7 = (longlong *)FUN_180002cc0(param_1,"AchievementId",(char *)0x0);
  if (plVar7 == (longlong *)0x0) {
    return 0;
  }
  local_3b8 = 1;
  local_3a8 = plVar7;
  lVar8 = FUN_180002cc0(param_1,"Progress",(char *)0x0);
  if (lVar8 != 0) {
    local_3b8 = FUN_18005ee08(lVar8);
  }
  local_390 = &DAT_1800a3e08;
  iVar4 = FUN_18004a6ac((uint *)&DAT_1800a3e08);
  if (iVar4 != 0) {
                    /* WARNING: Subroutine does not return */
    FUN_18004a648(5);
  }
  if (DAT_1800a3e54 == 0x7fffffff) {
    DAT_1800a3e54 = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    FUN_18004a648(6);
  }
  local_388 = (undefined8 *******)0x0;
  lStack_380 = 0;
  local_378 = 0;
  uStack_370 = 0;
  uVar22 = 0xffffffffffffffff;
  uVar19 = 0xffffffffffffffff;
  do {
    uVar19 = uVar19 + 1;
  } while (*(char *)(uVar19 + (longlong)plVar7) != '\0');
  FUN_180006b30(&local_388,plVar7,uVar19);
  uVar3 = uStack_370;
  uVar21 = local_378;
  ppppppuVar17 = local_388;
  cVar2 = *(char *)(DAT_1800a3e58[1] + 0x19);
  plVar10 = (longlong *)DAT_1800a3e58[1];
  plVar16 = DAT_1800a3e58;
  plVar9 = DAT_1800a3e58;
  uVar19 = uStack_370;
  while (plVar18 = plVar10, DAT_1800a3e58 = plVar9, uStack_370 = uVar19, cVar2 == '\0') {
    plVar7 = plVar18 + 4;
    pppppppuVar15 = &local_388;
    if (0xf < uVar3) {
      pppppppuVar15 = (undefined8 *******)ppppppuVar17;
    }
    uVar19 = plVar18[6];
    if (0xf < (ulonglong)plVar18[7]) {
      plVar7 = (longlong *)*plVar7;
    }
    uVar20 = uVar19;
    if (uVar21 < uVar19) {
      uVar20 = uVar21;
    }
    uVar5 = memcmp(plVar7,pppppppuVar15,uVar20);
    if (uVar5 == 0) {
      if (uVar19 < uVar21) {
        uVar5 = 0xffffffff;
      }
      else {
        uVar5 = (uint)(uVar21 < uVar19);
      }
    }
    if ((int)uVar5 < 0) {
      plVar10 = (longlong *)plVar18[2];
      plVar18 = plVar16;
    }
    else {
      plVar10 = (longlong *)*plVar18;
    }
    cVar2 = *(char *)((longlong)plVar10 + 0x19);
    plVar7 = local_3a8;
    plVar16 = plVar18;
    plVar9 = DAT_1800a3e58;
    uVar19 = uStack_370;
  }
  if (*(char *)((longlong)plVar16 + 0x19) == '\0') {
    plVar10 = plVar16 + 4;
    uVar3 = plVar16[6];
    if (0xf < (ulonglong)plVar16[7]) {
      plVar10 = (longlong *)*plVar10;
    }
    pppppppuVar15 = &local_388;
    if (0xf < uVar19) {
      pppppppuVar15 = (undefined8 *******)ppppppuVar17;
    }
    uVar20 = uVar21;
    if (uVar3 < uVar21) {
      uVar20 = uVar3;
    }
    uVar5 = memcmp(pppppppuVar15,plVar10,uVar20);
    if (uVar5 == 0) {
      if (uVar21 < uVar3) {
        uVar5 = 0xffffffff;
      }
      else {
        uVar5 = (uint)(uVar3 < uVar21);
      }
    }
    plVar9 = DAT_1800a3e58;
    if (-1 < (int)uVar5) {
      plVar9 = plVar16;
    }
  }
  if (0xf < uVar19) {
    pppppppuVar15 = (undefined8 *******)ppppppuVar17;
    if ((0xfff < uVar19 + 1) &&
       (pppppppuVar15 = (undefined8 *******)ppppppuVar17[-1],
       0x1f < (ulonglong)((longlong)ppppppuVar17 + (-8 - (longlong)pppppppuVar15))))
    goto LAB_180030c2e;
    thunk_FUN_18005caec(pppppppuVar15);
  }
  uVar5 = local_3b8;
  if (plVar9 != DAT_1800a3e58) goto LAB_180030626;
  local_348 = local_3b8;
  local_344 = 0;
  local_338 = (undefined8 *)0x0;
  uStack_330 = 0;
  local_338 = (undefined8 *)FUN_18004b0bc(0x20);
  local_328 = 0x13;
  uStack_320 = 0x1f;
  *local_338 = 0x2d30302d30303030;
  local_338[1] = 0x30303a3030543030;
  *(undefined2 *)(local_338 + 2) = 0x303a;
  *(undefined1 *)((longlong)local_338 + 0x12) = 0x30;
  *(undefined1 *)((longlong)local_338 + 0x13) = 0;
  uStack_310 = 0;
  local_308 = 0;
  uStack_300 = 0xf;
  local_318 = 0;
  uStack_2f0 = 0;
  local_2e8 = 0;
  uStack_2e0 = 0xf;
  local_2f8 = 0;
  local_2d8 = 0;
  local_388 = (undefined8 *******)0x0;
  lStack_380 = 0;
  local_378 = 0;
  uStack_370 = 0;
  uVar19 = 0xffffffffffffffff;
  do {
    uVar19 = uVar19 + 1;
  } while (*(char *)(uVar19 + (longlong)plVar7) != '\0');
  FUN_180006b30(&local_388,plVar7,uVar19);
  uVar21 = uStack_370;
  uVar19 = local_378;
  local_368 = (longlong *)DAT_1800a3e58[1];
  uStack_360 = (ulonglong)uStack_360._4_4_ << 0x20;
  cVar2 = *(char *)((longlong)local_368 + 0x19);
  plVar10 = local_368;
  plVar16 = DAT_1800a3e58;
  plVar9 = DAT_1800a3e58;
  while (plVar18 = plVar10, local_3a8 = plVar16, cVar2 == '\0') {
    plVar10 = plVar18 + 4;
    pppppppuVar15 = &local_388;
    if (0xf < uVar21) {
      pppppppuVar15 = (undefined8 *******)local_388;
    }
    uVar3 = plVar18[6];
    if (0xf < (ulonglong)plVar18[7]) {
      plVar10 = (longlong *)*plVar10;
    }
    uVar20 = uVar3;
    if (uVar19 < uVar3) {
      uVar20 = uVar19;
    }
    local_368 = plVar18;
    uVar5 = memcmp(plVar10,pppppppuVar15,uVar20);
    if (uVar5 == 0) {
      if (uVar3 < uVar19) {
        uVar5 = 0xffffffff;
      }
      else {
        uVar5 = (uint)(uVar19 < uVar3);
      }
    }
    if ((int)uVar5 < 0) {
      uStack_360 = (ulonglong)uStack_360._4_4_ << 0x20;
      plVar10 = (longlong *)plVar18[2];
      plVar18 = plVar9;
    }
    else {
      uStack_360 = CONCAT44(uStack_360._4_4_,1);
      plVar10 = (longlong *)*plVar18;
    }
    cVar2 = *(char *)((longlong)plVar10 + 0x19);
    plVar16 = local_3a8;
    plVar9 = plVar18;
  }
  if (*(char *)((longlong)plVar9 + 0x19) == '\0') {
    plVar10 = plVar9 + 4;
    uVar3 = plVar9[6];
    if (0xf < (ulonglong)plVar9[7]) {
      plVar10 = (longlong *)*plVar10;
    }
    pppppppuVar15 = &local_388;
    if (0xf < uVar21) {
      pppppppuVar15 = (undefined8 *******)local_388;
    }
    uVar21 = uVar19;
    if (uVar3 < uVar19) {
      uVar21 = uVar3;
    }
    uVar5 = memcmp(pppppppuVar15,plVar10,uVar21);
    if (uVar5 == 0) {
      if (uVar19 < uVar3) {
        uVar5 = 0xffffffff;
      }
      else {
        uVar5 = (uint)(uVar3 < uVar19);
      }
    }
    if ((int)uVar5 < 0) goto LAB_1800303f3;
  }
  else {
LAB_1800303f3:
    if (DAT_1800a3e60 == 0x1642c8590b21642) {
                    /* WARNING: Subroutine does not return */
      FUN_18000cdf0();
    }
    local_3a8 = (longlong *)&DAT_1800a3e58;
    lStack_3a0 = 0;
    plVar10 = (longlong *)FUN_18004b0bc(0xb8);
    plVar10[6] = 0;
    plVar10[7] = 0;
    plVar10[4] = (longlong)local_388;
    plVar10[5] = lStack_380;
    plVar10[6] = local_378;
    plVar10[7] = uStack_370;
    local_378 = 0;
    uStack_370 = 0xf;
    local_388 = (undefined8 ******)((ulonglong)local_388 & 0xffffffffffffff00);
    *(undefined8 *)((longlong)plVar10 + 0x44) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x4c) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x54) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x5c) = 0;
    *(undefined8 *)((longlong)plVar10 + 100) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x6c) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x74) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x7c) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x84) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x8c) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x94) = 0;
    *(undefined8 *)((longlong)plVar10 + 0x9c) = 0;
    *(undefined8 *)((longlong)plVar10 + 0xa4) = 0;
    *(undefined8 *)((longlong)plVar10 + 0xac) = 0;
    plVar10[8] = 0;
    *(undefined4 *)(plVar10 + 9) = 0;
    plVar10[10] = 0;
    plVar10[0xb] = 0;
    plVar10[0xc] = 0;
    plVar10[0xd] = 0xf;
    *(undefined1 *)(plVar10 + 10) = 0;
    plVar10[0xe] = 0;
    plVar10[0xf] = 0;
    plVar10[0x10] = 0;
    plVar10[0x11] = 0xf;
    *(undefined1 *)(plVar10 + 0xe) = 0;
    plVar10[0x12] = 0;
    plVar10[0x13] = 0;
    plVar10[0x14] = 0;
    plVar10[0x15] = 0xf;
    *(undefined1 *)(plVar10 + 0x12) = 0;
    plVar10[0x16] = 0;
    *plVar10 = (longlong)plVar16;
    plVar10[1] = (longlong)plVar16;
    plVar10[2] = (longlong)plVar16;
    *(undefined2 *)(plVar10 + 3) = 0;
    local_3a8 = local_368;
    lStack_3a0 = uStack_360;
    plVar9 = FUN_18000c220((longlong *)&DAT_1800a3e58,(longlong *)&local_3a8,plVar10);
  }
  uVar5 = local_3b8;
  *(uint *)(plVar9 + 8) = local_3b8;
  *(undefined8 *)((longlong)plVar9 + 0x44) = 0;
  FUN_180016460(plVar9 + 10,(longlong *)&local_338);
  FUN_180016460(plVar9 + 0xe,&local_318);
  FUN_180016460(plVar9 + 0x12,&local_2f8);
  plVar9[0x16] = local_2d8;
  FUN_18000aa50((longlong *)&local_388);
  FUN_180015d90((longlong)&local_348);
  local_388 = (undefined8 *******)0x0;
  lStack_380 = 0;
  local_378 = 0;
  uStack_370 = 0;
  uVar19 = 0xffffffffffffffff;
  do {
    uVar19 = uVar19 + 1;
  } while (*(char *)(uVar19 + (longlong)plVar7) != '\0');
  FUN_180006b30(&local_388,plVar7,uVar19);
  FUN_1800071d0((longlong *)&DAT_1800a3e58,(longlong *)&local_368,&local_388);
  plVar9 = local_358;
  uVar19 = uStack_370;
  ppppppuVar17 = local_388;
  if (*(char *)((longlong)local_358 + 0x19) == '\0') {
    plVar10 = local_358 + 4;
    if (0xf < (ulonglong)local_358[7]) {
      plVar10 = (longlong *)*plVar10;
    }
    pppppppuVar15 = &local_388;
    if (0xf < uStack_370) {
      pppppppuVar15 = (undefined8 *******)local_388;
    }
    uVar6 = FUN_1800127c0(pppppppuVar15,local_378,plVar10,local_358[6]);
    if ((int)uVar6 < 0) goto LAB_1800305e1;
  }
  else {
LAB_1800305e1:
    plVar9 = DAT_1800a3e58;
  }
  if (0xf < uVar19) {
    pppppppuVar15 = (undefined8 *******)ppppppuVar17;
    if ((0xfff < uVar19 + 1) &&
       (pppppppuVar15 = (undefined8 *******)ppppppuVar17[-1],
       0x1f < (ulonglong)((longlong)ppppppuVar17 + (-8 - (longlong)pppppppuVar15)))) {
LAB_180030c2e:
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(pppppppuVar15);
  }
LAB_180030626:
  puVar1 = (uint *)(plVar9 + 8);
  if ((int)plVar9[9] < (int)*puVar1) {
    *(uint *)(plVar9 + 9) = (int)plVar9[9] + uVar5;
    plVar10 = FUN_180041b60((longlong *)&local_388,plVar9 + 0x16,'\0');
    FUN_180016460(plVar9 + 10,plVar10);
    FUN_18000aa50((longlong *)&local_388);
    if ((int)*puVar1 <= (int)plVar9[9]) {
      *(undefined4 *)((longlong)plVar9 + 0x44) = 1;
      *(uint *)(plVar9 + 9) = *puVar1;
      FUN_1800019b0(local_188,0,0,0);
      FUN_180004080(local_188,0x180084c58,'\0');
      FUN_180004080(local_188,0x180084c5c,'\0');
      FUN_180005560(local_188,0x180084c64,(byte *)"EbisuSDK");
      FUN_18002bf60(local_188);
      local_388 = (undefined8 ******)0x0;
      lStack_380 = 0;
      local_378 = 0;
      uStack_370 = 0;
      uVar19 = 0xffffffffffffffff;
      do {
        uVar19 = uVar19 + 1;
      } while (*(char *)(uVar19 + (longlong)plVar7) != '\0');
      FUN_180006b30(&local_388,plVar7,uVar19);
      FUN_18002c080(local_188,(byte *)&local_388,puVar1);
      FUN_18000aa50((longlong *)&local_388);
      (**(code **)(local_188[0] + 0x48))(local_188,0);
      (**(code **)(local_188[0] + 0x48))(local_188,0);
      (**(code **)(local_188[0] + 0x48))(local_188,0);
      (**(code **)(local_188[0] + 0x48))(local_188,0);
      puVar11 = (uint *)FUN_18002c620();
      FUN_18002bc10(&local_3a8,puVar11);
      puVar12 = FUN_18002c400();
      local_388 = (undefined8 *******)0x0;
      lStack_380 = 0;
      local_378 = 0;
      uStack_370 = 0;
      uVar19 = 0xffffffffffffffff;
      do {
        uVar19 = uVar19 + 1;
      } while (*(char *)((longlong)local_78 + uVar19) != '\0');
      FUN_180006b30(&local_388,local_78,uVar19);
      FUN_180034660((longlong)puVar12,&local_388);
      FUN_18000aa50((longlong *)&local_388);
      FUN_18004a6b4((longlong)local_3a8);
      FUN_180001f60(local_188);
    }
    if (DAT_1800a3df8 != 0) {
      FUN_1800019b0(local_2c8,0,0,0);
      FUN_180004080((longlong *)local_2c8,0x180083c28,'\0');
      local_358 = (longlong *)0xc;
      uStack_350 = 0xf;
      local_368 = (longlong *)0x5f59414c50534944;
      uStack_360 = 0x454d414e;
      pbVar13 = (byte *)FUN_180016ab0(&DAT_1800a3d88,(longlong *)&local_368);
      if (0xf < *(ulonglong *)(pbVar13 + 0x18)) {
        pbVar13 = *(byte **)pbVar13;
      }
      FUN_180005560((longlong *)local_2c8,0x1800857d8,pbVar13);
      FUN_18000aa50((longlong *)&local_368);
      pbVar13 = (byte *)&DAT_1800a3af8;
      if (0xf < DAT_1800a3b10) {
        pbVar13 = DAT_1800a3af8;
      }
      FUN_180005560((longlong *)local_2c8,0x180083990,pbVar13);
      plVar10 = (longlong *)*DAT_1800a3e58;
      cVar2 = *(char *)((longlong)plVar10 + 0x19);
      while (cVar2 == '\0') {
        if ((int)plVar10[9] != 0) {
          FUN_180004080((longlong *)local_2c8,0x1800857e8,'\0');
          pbVar13 = (byte *)(plVar10 + 4);
          if (0xf < (ulonglong)plVar10[7]) {
            pbVar13 = (byte *)plVar10[4];
          }
          FUN_180005560((longlong *)local_2c8,0x1800841e0,pbVar13);
          pbVar13 = (byte *)FUN_1800348d0(&local_368,*(uint *)((longlong)plVar10 + 0x44));
          if (0xf < *(ulonglong *)(pbVar13 + 0x18)) {
            pbVar13 = *(byte **)pbVar13;
          }
          FUN_180005560((longlong *)local_2c8,0x1800841e4,pbVar13);
          if (0xf < uStack_350) {
            plVar16 = local_368;
            if ((0xfff < uStack_350 + 1) &&
               (plVar16 = (longlong *)local_368[-1],
               0x1f < (ulonglong)((longlong)local_368 + (-8 - (longlong)plVar16)))) {
                    /* WARNING: Subroutine does not return */
              FUN_18005c974();
            }
            thunk_FUN_18005caec(plVar16);
          }
          pbVar13 = (byte *)FUN_1800348d0(&local_368,*(uint *)(plVar10 + 9));
          if (0xf < *(ulonglong *)(pbVar13 + 0x18)) {
            pbVar13 = *(byte **)pbVar13;
          }
          FUN_180005560((longlong *)local_2c8,0x1800841f0,pbVar13);
          if (0xf < uStack_350) {
            plVar16 = local_368;
            if ((0xfff < uStack_350 + 1) &&
               (plVar16 = (longlong *)local_368[-1],
               0x1f < (ulonglong)((longlong)local_368 + (-8 - (longlong)plVar16)))) {
                    /* WARNING: Subroutine does not return */
              FUN_18005c974();
            }
            thunk_FUN_18005caec(plVar16);
          }
          pbVar13 = (byte *)(plVar10 + 10);
          if (0xf < (ulonglong)plVar10[0xd]) {
            pbVar13 = *(byte **)pbVar13;
          }
          FUN_180005560((longlong *)local_2c8,0x1800841fc,pbVar13);
          pbVar13 = (byte *)(plVar10 + 0xe);
          if (0xf < (ulonglong)plVar10[0x11]) {
            pbVar13 = *(byte **)pbVar13;
          }
          FUN_180005560((longlong *)local_2c8,0x180083954,pbVar13);
          FUN_180002f20((longlong *)local_2c8,'\0');
        }
        plVar16 = (longlong *)plVar10[2];
        if (*(char *)((longlong)plVar16 + 0x19) == '\0') {
          cVar2 = *(char *)(*plVar16 + 0x19);
          plVar10 = plVar16;
          plVar16 = (longlong *)*plVar16;
          while (cVar2 == '\0') {
            cVar2 = *(char *)(*plVar16 + 0x19);
            plVar10 = plVar16;
            plVar16 = (longlong *)*plVar16;
          }
        }
        else {
          cVar2 = *(char *)(plVar10[1] + 0x19);
          plVar9 = (longlong *)plVar10[1];
          plVar16 = plVar10;
          while ((plVar10 = plVar9, cVar2 == '\0' && (plVar16 == (longlong *)plVar10[2]))) {
            cVar2 = *(char *)(plVar10[1] + 0x19);
            plVar9 = (longlong *)plVar10[1];
            plVar16 = plVar10;
          }
        }
        cVar2 = *(char *)((longlong)plVar10 + 0x19);
      }
      FUN_180002f20((longlong *)local_2c8,'\0');
      local_388 = (undefined8 ******)0x0;
      lStack_380 = 0;
      local_378 = 0;
      uStack_370 = 0;
      uVar19 = 0xffffffffffffffff;
      do {
        uVar19 = uVar19 + 1;
      } while (*(char *)((longlong)local_1b8 + uVar19) != '\0');
      FUN_180006b30(&local_388,local_1b8,uVar19);
      FUN_18003c1d0((wchar_t *)&DAT_1800a3de8,&local_388);
      if (0xf < uStack_370) {
        ppppppuVar17 = local_388;
        if ((0xfff < uStack_370 + 1) &&
           (ppppppuVar17 = (undefined8 ******)local_388[-1],
           0x1f < (ulonglong)((longlong)local_388 + (-8 - (longlong)ppppppuVar17)))) {
                    /* WARNING: Subroutine does not return */
          FUN_18005c974();
        }
        thunk_FUN_18005caec(ppppppuVar17);
      }
      local_378 = 0;
      uStack_370 = 0xf;
      local_388 = (undefined8 ******)((ulonglong)local_388 & 0xffffffffffffff00);
      local_2c8[0] = tinyxml2::XMLPrinter::vftable;
      if (local_1b8 != local_1b0) {
        thunk_FUN_18005caec(local_1b8);
      }
      if (local_2b8 != local_2b0) {
        thunk_FUN_18005caec(local_2b8);
      }
    }
    FUN_18001f9a0();
  }
  local_388 = (undefined8 ******)0x0;
  lStack_380 = 0;
  local_378 = 0;
  uStack_370 = 0;
  do {
    uVar22 = uVar22 + 1;
  } while (*(char *)(uVar22 + (longlong)plVar7) != '\0');
  FUN_180006b30(&local_388,plVar7,uVar22);
  FUN_18002c080(local_398,(byte *)&local_388,puVar1);
  if (0xf < uStack_370) {
    ppppppuVar17 = local_388;
    if ((0xfff < uStack_370 + 1) &&
       (ppppppuVar17 = (undefined8 ******)local_388[-1],
       0x1f < (ulonglong)((longlong)local_388 + (-8 - (longlong)ppppppuVar17)))) {
                    /* WARNING: Subroutine does not return */
      FUN_18005c974();
    }
    thunk_FUN_18005caec(ppppppuVar17);
  }
  uVar14 = FUN_18004a6b4(0x1800a3e08);
  return CONCAT71((int7)((ulonglong)uVar14 >> 8),1);
}



