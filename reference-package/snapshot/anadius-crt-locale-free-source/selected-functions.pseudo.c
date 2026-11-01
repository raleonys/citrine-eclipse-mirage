/* 0x1800706c0 __acrt_free_locale; Evidence ev_dfb45e5c4c5e242c804d70a9dd43ed27cd4aba8a1f6518c86bb9428749bf90c1 */

/* Library Function - Single Match
    __acrt_free_locale
   
   Libraries: Visual Studio 2017 Release, Visual Studio 2019 Release */

void __acrt_free_locale(LPVOID param_1)

{
  int *piVar1;
  longlong lVar2;
  undefined8 *puVar3;
  undefined8 *puVar4;
  
  if ((((*(undefined ***)((longlong)param_1 + 0xf8) != (undefined **)0x0) &&
       (*(undefined ***)((longlong)param_1 + 0xf8) != &PTR_DAT_1800a1700)) &&
      (*(int **)((longlong)param_1 + 0xe0) != (int *)0x0)) &&
     (**(int **)((longlong)param_1 + 0xe0) == 0)) {
    piVar1 = *(int **)((longlong)param_1 + 0xf0);
    if ((piVar1 != (int *)0x0) && (*piVar1 == 0)) {
      FUN_1800668c0(piVar1);
      __acrt_locale_free_monetary(*(longlong *)((longlong)param_1 + 0xf8));
    }
    piVar1 = *(int **)((longlong)param_1 + 0xe8);
    if ((piVar1 != (int *)0x0) && (*piVar1 == 0)) {
      FUN_1800668c0(piVar1);
      __acrt_locale_free_numeric(*(undefined8 **)((longlong)param_1 + 0xf8));
    }
    FUN_1800668c0(*(LPVOID *)((longlong)param_1 + 0xe0));
    FUN_1800668c0(*(LPVOID *)((longlong)param_1 + 0xf8));
  }
  if ((*(int **)((longlong)param_1 + 0x100) != (int *)0x0) &&
     (**(int **)((longlong)param_1 + 0x100) == 0)) {
    FUN_1800668c0((LPVOID)(*(longlong *)((longlong)param_1 + 0x108) + -0xfe));
    FUN_1800668c0((LPVOID)(*(longlong *)((longlong)param_1 + 0x110) + -0x80));
    FUN_1800668c0((LPVOID)(*(longlong *)((longlong)param_1 + 0x118) + -0x80));
    FUN_1800668c0(*(LPVOID *)((longlong)param_1 + 0x100));
  }
  __acrt_locale_free_lc_time_if_unreferenced(*(undefined ***)((longlong)param_1 + 0x120));
  puVar3 = (undefined8 *)((longlong)param_1 + 0x128);
  lVar2 = 6;
  puVar4 = (undefined8 *)((longlong)param_1 + 0x38);
  do {
    if ((((undefined *)puVar4[-2] != &DAT_1800a1948) &&
        (piVar1 = (int *)*puVar4, piVar1 != (int *)0x0)) && (*piVar1 == 0)) {
      FUN_1800668c0(piVar1);
      FUN_1800668c0((LPVOID)*puVar3);
    }
    if (((puVar4[-3] != 0) && (piVar1 = (int *)puVar4[-1], piVar1 != (int *)0x0)) && (*piVar1 == 0))
    {
      FUN_1800668c0(piVar1);
    }
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 4;
    lVar2 = lVar2 + -1;
  } while (lVar2 != 0);
  FUN_1800668c0(param_1);
  return;
}




/* 0x180070a40 __acrt_locale_free_monetary; Evidence ev_668ecd85f87cd9f3c4f0756fd946d4c21aa937a6318609791d886b29b4a1f14f */

/* Library Function - Single Match
    __acrt_locale_free_monetary
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __acrt_locale_free_monetary(longlong param_1)

{
  if (param_1 != 0) {
    if (*(undefined **)(param_1 + 0x18) != PTR_DAT_1800a1718) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x18));
    }
    if (*(undefined **)(param_1 + 0x20) != PTR_DAT_1800a1720) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x20));
    }
    if (*(undefined **)(param_1 + 0x28) != PTR_DAT_1800a1728) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x28));
    }
    if (*(undefined **)(param_1 + 0x30) != PTR_DAT_1800a1730) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x30));
    }
    if (*(undefined **)(param_1 + 0x38) != PTR_DAT_1800a1738) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x38));
    }
    if (*(undefined **)(param_1 + 0x40) != PTR_DAT_1800a1740) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x40));
    }
    if (*(undefined **)(param_1 + 0x48) != PTR_DAT_1800a1748) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x48));
    }
    if (*(undefined **)(param_1 + 0x68) != PTR_DAT_1800a1768) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x68));
    }
    if (*(undefined **)(param_1 + 0x70) != PTR_DAT_1800a1770) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x70));
    }
    if (*(undefined **)(param_1 + 0x78) != PTR_DAT_1800a1778) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x78));
    }
    if (*(undefined **)(param_1 + 0x80) != PTR_DAT_1800a1780) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x80));
    }
    if (*(undefined **)(param_1 + 0x88) != PTR_DAT_1800a1788) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x88));
    }
    if (*(undefined **)(param_1 + 0x90) != PTR_DAT_1800a1790) {
      FUN_1800668c0(*(undefined **)(param_1 + 0x90));
    }
  }
  return;
}




/* 0x180071068 __acrt_locale_free_numeric; Evidence ev_72ebac3df15852c23bd9c9b2b2fe06b6585d8a0b1dbab04999111e35298ff0a5 */

