/* 0x18000fd00 FUN_18000fd00; Evidence ev_56829116b2c9c143c2c0fe7d3fa88a54fc6499474948b76b08e8cd2740d33c68 */

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_18000fd00(undefined8 param_1,short *param_2,int param_3)

{
  u_short uVar1;
  longlong *plVar2;
  longlong *plVar3;
  undefined8 local_50 [4];
  undefined8 local_30 [5];
  
  if (((param_3 != DAT_1800a10d0) || (*param_2 != 2)) &&
     ((param_3 != DAT_1800a10d4 || (*param_2 != 0x17)))) goto LAB_18000fde4;
  uVar1 = ntohs(param_2[1]);
  if ((uVar1 == 0x1bb) || (uVar1 == 0x50)) {
LAB_18000fd73:
    if (uVar1 != DAT_1800a3c90) goto LAB_18000fde4;
  }
  else {
    FUN_180036880(-1);
    if (uVar1 != 0xc90) goto LAB_18000fd73;
  }
  plVar2 = FUN_180006880(&DAT_1800a4130,0x180080478);
  plVar2 = FUN_18000ad70(plVar2,uVar1);
  plVar2 = FUN_180006880(plVar2,0x180080460);
  plVar2 = FUN_18000ad70(plVar2,DAT_1800a3c90);
  FUN_1800088e0(plVar2);
  DAT_1800a3888 = DAT_1800a3888 + 1;
  if (DAT_1800a3888 == 0xf) {
    plVar2 = FUN_180009fd0(local_50,(undefined8 *)
                                    "The game tried to connect to the crack 15 times already. Most probably your firewall or anti-virus blocks it. Disable them and try again."
                          );
    plVar3 = FUN_180009fd0(local_30,(undefined8 *)"error");
    FUN_180038bc0(plVar3,plVar2,0x10);
                    /* WARNING: Subroutine does not return */
    FUN_18005cdc0(0xffffffff);
  }
  uVar1 = htons(DAT_1800a3c90);
  param_2[1] = uVar1;
LAB_18000fde4:
                    /* WARNING: Could not recover jumptable at 0x00018000fe02. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*_DAT_1800a3838)(param_1,param_2,param_3);
  return;
}