/* Library Function - Single Match
    __acrt_locale_free_numeric
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __acrt_locale_free_numeric(undefined8 *param_1)

{
  if (param_1 != (undefined8 *)0x0) {
    if ((undefined *)*param_1 != PTR_DAT_1800a1700) {
      FUN_1800668c0((undefined *)*param_1);
    }
    if ((undefined *)param_1[1] != PTR_DAT_1800a1708) {
      FUN_1800668c0((undefined *)param_1[1]);
    }
    if ((undefined *)param_1[2] != PTR_DAT_1800a1710) {
      FUN_1800668c0((undefined *)param_1[2]);
    }
    if ((undefined *)param_1[0xb] != PTR_DAT_1800a1758) {
      FUN_1800668c0((undefined *)param_1[0xb]);
    }
    if ((undefined *)param_1[0xc] != PTR_DAT_1800a1760) {
      FUN_1800668c0((undefined *)param_1[0xc]);
    }
  }
  return;
}




/* 0x180070860 __acrt_locale_free_lc_time_if_unreferenced; Evidence ev_a093efa34b4b095fb071a2dc5f786e694ffd291dd53fe42f0eca522834c662d3 */

/* Library Function - Single Match
    __acrt_locale_free_lc_time_if_unreferenced
   
   Libraries: Visual Studio 2015 Release, Visual Studio 2017 Release, Visual Studio 2019 Release */

void __acrt_locale_free_lc_time_if_unreferenced(undefined **param_1)

{
  if (((param_1 != (undefined **)0x0) && (param_1 != &PTR_DAT_18008ad20)) &&
     (*(int *)((longlong)param_1 + 0x15c) == 0)) {
    __acrt_locale_free_time(param_1);
    FUN_1800668c0(param_1);
  }
  return;
}




/* 0x180071730 __acrt_locale_free_time; Evidence ev_84c78024a6b69ee549cee89a3c5592e0313838a821f514cb0fbd5c2a705f42b7 */

/* Library Function - Single Match
    __acrt_locale_free_time
   
   Libraries: Visual Studio 2017 Release, Visual Studio 2019 Release */

void __acrt_locale_free_time(undefined8 *param_1)

{
  if (param_1 != (undefined8 *)0x0) {
    FUN_1800713bc(param_1,7);
    FUN_1800713bc(param_1 + 7,7);
    FUN_1800713bc(param_1 + 0xe,0xc);
    FUN_1800713bc(param_1 + 0x1a,0xc);
    FUN_1800713bc(param_1 + 0x26,2);
    FUN_1800668c0((LPVOID)param_1[0x28]);
    FUN_1800668c0((LPVOID)param_1[0x29]);
    FUN_1800668c0((LPVOID)param_1[0x2a]);
    FUN_1800713bc(param_1 + 0x2c,7);
    FUN_1800713bc(param_1 + 0x33,7);
    FUN_1800713bc(param_1 + 0x3a,0xc);
    FUN_1800713bc(param_1 + 0x46,0xc);
    FUN_1800713bc(param_1 + 0x52,2);
    FUN_1800668c0((LPVOID)param_1[0x54]);
    FUN_1800668c0((LPVOID)param_1[0x55]);
    FUN_1800668c0((LPVOID)param_1[0x56]);
    FUN_1800668c0((LPVOID)param_1[0x57]);
  }
  return;
}




/* 0x1800713bc FUN_1800713bc; Evidence ev_5efc80503c52cf75419b8050eb9dcfc5425cdf145bbe40e476853af6f9286e27 */

void FUN_1800713bc(undefined8 *param_1,longlong param_2)

{
  undefined8 *puVar1;
  
  puVar1 = param_1 + param_2;
  for (; param_1 != puVar1; param_1 = param_1 + 1) {
    FUN_1800668c0((LPVOID)*param_1);
  }
  return;
}



