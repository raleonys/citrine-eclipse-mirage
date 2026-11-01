; Reconstructed from retained REA/Ghidra wrapper assembly. Do not edit generated file.
option casemap:none
EXTERN __imp_GetProcAddress:QWORD
EXTERN OuterState:QWORD
.const
name_1 BYTE "CloseDriver",0
name_2 BYTE "DefDriverProc",0
name_4 BYTE "DriverCallback",0
name_5 BYTE "DrvGetModuleHandle",0
name_6 BYTE "GetDriverModuleHandle",0
name_7 BYTE "GetFileVersionInfoA",0
name_8 BYTE "GetFileVersionInfoByHandle",0
name_9 BYTE "GetFileVersionInfoExA",0
name_10 BYTE "GetFileVersionInfoExW",0
name_11 BYTE "GetFileVersionInfoSizeA",0
name_12 BYTE "GetFileVersionInfoSizeExA",0
name_13 BYTE "GetFileVersionInfoSizeExW",0
name_14 BYTE "GetFileVersionInfoSizeW",0
name_15 BYTE "GetFileVersionInfoW",0
name_16 BYTE "OpenDriver",0
name_17 BYTE "PlaySound",0
name_18 BYTE "PlaySoundA",0
name_19 BYTE "PlaySoundW",0
name_20 BYTE "SendDriverMessage",0
name_21 BYTE "VerFindFileA",0
name_22 BYTE "VerFindFileW",0
name_23 BYTE "VerInstallFileA",0
name_24 BYTE "VerInstallFileW",0
name_25 BYTE "VerLanguageNameA",0
name_26 BYTE "VerLanguageNameW",0
name_27 BYTE "VerQueryValueA",0
name_28 BYTE "VerQueryValueW",0
name_29 BYTE "WOWAppExit",0
name_30 BYTE "auxGetDevCapsA",0
name_31 BYTE "auxGetDevCapsW",0
name_32 BYTE "auxGetNumDevs",0
name_33 BYTE "auxGetVolume",0
name_34 BYTE "auxOutMessage",0
name_35 BYTE "auxSetVolume",0
name_36 BYTE "joyConfigChanged",0
name_37 BYTE "joyGetDevCapsA",0
name_38 BYTE "joyGetDevCapsW",0
name_39 BYTE "joyGetNumDevs",0
name_40 BYTE "joyGetPos",0
name_41 BYTE "joyGetPosEx",0
name_42 BYTE "joyGetThreshold",0
name_43 BYTE "joyReleaseCapture",0
name_44 BYTE "joySetCapture",0
name_45 BYTE "joySetThreshold",0
name_46 BYTE "mciDriverNotify",0
name_47 BYTE "mciDriverYield",0
name_48 BYTE "mciExecute",0
name_49 BYTE "mciFreeCommandResource",0
name_50 BYTE "mciGetCreatorTask",0
name_51 BYTE "mciGetDeviceIDA",0
name_52 BYTE "mciGetDeviceIDFromElementIDA",0
name_53 BYTE "mciGetDeviceIDFromElementIDW",0
name_54 BYTE "mciGetDeviceIDW",0
name_55 BYTE "mciGetDriverData",0
name_56 BYTE "mciGetErrorStringA",0
name_57 BYTE "mciGetErrorStringW",0
name_58 BYTE "mciGetYieldProc",0
name_59 BYTE "mciLoadCommandResource",0
name_60 BYTE "mciSendCommandA",0
name_61 BYTE "mciSendCommandW",0
name_62 BYTE "mciSendStringA",0
name_63 BYTE "mciSendStringW",0
name_64 BYTE "mciSetDriverData",0
name_65 BYTE "mciSetYieldProc",0
name_66 BYTE "midiConnect",0
name_67 BYTE "midiDisconnect",0
name_68 BYTE "midiInAddBuffer",0
name_69 BYTE "midiInClose",0
name_70 BYTE "midiInGetDevCapsA",0
name_71 BYTE "midiInGetDevCapsW",0
name_72 BYTE "midiInGetErrorTextA",0
name_73 BYTE "midiInGetErrorTextW",0
name_74 BYTE "midiInGetID",0
name_75 BYTE "midiInGetNumDevs",0
name_76 BYTE "midiInMessage",0
name_77 BYTE "midiInOpen",0
name_78 BYTE "midiInPrepareHeader",0
name_79 BYTE "midiInReset",0
name_80 BYTE "midiInStart",0
name_81 BYTE "midiInStop",0
name_82 BYTE "midiInUnprepareHeader",0
name_83 BYTE "midiOutCacheDrumPatches",0
name_84 BYTE "midiOutCachePatches",0
name_85 BYTE "midiOutClose",0
name_86 BYTE "midiOutGetDevCapsA",0
name_87 BYTE "midiOutGetDevCapsW",0
name_88 BYTE "midiOutGetErrorTextA",0
name_89 BYTE "midiOutGetErrorTextW",0
name_90 BYTE "midiOutGetID",0
name_91 BYTE "midiOutGetNumDevs",0
name_92 BYTE "midiOutGetVolume",0
name_93 BYTE "midiOutLongMsg",0
name_94 BYTE "midiOutMessage",0
name_95 BYTE "midiOutOpen",0
name_96 BYTE "midiOutPrepareHeader",0
name_97 BYTE "midiOutReset",0
name_98 BYTE "midiOutSetVolume",0
name_99 BYTE "midiOutShortMsg",0
name_100 BYTE "midiOutUnprepareHeader",0
name_101 BYTE "midiStreamClose",0
name_102 BYTE "midiStreamOpen",0
name_103 BYTE "midiStreamOut",0
name_104 BYTE "midiStreamPause",0
name_105 BYTE "midiStreamPosition",0
name_106 BYTE "midiStreamProperty",0
name_107 BYTE "midiStreamRestart",0
name_108 BYTE "midiStreamStop",0
name_109 BYTE "mixerClose",0
name_110 BYTE "mixerGetControlDetailsA",0
name_111 BYTE "mixerGetControlDetailsW",0
name_112 BYTE "mixerGetDevCapsA",0
name_113 BYTE "mixerGetDevCapsW",0
name_114 BYTE "mixerGetID",0
name_115 BYTE "mixerGetLineControlsA",0
name_116 BYTE "mixerGetLineControlsW",0
name_117 BYTE "mixerGetLineInfoA",0
name_118 BYTE "mixerGetLineInfoW",0
name_119 BYTE "mixerGetNumDevs",0
name_120 BYTE "mixerMessage",0
name_121 BYTE "mixerOpen",0
name_122 BYTE "mixerSetControlDetails",0
name_123 BYTE "mmDrvInstall",0
name_124 BYTE "mmGetCurrentTask",0
name_125 BYTE "mmTaskBlock",0
name_126 BYTE "mmTaskCreate",0
name_127 BYTE "mmTaskSignal",0
name_128 BYTE "mmTaskYield",0
name_129 BYTE "mmioAdvance",0
name_130 BYTE "mmioAscend",0
name_131 BYTE "mmioClose",0
name_132 BYTE "mmioCreateChunk",0
name_133 BYTE "mmioDescend",0
name_134 BYTE "mmioFlush",0
name_135 BYTE "mmioGetInfo",0
name_136 BYTE "mmioInstallIOProcA",0
name_137 BYTE "mmioInstallIOProcW",0
name_138 BYTE "mmioOpenA",0
name_139 BYTE "mmioOpenW",0
name_140 BYTE "mmioRead",0
name_141 BYTE "mmioRenameA",0
name_142 BYTE "mmioRenameW",0
name_143 BYTE "mmioSeek",0
name_144 BYTE "mmioSendMessage",0
name_145 BYTE "mmioSetBuffer",0
name_146 BYTE "mmioSetInfo",0
name_147 BYTE "mmioStringToFOURCCA",0
name_148 BYTE "mmioStringToFOURCCW",0
name_149 BYTE "mmioWrite",0
name_150 BYTE "mmsystemGetVersion",0
name_151 BYTE "sndPlaySoundA",0
name_152 BYTE "sndPlaySoundW",0
name_153 BYTE "timeBeginPeriod",0
name_154 BYTE "timeEndPeriod",0
name_155 BYTE "timeGetDevCaps",0
name_156 BYTE "timeGetSystemTime",0
name_157 BYTE "timeGetTime",0
name_158 BYTE "timeKillEvent",0
name_159 BYTE "timeSetEvent",0
name_160 BYTE "waveInAddBuffer",0
name_161 BYTE "waveInClose",0
name_162 BYTE "waveInGetDevCapsA",0
name_163 BYTE "waveInGetDevCapsW",0
name_164 BYTE "waveInGetErrorTextA",0
name_165 BYTE "waveInGetErrorTextW",0
name_166 BYTE "waveInGetID",0
name_167 BYTE "waveInGetNumDevs",0
name_168 BYTE "waveInGetPosition",0
name_169 BYTE "waveInMessage",0
name_170 BYTE "waveInOpen",0
name_171 BYTE "waveInPrepareHeader",0
name_172 BYTE "waveInReset",0
name_173 BYTE "waveInStart",0
name_174 BYTE "waveInStop",0
name_175 BYTE "waveInUnprepareHeader",0
name_176 BYTE "waveOutBreakLoop",0
name_177 BYTE "waveOutClose",0
name_178 BYTE "waveOutGetDevCapsA",0
name_179 BYTE "waveOutGetDevCapsW",0
name_180 BYTE "waveOutGetErrorTextA",0
name_181 BYTE "waveOutGetErrorTextW",0
name_182 BYTE "waveOutGetID",0
name_183 BYTE "waveOutGetNumDevs",0
name_184 BYTE "waveOutGetPitch",0
name_185 BYTE "waveOutGetPlaybackRate",0
name_186 BYTE "waveOutGetPosition",0
name_187 BYTE "waveOutGetVolume",0
name_188 BYTE "waveOutMessage",0
name_189 BYTE "waveOutOpen",0
name_190 BYTE "waveOutPause",0
name_191 BYTE "waveOutPrepareHeader",0
name_192 BYTE "waveOutReset",0
name_193 BYTE "waveOutRestart",0
name_194 BYTE "waveOutSetPitch",0
name_195 BYTE "waveOutSetPlaybackRate",0
name_196 BYTE "waveOutSetVolume",0
name_197 BYTE "waveOutUnprepareHeader",0
name_198 BYTE "waveOutWrite",0
.code
; CloseDriver @ 1; original VA 0x18000120a
; assembly ev_2665d70bc8ff84fc4eabb6a65c73be4775dea08e18c8e4dfd10cdcb60dbb5710; pseudocode ev_8870a96b088136300aac336461eb8383bf40c81f9742d9b20f151cd76bb495e7
PUBLIC reconstructed_1
reconstructed_1 PROC FRAME
L_18000120a:
    PUSH R14
    .pushreg R14
L_18000120c:
    PUSH RSI
    .pushreg RSI
L_18000120d:
    PUSH RDI
    .pushreg RDI
L_18000120e:
    PUSH RBX
    .pushreg RBX
L_18000120f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001213:
    MOV R14, RCX
L_180001216:
    MOV RCX, qword ptr [OuterState+120]
L_18000121d:
    TEST RCX, RCX
L_180001220:
    JZ L_180001255
L_180001222:
    MOV RSI, R9
L_180001225:
    MOV RDI, R8
L_180001228:
    MOV RBX, RDX
L_18000122b:
    LEA RDX, [name_1]
L_180001232:
    CALL qword ptr [__imp_GetProcAddress]
L_180001238:
    TEST RAX, RAX
L_18000123b:
    JZ L_180001255
L_18000123d:
    MOV RCX, R14
L_180001240:
    MOV RDX, RBX
L_180001243:
    MOV R8, RDI
L_180001246:
    MOV R9, RSI
L_180001249:
    ADD RSP, 028h
L_18000124d:
    POP RBX
L_18000124e:
    POP RDI
L_18000124f:
    POP RSI
L_180001250:
    POP R14
L_180001252:
    JMP RAX
L_180001255:
    XOR EAX, EAX
L_180001257:
    ADD RSP, 028h
L_18000125b:
    POP RBX
L_18000125c:
    POP RDI
L_18000125d:
    POP RSI
L_18000125e:
    POP R14
L_180001260:
    RET
reconstructed_1 ENDP
; DefDriverProc @ 2; original VA 0x180001261
; assembly ev_db504236afbdd9bbe7b996f02406232db5c1ec5c6b5a75fc3f8aa6a253d848f1; pseudocode ev_e2afc2235eb28d2018d16247b939f7feefa34b3dafd97f15fe3c49d7db36684c
PUBLIC reconstructed_2
reconstructed_2 PROC FRAME
L_180001261:
    PUSH R14
    .pushreg R14
L_180001263:
    PUSH RSI
    .pushreg RSI
L_180001264:
    PUSH RDI
    .pushreg RDI
L_180001265:
    PUSH RBX
    .pushreg RBX
L_180001266:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000126a:
    MOV R14, RCX
L_18000126d:
    MOV RCX, qword ptr [OuterState+120]
L_180001274:
    TEST RCX, RCX
L_180001277:
    JZ L_1800012ac
L_180001279:
    MOV RSI, R9
L_18000127c:
    MOV RDI, R8
L_18000127f:
    MOV RBX, RDX
L_180001282:
    LEA RDX, [name_2]
L_180001289:
    CALL qword ptr [__imp_GetProcAddress]
L_18000128f:
    TEST RAX, RAX
L_180001292:
    JZ L_1800012ac
L_180001294:
    MOV RCX, R14
L_180001297:
    MOV RDX, RBX
L_18000129a:
    MOV R8, RDI
L_18000129d:
    MOV R9, RSI
L_1800012a0:
    ADD RSP, 028h
L_1800012a4:
    POP RBX
L_1800012a5:
    POP RDI
L_1800012a6:
    POP RSI
L_1800012a7:
    POP R14
L_1800012a9:
    JMP RAX
L_1800012ac:
    XOR EAX, EAX
L_1800012ae:
    ADD RSP, 028h
L_1800012b2:
    POP RBX
L_1800012b3:
    POP RDI
L_1800012b4:
    POP RSI
L_1800012b5:
    POP R14
L_1800012b7:
    RET
reconstructed_2 ENDP
; DriverCallback @ 4; original VA 0x1800012b8
; assembly ev_3b6d3fec3bdc0eb70a532b62f66a580d3d0c78ea8146ca88926355b54ae823b1; pseudocode ev_8e9b96a763b0575263306fdaf672a9bf52cfad7cd06275c6f33d3242158793c4
PUBLIC reconstructed_4
reconstructed_4 PROC FRAME
L_1800012b8:
    PUSH R14
    .pushreg R14
L_1800012ba:
    PUSH RSI
    .pushreg RSI
L_1800012bb:
    PUSH RDI
    .pushreg RDI
L_1800012bc:
    PUSH RBX
    .pushreg RBX
L_1800012bd:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800012c1:
    MOV R14, RCX
L_1800012c4:
    MOV RCX, qword ptr [OuterState+120]
L_1800012cb:
    TEST RCX, RCX
L_1800012ce:
    JZ L_180001303
L_1800012d0:
    MOV RSI, R9
L_1800012d3:
    MOV RDI, R8
L_1800012d6:
    MOV RBX, RDX
L_1800012d9:
    LEA RDX, [name_4]
L_1800012e0:
    CALL qword ptr [__imp_GetProcAddress]
L_1800012e6:
    TEST RAX, RAX
L_1800012e9:
    JZ L_180001303
L_1800012eb:
    MOV RCX, R14
L_1800012ee:
    MOV RDX, RBX
L_1800012f1:
    MOV R8, RDI
L_1800012f4:
    MOV R9, RSI
L_1800012f7:
    ADD RSP, 028h
L_1800012fb:
    POP RBX
L_1800012fc:
    POP RDI
L_1800012fd:
    POP RSI
L_1800012fe:
    POP R14
L_180001300:
    JMP RAX
L_180001303:
    XOR EAX, EAX
L_180001305:
    ADD RSP, 028h
L_180001309:
    POP RBX
L_18000130a:
    POP RDI
L_18000130b:
    POP RSI
L_18000130c:
    POP R14
L_18000130e:
    RET
reconstructed_4 ENDP
; DrvGetModuleHandle @ 5; original VA 0x18000130f
; assembly ev_1bb66cf94171a2c6a03b2d63ca80fe64a5ff221f066320b006ffd7c0f8ebfb4e; pseudocode ev_fe52a1160f76e01319c9005e83ab6db7bedeea88f9f3c1dfc856e6b96af27200
PUBLIC reconstructed_5
reconstructed_5 PROC FRAME
L_18000130f:
    PUSH R14
    .pushreg R14
L_180001311:
    PUSH RSI
    .pushreg RSI
L_180001312:
    PUSH RDI
    .pushreg RDI
L_180001313:
    PUSH RBX
    .pushreg RBX
L_180001314:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001318:
    MOV R14, RCX
L_18000131b:
    MOV RCX, qword ptr [OuterState+120]
L_180001322:
    TEST RCX, RCX
L_180001325:
    JZ L_18000135a
L_180001327:
    MOV RSI, R9
L_18000132a:
    MOV RDI, R8
L_18000132d:
    MOV RBX, RDX
L_180001330:
    LEA RDX, [name_5]
L_180001337:
    CALL qword ptr [__imp_GetProcAddress]
L_18000133d:
    TEST RAX, RAX
L_180001340:
    JZ L_18000135a
L_180001342:
    MOV RCX, R14
L_180001345:
    MOV RDX, RBX
L_180001348:
    MOV R8, RDI
L_18000134b:
    MOV R9, RSI
L_18000134e:
    ADD RSP, 028h
L_180001352:
    POP RBX
L_180001353:
    POP RDI
L_180001354:
    POP RSI
L_180001355:
    POP R14
L_180001357:
    JMP RAX
L_18000135a:
    XOR EAX, EAX
L_18000135c:
    ADD RSP, 028h
L_180001360:
    POP RBX
L_180001361:
    POP RDI
L_180001362:
    POP RSI
L_180001363:
    POP R14
L_180001365:
    RET
reconstructed_5 ENDP
; GetDriverModuleHandle @ 6; original VA 0x180001366
; assembly ev_d310e86a766e93c3b782d5a4b1164bc53ce34d188ed473ead8e174f8c442b6a9; pseudocode ev_67acdaa0a44e12d6c8d7a58417ed99189a5f57852f8d41fd36505801073895bd
PUBLIC reconstructed_6
reconstructed_6 PROC FRAME
L_180001366:
    PUSH R14
    .pushreg R14
L_180001368:
    PUSH RSI
    .pushreg RSI
L_180001369:
    PUSH RDI
    .pushreg RDI
L_18000136a:
    PUSH RBX
    .pushreg RBX
L_18000136b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000136f:
    MOV R14, RCX
L_180001372:
    MOV RCX, qword ptr [OuterState+120]
L_180001379:
    TEST RCX, RCX
L_18000137c:
    JZ L_1800013b1
L_18000137e:
    MOV RSI, R9
L_180001381:
    MOV RDI, R8
L_180001384:
    MOV RBX, RDX
L_180001387:
    LEA RDX, [name_6]
L_18000138e:
    CALL qword ptr [__imp_GetProcAddress]
L_180001394:
    TEST RAX, RAX
L_180001397:
    JZ L_1800013b1
L_180001399:
    MOV RCX, R14
L_18000139c:
    MOV RDX, RBX
L_18000139f:
    MOV R8, RDI
L_1800013a2:
    MOV R9, RSI
L_1800013a5:
    ADD RSP, 028h
L_1800013a9:
    POP RBX
L_1800013aa:
    POP RDI
L_1800013ab:
    POP RSI
L_1800013ac:
    POP R14
L_1800013ae:
    JMP RAX
L_1800013b1:
    XOR EAX, EAX
L_1800013b3:
    ADD RSP, 028h
L_1800013b7:
    POP RBX
L_1800013b8:
    POP RDI
L_1800013b9:
    POP RSI
L_1800013ba:
    POP R14
L_1800013bc:
    RET
reconstructed_6 ENDP
; GetFileVersionInfoA @ 7; original VA 0x180004dda
; assembly ev_a24cbdadca5716aeebe1881854f40956649087d443a46b51c2953b1336a18b9e; pseudocode ev_4badacd0a18495fbee48af719a5d0ad96eef78cc25dab09b55548300bf83201b
PUBLIC reconstructed_7
reconstructed_7 PROC FRAME
L_180004dda:
    PUSH R14
    .pushreg R14
L_180004ddc:
    PUSH RSI
    .pushreg RSI
L_180004ddd:
    PUSH RDI
    .pushreg RDI
L_180004dde:
    PUSH RBX
    .pushreg RBX
L_180004ddf:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004de3:
    MOV RSI, R9
L_180004de6:
    MOV EDI, R8D
L_180004de9:
    MOV EBX, EDX
L_180004deb:
    MOV R14, RCX
L_180004dee:
    MOV RAX, qword ptr [OuterState+0]
L_180004df5:
    TEST RAX, RAX
L_180004df8:
    JZ L_180004e11
L_180004dfa:
    MOV RCX, R14
L_180004dfd:
    MOV EDX, EBX
L_180004dff:
    MOV R8D, EDI
L_180004e02:
    MOV R9, RSI
L_180004e05:
    ADD RSP, 028h
L_180004e09:
    POP RBX
L_180004e0a:
    POP RDI
L_180004e0b:
    POP RSI
L_180004e0c:
    POP R14
L_180004e0e:
    JMP RAX
L_180004e11:
    MOV RCX, qword ptr [OuterState+136]
L_180004e18:
    TEST RCX, RCX
L_180004e1b:
    JZ L_180004e38
L_180004e1d:
    LEA RDX, [name_7]
L_180004e24:
    CALL qword ptr [__imp_GetProcAddress]
L_180004e2a:
    MOV qword ptr [OuterState+0], RAX
L_180004e31:
    TEST RAX, RAX
L_180004e34:
    JNZ L_180004dfa
L_180004e36:
    JMP L_180004e43
L_180004e38:
    MOV qword ptr [OuterState+0], 00h
L_180004e43:
    XOR EAX, EAX
L_180004e45:
    ADD RSP, 028h
L_180004e49:
    POP RBX
L_180004e4a:
    POP RDI
L_180004e4b:
    POP RSI
L_180004e4c:
    POP R14
L_180004e4e:
    RET
reconstructed_7 ENDP
; GetFileVersionInfoByHandle @ 8; original VA 0x180004e4f
; assembly ev_4c5dd50a9e0952162d375aa2e2679d882ada9f8074c699cac73f8de527bf2f65; pseudocode ev_0a423c19bf9fe32db3d62b43757b664eb987b1cea7d412b31982519b5db65bad
PUBLIC reconstructed_8
reconstructed_8 PROC FRAME
L_180004e4f:
    PUSH RSI
    .pushreg RSI
L_180004e50:
    PUSH RDI
    .pushreg RDI
L_180004e51:
    PUSH RBX
    .pushreg RBX
L_180004e52:
    SUB RSP, 020h
    .allocstack 020h
    .endprolog
L_180004e56:
    MOV RSI, R8
L_180004e59:
    MOV EDI, EDX
L_180004e5b:
    MOV RBX, RCX
L_180004e5e:
    MOV RAX, qword ptr [OuterState+8]
L_180004e65:
    TEST RAX, RAX
L_180004e68:
    JZ L_180004e7c
L_180004e6a:
    MOV RCX, RBX
L_180004e6d:
    MOV EDX, EDI
L_180004e6f:
    MOV R8, RSI
L_180004e72:
    ADD RSP, 020h
L_180004e76:
    POP RBX
L_180004e77:
    POP RDI
L_180004e78:
    POP RSI
L_180004e79:
    JMP RAX
L_180004e7c:
    MOV RCX, qword ptr [OuterState+136]
L_180004e83:
    TEST RCX, RCX
L_180004e86:
    JZ L_180004ea3
L_180004e88:
    LEA RDX, [name_8]
L_180004e8f:
    CALL qword ptr [__imp_GetProcAddress]
L_180004e95:
    MOV qword ptr [OuterState+8], RAX
L_180004e9c:
    TEST RAX, RAX
L_180004e9f:
    JNZ L_180004e6a
L_180004ea1:
    JMP L_180004eae
L_180004ea3:
    MOV qword ptr [OuterState+8], 00h
L_180004eae:
    XOR EAX, EAX
L_180004eb0:
    ADD RSP, 020h
L_180004eb4:
    POP RBX
L_180004eb5:
    POP RDI
L_180004eb6:
    POP RSI
L_180004eb7:
    RET
reconstructed_8 ENDP
; GetFileVersionInfoExA @ 9; original VA 0x180004eb8
; assembly ev_81ff9a722ab781b03b487a96ca657d530d73912bae68843fc4c18cf42b732ebd; pseudocode ev_b6f75f6277bb6d9de363806e9e4d5107948dc3eb98913d3e78d39aa1b63c82a5
PUBLIC reconstructed_9
reconstructed_9 PROC FRAME
L_180004eb8:
    PUSH RSI
    .pushreg RSI
L_180004eb9:
    PUSH RDI
    .pushreg RDI
L_180004eba:
    PUSH RBP
    .pushreg RBP
L_180004ebb:
    PUSH RBX
    .pushreg RBX
L_180004ebc:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004ec0:
    MOV ESI, R9D
L_180004ec3:
    MOV EDI, R8D
L_180004ec6:
    MOV RBX, RDX
L_180004ec9:
    MOV EBP, ECX
L_180004ecb:
    MOV RAX, qword ptr [OuterState+16]
L_180004ed2:
    TEST RAX, RAX
L_180004ed5:
    JZ L_180004eed
L_180004ed7:
    MOV ECX, EBP
L_180004ed9:
    MOV RDX, RBX
L_180004edc:
    MOV R8D, EDI
L_180004edf:
    MOV R9D, ESI
L_180004ee2:
    ADD RSP, 028h
L_180004ee6:
    POP RBX
L_180004ee7:
    POP RBP
L_180004ee8:
    POP RDI
L_180004ee9:
    POP RSI
L_180004eea:
    JMP RAX
L_180004eed:
    MOV RCX, qword ptr [OuterState+136]
L_180004ef4:
    TEST RCX, RCX
L_180004ef7:
    JZ L_180004f14
L_180004ef9:
    LEA RDX, [name_9]
L_180004f00:
    CALL qword ptr [__imp_GetProcAddress]
L_180004f06:
    MOV qword ptr [OuterState+16], RAX
L_180004f0d:
    TEST RAX, RAX
L_180004f10:
    JNZ L_180004ed7
L_180004f12:
    JMP L_180004f1f
L_180004f14:
    MOV qword ptr [OuterState+16], 00h
L_180004f1f:
    XOR EAX, EAX
L_180004f21:
    ADD RSP, 028h
L_180004f25:
    POP RBX
L_180004f26:
    POP RBP
L_180004f27:
    POP RDI
L_180004f28:
    POP RSI
L_180004f29:
    RET
reconstructed_9 ENDP
; GetFileVersionInfoExW @ 10; original VA 0x180004f2a
; assembly ev_e478ef8eccd7c3dc32fe4b37c054403e81f0c134e631689c48768436818233dc; pseudocode ev_4f2925c76e5c824ab7eb764d2d3020f3078853c2d6eda5b618c244ae17427958
PUBLIC reconstructed_10
reconstructed_10 PROC FRAME
L_180004f2a:
    PUSH RSI
    .pushreg RSI
L_180004f2b:
    PUSH RDI
    .pushreg RDI
L_180004f2c:
    PUSH RBP
    .pushreg RBP
L_180004f2d:
    PUSH RBX
    .pushreg RBX
L_180004f2e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004f32:
    MOV ESI, R9D
L_180004f35:
    MOV EDI, R8D
L_180004f38:
    MOV RBX, RDX
L_180004f3b:
    MOV EBP, ECX
L_180004f3d:
    MOV RAX, qword ptr [OuterState+24]
L_180004f44:
    TEST RAX, RAX
L_180004f47:
    JZ L_180004f5f
L_180004f49:
    MOV ECX, EBP
L_180004f4b:
    MOV RDX, RBX
L_180004f4e:
    MOV R8D, EDI
L_180004f51:
    MOV R9D, ESI
L_180004f54:
    ADD RSP, 028h
L_180004f58:
    POP RBX
L_180004f59:
    POP RBP
L_180004f5a:
    POP RDI
L_180004f5b:
    POP RSI
L_180004f5c:
    JMP RAX
L_180004f5f:
    MOV RCX, qword ptr [OuterState+136]
L_180004f66:
    TEST RCX, RCX
L_180004f69:
    JZ L_180004f86
L_180004f6b:
    LEA RDX, [name_10]
L_180004f72:
    CALL qword ptr [__imp_GetProcAddress]
L_180004f78:
    MOV qword ptr [OuterState+24], RAX
L_180004f7f:
    TEST RAX, RAX
L_180004f82:
    JNZ L_180004f49
L_180004f84:
    JMP L_180004f91
L_180004f86:
    MOV qword ptr [OuterState+24], 00h
L_180004f91:
    XOR EAX, EAX
L_180004f93:
    ADD RSP, 028h
L_180004f97:
    POP RBX
L_180004f98:
    POP RBP
L_180004f99:
    POP RDI
L_180004f9a:
    POP RSI
L_180004f9b:
    RET
reconstructed_10 ENDP
; GetFileVersionInfoSizeA @ 11; original VA 0x180004f9c
; assembly ev_6f807d8577f093b1200f7f12d32e24287c4ee19a009d8780d4fe059e2e44bd41; pseudocode ev_6d39d509877f15fc1cf6d6d99728a5943e65017ef27eb8c34e86a62fde02b479
PUBLIC reconstructed_11
reconstructed_11 PROC FRAME
L_180004f9c:
    PUSH RSI
    .pushreg RSI
L_180004f9d:
    PUSH RDI
    .pushreg RDI
L_180004f9e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004fa2:
    MOV RSI, RDX
L_180004fa5:
    MOV RDI, RCX
L_180004fa8:
    MOV RAX, qword ptr [OuterState+32]
L_180004faf:
    TEST RAX, RAX
L_180004fb2:
    JZ L_180004fc3
L_180004fb4:
    MOV RCX, RDI
L_180004fb7:
    MOV RDX, RSI
L_180004fba:
    ADD RSP, 028h
L_180004fbe:
    POP RDI
L_180004fbf:
    POP RSI
L_180004fc0:
    JMP RAX
L_180004fc3:
    MOV RCX, qword ptr [OuterState+136]
L_180004fca:
    TEST RCX, RCX
L_180004fcd:
    JZ L_180004fea
L_180004fcf:
    LEA RDX, [name_11]
L_180004fd6:
    CALL qword ptr [__imp_GetProcAddress]
L_180004fdc:
    MOV qword ptr [OuterState+32], RAX
L_180004fe3:
    TEST RAX, RAX
L_180004fe6:
    JNZ L_180004fb4
L_180004fe8:
    JMP L_180004ff5
L_180004fea:
    MOV qword ptr [OuterState+32], 00h
L_180004ff5:
    XOR EAX, EAX
L_180004ff7:
    ADD RSP, 028h
L_180004ffb:
    POP RDI
L_180004ffc:
    POP RSI
L_180004ffd:
    RET
reconstructed_11 ENDP
; GetFileVersionInfoSizeExA @ 12; original VA 0x180004ffe
; assembly ev_acd6fd303bf36715eeb7be71979af077ca3703fe1fcb919942975140f06643b5; pseudocode ev_6473d3c0d1b515ff22ca2f1aa20c4f1cd087b39120e544c7b3977e263c1d10c0
PUBLIC reconstructed_12
reconstructed_12 PROC FRAME
L_180004ffe:
    PUSH RSI
    .pushreg RSI
L_180004fff:
    PUSH RDI
    .pushreg RDI
L_180005000:
    PUSH RBX
    .pushreg RBX
L_180005001:
    SUB RSP, 020h
    .allocstack 020h
    .endprolog
L_180005005:
    MOV RSI, R8
L_180005008:
    MOV RDI, RDX
L_18000500b:
    MOV EBX, ECX
L_18000500d:
    MOV RAX, qword ptr [OuterState+40]
L_180005014:
    TEST RAX, RAX
L_180005017:
    JZ L_18000502b
L_180005019:
    MOV ECX, EBX
L_18000501b:
    MOV RDX, RDI
L_18000501e:
    MOV R8, RSI
L_180005021:
    ADD RSP, 020h
L_180005025:
    POP RBX
L_180005026:
    POP RDI
L_180005027:
    POP RSI
L_180005028:
    JMP RAX
L_18000502b:
    MOV RCX, qword ptr [OuterState+136]
L_180005032:
    TEST RCX, RCX
L_180005035:
    JZ L_180005052
L_180005037:
    LEA RDX, [name_12]
L_18000503e:
    CALL qword ptr [__imp_GetProcAddress]
L_180005044:
    MOV qword ptr [OuterState+40], RAX
L_18000504b:
    TEST RAX, RAX
L_18000504e:
    JNZ L_180005019
L_180005050:
    JMP L_18000505d
L_180005052:
    MOV qword ptr [OuterState+40], 00h
L_18000505d:
    XOR EAX, EAX
L_18000505f:
    ADD RSP, 020h
L_180005063:
    POP RBX
L_180005064:
    POP RDI
L_180005065:
    POP RSI
L_180005066:
    RET
reconstructed_12 ENDP
; GetFileVersionInfoSizeExW @ 13; original VA 0x180005067
; assembly ev_87709700c54d9ce446d92df843785060cd8ee14afc284e81edb00146534dbb51; pseudocode ev_8deb847725f53a24a33af422c50f110d16af09c6d1d09763075adc30edd55513
PUBLIC reconstructed_13
reconstructed_13 PROC FRAME
L_180005067:
    PUSH RSI
    .pushreg RSI
L_180005068:
    PUSH RDI
    .pushreg RDI
L_180005069:
    PUSH RBX
    .pushreg RBX
L_18000506a:
    SUB RSP, 020h
    .allocstack 020h
    .endprolog
L_18000506e:
    MOV RSI, R8
L_180005071:
    MOV RDI, RDX
L_180005074:
    MOV EBX, ECX
L_180005076:
    MOV RAX, qword ptr [OuterState+48]
L_18000507d:
    TEST RAX, RAX
L_180005080:
    JZ L_180005094
L_180005082:
    MOV ECX, EBX
L_180005084:
    MOV RDX, RDI
L_180005087:
    MOV R8, RSI
L_18000508a:
    ADD RSP, 020h
L_18000508e:
    POP RBX
L_18000508f:
    POP RDI
L_180005090:
    POP RSI
L_180005091:
    JMP RAX
L_180005094:
    MOV RCX, qword ptr [OuterState+136]
L_18000509b:
    TEST RCX, RCX
L_18000509e:
    JZ L_1800050bb
L_1800050a0:
    LEA RDX, [name_13]
L_1800050a7:
    CALL qword ptr [__imp_GetProcAddress]
L_1800050ad:
    MOV qword ptr [OuterState+48], RAX
L_1800050b4:
    TEST RAX, RAX
L_1800050b7:
    JNZ L_180005082
L_1800050b9:
    JMP L_1800050c6
L_1800050bb:
    MOV qword ptr [OuterState+48], 00h
L_1800050c6:
    XOR EAX, EAX
L_1800050c8:
    ADD RSP, 020h
L_1800050cc:
    POP RBX
L_1800050cd:
    POP RDI
L_1800050ce:
    POP RSI
L_1800050cf:
    RET
reconstructed_13 ENDP
; GetFileVersionInfoSizeW @ 14; original VA 0x1800050d0
; assembly ev_8a2375e11ed1756d5d7cce3f8598eada62ac62071ddb0b413e1eca2e8f2a9d4e; pseudocode ev_ed9d1a1c9f678024ee100c7420b155e17bbebdc13829335ed950da174ca58a93
PUBLIC reconstructed_14
reconstructed_14 PROC FRAME
L_1800050d0:
    PUSH RSI
    .pushreg RSI
L_1800050d1:
    PUSH RDI
    .pushreg RDI
L_1800050d2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800050d6:
    MOV RSI, RDX
L_1800050d9:
    MOV RDI, RCX
L_1800050dc:
    MOV RAX, qword ptr [OuterState+56]
L_1800050e3:
    TEST RAX, RAX
L_1800050e6:
    JZ L_1800050f7
L_1800050e8:
    MOV RCX, RDI
L_1800050eb:
    MOV RDX, RSI
L_1800050ee:
    ADD RSP, 028h
L_1800050f2:
    POP RDI
L_1800050f3:
    POP RSI
L_1800050f4:
    JMP RAX
L_1800050f7:
    MOV RCX, qword ptr [OuterState+136]
L_1800050fe:
    TEST RCX, RCX
L_180005101:
    JZ L_18000511e
L_180005103:
    LEA RDX, [name_14]
L_18000510a:
    CALL qword ptr [__imp_GetProcAddress]
L_180005110:
    MOV qword ptr [OuterState+56], RAX
L_180005117:
    TEST RAX, RAX
L_18000511a:
    JNZ L_1800050e8
L_18000511c:
    JMP L_180005129
L_18000511e:
    MOV qword ptr [OuterState+56], 00h
L_180005129:
    XOR EAX, EAX
L_18000512b:
    ADD RSP, 028h
L_18000512f:
    POP RDI
L_180005130:
    POP RSI
L_180005131:
    RET
reconstructed_14 ENDP
; GetFileVersionInfoW @ 15; original VA 0x180005132
; assembly ev_b627299ce403dd57d030e0acf68c2ec54f5e9d2346db06a42554c2cbd53242e0; pseudocode ev_4f241992b119837f3fd58675efefbbe9fed0541b84dafcac055972dfb5fec4d9
PUBLIC reconstructed_15
reconstructed_15 PROC FRAME
L_180005132:
    PUSH R14
    .pushreg R14
L_180005134:
    PUSH RSI
    .pushreg RSI
L_180005135:
    PUSH RDI
    .pushreg RDI
L_180005136:
    PUSH RBX
    .pushreg RBX
L_180005137:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000513b:
    MOV RSI, R9
L_18000513e:
    MOV EDI, R8D
L_180005141:
    MOV EBX, EDX
L_180005143:
    MOV R14, RCX
L_180005146:
    MOV RAX, qword ptr [OuterState+64]
L_18000514d:
    TEST RAX, RAX
L_180005150:
    JZ L_180005169
L_180005152:
    MOV RCX, R14
L_180005155:
    MOV EDX, EBX
L_180005157:
    MOV R8D, EDI
L_18000515a:
    MOV R9, RSI
L_18000515d:
    ADD RSP, 028h
L_180005161:
    POP RBX
L_180005162:
    POP RDI
L_180005163:
    POP RSI
L_180005164:
    POP R14
L_180005166:
    JMP RAX
L_180005169:
    MOV RCX, qword ptr [OuterState+136]
L_180005170:
    TEST RCX, RCX
L_180005173:
    JZ L_180005190
L_180005175:
    LEA RDX, [name_15]
L_18000517c:
    CALL qword ptr [__imp_GetProcAddress]
L_180005182:
    MOV qword ptr [OuterState+64], RAX
L_180005189:
    TEST RAX, RAX
L_18000518c:
    JNZ L_180005152
L_18000518e:
    JMP L_18000519b
L_180005190:
    MOV qword ptr [OuterState+64], 00h
L_18000519b:
    XOR EAX, EAX
L_18000519d:
    ADD RSP, 028h
L_1800051a1:
    POP RBX
L_1800051a2:
    POP RDI
L_1800051a3:
    POP RSI
L_1800051a4:
    POP R14
L_1800051a6:
    RET
reconstructed_15 ENDP
; OpenDriver @ 16; original VA 0x180003ad2
; assembly ev_b754c15c943fb170be27880eb53d4970d25be1e9fc44a0384eec19045f1d503f; pseudocode ev_714310d4db05beaf2995c5b5de71df425e0028ec10e9751e27b20feea8fd2b27
PUBLIC reconstructed_16
reconstructed_16 PROC FRAME
L_180003ad2:
    PUSH R14
    .pushreg R14
L_180003ad4:
    PUSH RSI
    .pushreg RSI
L_180003ad5:
    PUSH RDI
    .pushreg RDI
L_180003ad6:
    PUSH RBX
    .pushreg RBX
L_180003ad7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003adb:
    MOV R14, RCX
L_180003ade:
    MOV RCX, qword ptr [OuterState+120]
L_180003ae5:
    TEST RCX, RCX
L_180003ae8:
    JZ L_180003b1d
L_180003aea:
    MOV RSI, R9
L_180003aed:
    MOV RDI, R8
L_180003af0:
    MOV RBX, RDX
L_180003af3:
    LEA RDX, [name_16]
L_180003afa:
    CALL qword ptr [__imp_GetProcAddress]
L_180003b00:
    TEST RAX, RAX
L_180003b03:
    JZ L_180003b1d
L_180003b05:
    MOV RCX, R14
L_180003b08:
    MOV RDX, RBX
L_180003b0b:
    MOV R8, RDI
L_180003b0e:
    MOV R9, RSI
L_180003b11:
    ADD RSP, 028h
L_180003b15:
    POP RBX
L_180003b16:
    POP RDI
L_180003b17:
    POP RSI
L_180003b18:
    POP R14
L_180003b1a:
    JMP RAX
L_180003b1d:
    XOR EAX, EAX
L_180003b1f:
    ADD RSP, 028h
L_180003b23:
    POP RBX
L_180003b24:
    POP RDI
L_180003b25:
    POP RSI
L_180003b26:
    POP R14
L_180003b28:
    RET
reconstructed_16 ENDP
; PlaySound @ 17; original VA 0x180003b29
; assembly ev_744e04df894acbdbe5b478a2270725c426c6138107da586ea1bddc01cc56a3a4; pseudocode ev_9634d73a443f340f865e2e8a8317c62e4c72e6d0395cdd1ea575c685bbd9fe1c
PUBLIC reconstructed_17
reconstructed_17 PROC FRAME
L_180003b29:
    PUSH R14
    .pushreg R14
L_180003b2b:
    PUSH RSI
    .pushreg RSI
L_180003b2c:
    PUSH RDI
    .pushreg RDI
L_180003b2d:
    PUSH RBX
    .pushreg RBX
L_180003b2e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003b32:
    MOV R14, RCX
L_180003b35:
    MOV RCX, qword ptr [OuterState+120]
L_180003b3c:
    TEST RCX, RCX
L_180003b3f:
    JZ L_180003b74
L_180003b41:
    MOV RSI, R9
L_180003b44:
    MOV RDI, R8
L_180003b47:
    MOV RBX, RDX
L_180003b4a:
    LEA RDX, [name_17]
L_180003b51:
    CALL qword ptr [__imp_GetProcAddress]
L_180003b57:
    TEST RAX, RAX
L_180003b5a:
    JZ L_180003b74
L_180003b5c:
    MOV RCX, R14
L_180003b5f:
    MOV RDX, RBX
L_180003b62:
    MOV R8, RDI
L_180003b65:
    MOV R9, RSI
L_180003b68:
    ADD RSP, 028h
L_180003b6c:
    POP RBX
L_180003b6d:
    POP RDI
L_180003b6e:
    POP RSI
L_180003b6f:
    POP R14
L_180003b71:
    JMP RAX
L_180003b74:
    XOR EAX, EAX
L_180003b76:
    ADD RSP, 028h
L_180003b7a:
    POP RBX
L_180003b7b:
    POP RDI
L_180003b7c:
    POP RSI
L_180003b7d:
    POP R14
L_180003b7f:
    RET
reconstructed_17 ENDP
; PlaySoundA @ 18; original VA 0x180003b80
; assembly ev_5b868c844de3b8cddb968b6121fe083b3e0369136a0f4193ba76aadd5bac9bf6; pseudocode ev_5a02164283b8b0cadfad579574d4a0b4e0402b3e2e3ba79b06b955d0c58ffa31
PUBLIC reconstructed_18
reconstructed_18 PROC FRAME
L_180003b80:
    PUSH R14
    .pushreg R14
L_180003b82:
    PUSH RSI
    .pushreg RSI
L_180003b83:
    PUSH RDI
    .pushreg RDI
L_180003b84:
    PUSH RBX
    .pushreg RBX
L_180003b85:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003b89:
    MOV R14, RCX
L_180003b8c:
    MOV RCX, qword ptr [OuterState+120]
L_180003b93:
    TEST RCX, RCX
L_180003b96:
    JZ L_180003bcb
L_180003b98:
    MOV RSI, R9
L_180003b9b:
    MOV RDI, R8
L_180003b9e:
    MOV RBX, RDX
L_180003ba1:
    LEA RDX, [name_18]
L_180003ba8:
    CALL qword ptr [__imp_GetProcAddress]
L_180003bae:
    TEST RAX, RAX
L_180003bb1:
    JZ L_180003bcb
L_180003bb3:
    MOV RCX, R14
L_180003bb6:
    MOV RDX, RBX
L_180003bb9:
    MOV R8, RDI
L_180003bbc:
    MOV R9, RSI
L_180003bbf:
    ADD RSP, 028h
L_180003bc3:
    POP RBX
L_180003bc4:
    POP RDI
L_180003bc5:
    POP RSI
L_180003bc6:
    POP R14
L_180003bc8:
    JMP RAX
L_180003bcb:
    XOR EAX, EAX
L_180003bcd:
    ADD RSP, 028h
L_180003bd1:
    POP RBX
L_180003bd2:
    POP RDI
L_180003bd3:
    POP RSI
L_180003bd4:
    POP R14
L_180003bd6:
    RET
reconstructed_18 ENDP
; PlaySoundW @ 19; original VA 0x180003bd7
; assembly ev_dc20e16b96ab164da066112440903c4036b1d0d9813d439185016826e16f0c89; pseudocode ev_8a509433cc9c231156b43a9479e4de4afd0e3c9df664508538654b25e2e11535
PUBLIC reconstructed_19
reconstructed_19 PROC FRAME
L_180003bd7:
    PUSH R14
    .pushreg R14
L_180003bd9:
    PUSH RSI
    .pushreg RSI
L_180003bda:
    PUSH RDI
    .pushreg RDI
L_180003bdb:
    PUSH RBX
    .pushreg RBX
L_180003bdc:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003be0:
    MOV R14, RCX
L_180003be3:
    MOV RCX, qword ptr [OuterState+120]
L_180003bea:
    TEST RCX, RCX
L_180003bed:
    JZ L_180003c22
L_180003bef:
    MOV RSI, R9
L_180003bf2:
    MOV RDI, R8
L_180003bf5:
    MOV RBX, RDX
L_180003bf8:
    LEA RDX, [name_19]
L_180003bff:
    CALL qword ptr [__imp_GetProcAddress]
L_180003c05:
    TEST RAX, RAX
L_180003c08:
    JZ L_180003c22
L_180003c0a:
    MOV RCX, R14
L_180003c0d:
    MOV RDX, RBX
L_180003c10:
    MOV R8, RDI
L_180003c13:
    MOV R9, RSI
L_180003c16:
    ADD RSP, 028h
L_180003c1a:
    POP RBX
L_180003c1b:
    POP RDI
L_180003c1c:
    POP RSI
L_180003c1d:
    POP R14
L_180003c1f:
    JMP RAX
L_180003c22:
    XOR EAX, EAX
L_180003c24:
    ADD RSP, 028h
L_180003c28:
    POP RBX
L_180003c29:
    POP RDI
L_180003c2a:
    POP RSI
L_180003c2b:
    POP R14
L_180003c2d:
    RET
reconstructed_19 ENDP
; SendDriverMessage @ 20; original VA 0x180003c2e
; assembly ev_88bb54c02e8249466490ce37404dd9583d119d6b07e83e228d983fdd63f37aa0; pseudocode ev_9073602d634f382267f3f2862bdbf33b9256740bf37d4bbbc205cd6e019e0d84
PUBLIC reconstructed_20
reconstructed_20 PROC FRAME
L_180003c2e:
    PUSH R14
    .pushreg R14
L_180003c30:
    PUSH RSI
    .pushreg RSI
L_180003c31:
    PUSH RDI
    .pushreg RDI
L_180003c32:
    PUSH RBX
    .pushreg RBX
L_180003c33:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003c37:
    MOV R14, RCX
L_180003c3a:
    MOV RCX, qword ptr [OuterState+120]
L_180003c41:
    TEST RCX, RCX
L_180003c44:
    JZ L_180003c79
L_180003c46:
    MOV RSI, R9
L_180003c49:
    MOV RDI, R8
L_180003c4c:
    MOV RBX, RDX
L_180003c4f:
    LEA RDX, [name_20]
L_180003c56:
    CALL qword ptr [__imp_GetProcAddress]
L_180003c5c:
    TEST RAX, RAX
L_180003c5f:
    JZ L_180003c79
L_180003c61:
    MOV RCX, R14
L_180003c64:
    MOV RDX, RBX
L_180003c67:
    MOV R8, RDI
L_180003c6a:
    MOV R9, RSI
L_180003c6d:
    ADD RSP, 028h
L_180003c71:
    POP RBX
L_180003c72:
    POP RDI
L_180003c73:
    POP RSI
L_180003c74:
    POP R14
L_180003c76:
    JMP RAX
L_180003c79:
    XOR EAX, EAX
L_180003c7b:
    ADD RSP, 028h
L_180003c7f:
    POP RBX
L_180003c80:
    POP RDI
L_180003c81:
    POP RSI
L_180003c82:
    POP R14
L_180003c84:
    RET
reconstructed_20 ENDP
; VerFindFileA @ 21; original VA 0x1800051a7
; assembly ev_5ac74adc83f0a55300b1e2cbf90076d7bcd5e7e420e284bb31c54db8fcd4847b; pseudocode ev_887e1ce6f7ffa56bb432cc245a227c7ca0913a1225eee5f9fb9e1204dea22e9d
PUBLIC reconstructed_21
reconstructed_21 PROC FRAME
L_1800051a7:
    PUSH RSI
    .pushreg RSI
L_1800051a8:
    PUSH RDI
    .pushreg RDI
L_1800051a9:
    PUSH RBP
    .pushreg RBP
L_1800051aa:
    PUSH RBX
    .pushreg RBX
L_1800051ab:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800051af:
    MOV RSI, R9
L_1800051b2:
    MOV RDI, R8
L_1800051b5:
    MOV RBX, RDX
L_1800051b8:
    MOV EBP, ECX
L_1800051ba:
    MOV RAX, qword ptr [OuterState+72]
L_1800051c1:
    TEST RAX, RAX
L_1800051c4:
    JZ L_1800051dc
L_1800051c6:
    MOV ECX, EBP
L_1800051c8:
    MOV RDX, RBX
L_1800051cb:
    MOV R8, RDI
L_1800051ce:
    MOV R9, RSI
L_1800051d1:
    ADD RSP, 028h
L_1800051d5:
    POP RBX
L_1800051d6:
    POP RBP
L_1800051d7:
    POP RDI
L_1800051d8:
    POP RSI
L_1800051d9:
    JMP RAX
L_1800051dc:
    MOV RCX, qword ptr [OuterState+136]
L_1800051e3:
    TEST RCX, RCX
L_1800051e6:
    JZ L_180005203
L_1800051e8:
    LEA RDX, [name_21]
L_1800051ef:
    CALL qword ptr [__imp_GetProcAddress]
L_1800051f5:
    MOV qword ptr [OuterState+72], RAX
L_1800051fc:
    TEST RAX, RAX
L_1800051ff:
    JNZ L_1800051c6
L_180005201:
    JMP L_18000520e
L_180005203:
    MOV qword ptr [OuterState+72], 00h
L_18000520e:
    MOV EAX, 0ffffffffh
L_180005213:
    ADD RSP, 028h
L_180005217:
    POP RBX
L_180005218:
    POP RBP
L_180005219:
    POP RDI
L_18000521a:
    POP RSI
L_18000521b:
    RET
reconstructed_21 ENDP
; VerFindFileW @ 22; original VA 0x18000521c
; assembly ev_845422b7cf0b9099a43d919e25ee37ec46fd222054b42d39ef302e70f8476ad3; pseudocode ev_2051b5f94f161a0df83567c4e1f065db5dc0f415d7720e44d79b03394bc1d06b
PUBLIC reconstructed_22
reconstructed_22 PROC FRAME
L_18000521c:
    PUSH RSI
    .pushreg RSI
L_18000521d:
    PUSH RDI
    .pushreg RDI
L_18000521e:
    PUSH RBP
    .pushreg RBP
L_18000521f:
    PUSH RBX
    .pushreg RBX
L_180005220:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180005224:
    MOV RSI, R9
L_180005227:
    MOV RDI, R8
L_18000522a:
    MOV RBX, RDX
L_18000522d:
    MOV EBP, ECX
L_18000522f:
    MOV RAX, qword ptr [OuterState+80]
L_180005236:
    TEST RAX, RAX
L_180005239:
    JZ L_180005251
L_18000523b:
    MOV ECX, EBP
L_18000523d:
    MOV RDX, RBX
L_180005240:
    MOV R8, RDI
L_180005243:
    MOV R9, RSI
L_180005246:
    ADD RSP, 028h
L_18000524a:
    POP RBX
L_18000524b:
    POP RBP
L_18000524c:
    POP RDI
L_18000524d:
    POP RSI
L_18000524e:
    JMP RAX
L_180005251:
    MOV RCX, qword ptr [OuterState+136]
L_180005258:
    TEST RCX, RCX
L_18000525b:
    JZ L_180005278
L_18000525d:
    LEA RDX, [name_22]
L_180005264:
    CALL qword ptr [__imp_GetProcAddress]
L_18000526a:
    MOV qword ptr [OuterState+80], RAX
L_180005271:
    TEST RAX, RAX
L_180005274:
    JNZ L_18000523b
L_180005276:
    JMP L_180005283
L_180005278:
    MOV qword ptr [OuterState+80], 00h
L_180005283:
    MOV EAX, 0ffffffffh
L_180005288:
    ADD RSP, 028h
L_18000528c:
    POP RBX
L_18000528d:
    POP RBP
L_18000528e:
    POP RDI
L_18000528f:
    POP RSI
L_180005290:
    RET
reconstructed_22 ENDP
; VerInstallFileA @ 23; original VA 0x180005291
; assembly ev_a0f29a037d1c942829ec6a74e673503f2e3270adb2242179451ed38cc79777de; pseudocode ev_d5750163664e86d3a39cab63b41a0ed216c75a3b767b328f93ce47e68ec4dbda
PUBLIC reconstructed_23
reconstructed_23 PROC FRAME
L_180005291:
    PUSH RSI
    .pushreg RSI
L_180005292:
    PUSH RDI
    .pushreg RDI
L_180005293:
    PUSH RBP
    .pushreg RBP
L_180005294:
    PUSH RBX
    .pushreg RBX
L_180005295:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180005299:
    MOV RSI, R9
L_18000529c:
    MOV RDI, R8
L_18000529f:
    MOV RBX, RDX
L_1800052a2:
    MOV EBP, ECX
L_1800052a4:
    MOV RAX, qword ptr [OuterState+88]
L_1800052ab:
    TEST RAX, RAX
L_1800052ae:
    JZ L_1800052c6
L_1800052b0:
    MOV ECX, EBP
L_1800052b2:
    MOV RDX, RBX
L_1800052b5:
    MOV R8, RDI
L_1800052b8:
    MOV R9, RSI
L_1800052bb:
    ADD RSP, 028h
L_1800052bf:
    POP RBX
L_1800052c0:
    POP RBP
L_1800052c1:
    POP RDI
L_1800052c2:
    POP RSI
L_1800052c3:
    JMP RAX
L_1800052c6:
    MOV RCX, qword ptr [OuterState+136]
L_1800052cd:
    TEST RCX, RCX
L_1800052d0:
    JZ L_1800052ed
L_1800052d2:
    LEA RDX, [name_23]
L_1800052d9:
    CALL qword ptr [__imp_GetProcAddress]
L_1800052df:
    MOV qword ptr [OuterState+88], RAX
L_1800052e6:
    TEST RAX, RAX
L_1800052e9:
    JNZ L_1800052b0
L_1800052eb:
    JMP L_1800052f8
L_1800052ed:
    MOV qword ptr [OuterState+88], 00h
L_1800052f8:
    MOV EAX, 0ffffffffh
L_1800052fd:
    ADD RSP, 028h
L_180005301:
    POP RBX
L_180005302:
    POP RBP
L_180005303:
    POP RDI
L_180005304:
    POP RSI
L_180005305:
    RET
reconstructed_23 ENDP
; VerInstallFileW @ 24; original VA 0x180005306
; assembly ev_ebfffcfb5e05f41e1b240d526a87a362a3350ad5705e5791e04d2d11695f114b; pseudocode ev_ca89f8b766f72dbce3bc9d55114e8282665b8b0c89cb50d117f3703acbae864e
PUBLIC reconstructed_24
reconstructed_24 PROC FRAME
L_180005306:
    PUSH RSI
    .pushreg RSI
L_180005307:
    PUSH RDI
    .pushreg RDI
L_180005308:
    PUSH RBP
    .pushreg RBP
L_180005309:
    PUSH RBX
    .pushreg RBX
L_18000530a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000530e:
    MOV RSI, R9
L_180005311:
    MOV RDI, R8
L_180005314:
    MOV RBX, RDX
L_180005317:
    MOV EBP, ECX
L_180005319:
    MOV RAX, qword ptr [OuterState+96]
L_180005320:
    TEST RAX, RAX
L_180005323:
    JZ L_18000533b
L_180005325:
    MOV ECX, EBP
L_180005327:
    MOV RDX, RBX
L_18000532a:
    MOV R8, RDI
L_18000532d:
    MOV R9, RSI
L_180005330:
    ADD RSP, 028h
L_180005334:
    POP RBX
L_180005335:
    POP RBP
L_180005336:
    POP RDI
L_180005337:
    POP RSI
L_180005338:
    JMP RAX
L_18000533b:
    MOV RCX, qword ptr [OuterState+136]
L_180005342:
    TEST RCX, RCX
L_180005345:
    JZ L_180005362
L_180005347:
    LEA RDX, [name_24]
L_18000534e:
    CALL qword ptr [__imp_GetProcAddress]
L_180005354:
    MOV qword ptr [OuterState+96], RAX
L_18000535b:
    TEST RAX, RAX
L_18000535e:
    JNZ L_180005325
L_180005360:
    JMP L_18000536d
L_180005362:
    MOV qword ptr [OuterState+96], 00h
L_18000536d:
    MOV EAX, 0ffffffffh
L_180005372:
    ADD RSP, 028h
L_180005376:
    POP RBX
L_180005377:
    POP RBP
L_180005378:
    POP RDI
L_180005379:
    POP RSI
L_18000537a:
    RET
reconstructed_24 ENDP
; VerLanguageNameA @ 25; original VA 0x180004d2c
; assembly ev_aa353f176c6a4770c5c9aad70abd90c69e334ae6a6b7b6887f9ad2e71b5b072f; pseudocode ev_e534548b2ec76dbd980193bc8af684e1a85da85b021dba4ab554b80db7176366
PUBLIC reconstructed_25
reconstructed_25 PROC FRAME
L_180004d2c:
    PUSH R14
    .pushreg R14
L_180004d2e:
    PUSH RSI
    .pushreg RSI
L_180004d2f:
    PUSH RDI
    .pushreg RDI
L_180004d30:
    PUSH RBX
    .pushreg RBX
L_180004d31:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004d35:
    MOV R14, RCX
L_180004d38:
    MOV RCX, qword ptr [OuterState+128]
L_180004d3f:
    TEST RCX, RCX
L_180004d42:
    JZ L_180004d77
L_180004d44:
    MOV RSI, R9
L_180004d47:
    MOV RDI, R8
L_180004d4a:
    MOV RBX, RDX
L_180004d4d:
    LEA RDX, [name_25]
L_180004d54:
    CALL qword ptr [__imp_GetProcAddress]
L_180004d5a:
    TEST RAX, RAX
L_180004d5d:
    JZ L_180004d77
L_180004d5f:
    MOV RCX, R14
L_180004d62:
    MOV RDX, RBX
L_180004d65:
    MOV R8, RDI
L_180004d68:
    MOV R9, RSI
L_180004d6b:
    ADD RSP, 028h
L_180004d6f:
    POP RBX
L_180004d70:
    POP RDI
L_180004d71:
    POP RSI
L_180004d72:
    POP R14
L_180004d74:
    JMP RAX
L_180004d77:
    XOR EAX, EAX
L_180004d79:
    ADD RSP, 028h
L_180004d7d:
    POP RBX
L_180004d7e:
    POP RDI
L_180004d7f:
    POP RSI
L_180004d80:
    POP R14
L_180004d82:
    RET
reconstructed_25 ENDP
; VerLanguageNameW @ 26; original VA 0x180004d83
; assembly ev_3ac25c7779a4049a96a145b6a8fcd4e3052c6adb3e0eade32f9083f6fcf22248; pseudocode ev_4fca8e8ea0ea71c0adf60f8cd111cb5c0f18d3138eb53c83e722db1f1f691289
PUBLIC reconstructed_26
reconstructed_26 PROC FRAME
L_180004d83:
    PUSH R14
    .pushreg R14
L_180004d85:
    PUSH RSI
    .pushreg RSI
L_180004d86:
    PUSH RDI
    .pushreg RDI
L_180004d87:
    PUSH RBX
    .pushreg RBX
L_180004d88:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004d8c:
    MOV R14, RCX
L_180004d8f:
    MOV RCX, qword ptr [OuterState+128]
L_180004d96:
    TEST RCX, RCX
L_180004d99:
    JZ L_180004dce
L_180004d9b:
    MOV RSI, R9
L_180004d9e:
    MOV RDI, R8
L_180004da1:
    MOV RBX, RDX
L_180004da4:
    LEA RDX, [name_26]
L_180004dab:
    CALL qword ptr [__imp_GetProcAddress]
L_180004db1:
    TEST RAX, RAX
L_180004db4:
    JZ L_180004dce
L_180004db6:
    MOV RCX, R14
L_180004db9:
    MOV RDX, RBX
L_180004dbc:
    MOV R8, RDI
L_180004dbf:
    MOV R9, RSI
L_180004dc2:
    ADD RSP, 028h
L_180004dc6:
    POP RBX
L_180004dc7:
    POP RDI
L_180004dc8:
    POP RSI
L_180004dc9:
    POP R14
L_180004dcb:
    JMP RAX
L_180004dce:
    XOR EAX, EAX
L_180004dd0:
    ADD RSP, 028h
L_180004dd4:
    POP RBX
L_180004dd5:
    POP RDI
L_180004dd6:
    POP RSI
L_180004dd7:
    POP R14
L_180004dd9:
    RET
reconstructed_26 ENDP
; VerQueryValueA @ 27; original VA 0x18000537b
; assembly ev_01ef3b10cea3e2d2eac970ba1c328d4b32221ec310cc56db0c1812091cf6cc88; pseudocode ev_caa131863b35ea4387393189980bc41d2a0b265b2dc09d4520422b17e6d21cf9
PUBLIC reconstructed_27
reconstructed_27 PROC FRAME
L_18000537b:
    PUSH R14
    .pushreg R14
L_18000537d:
    PUSH RSI
    .pushreg RSI
L_18000537e:
    PUSH RDI
    .pushreg RDI
L_18000537f:
    PUSH RBX
    .pushreg RBX
L_180005380:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180005384:
    MOV RSI, R9
L_180005387:
    MOV RDI, R8
L_18000538a:
    MOV RBX, RDX
L_18000538d:
    MOV R14, RCX
L_180005390:
    MOV RAX, qword ptr [OuterState+104]
L_180005397:
    TEST RAX, RAX
L_18000539a:
    JZ L_1800053b4
L_18000539c:
    MOV RCX, R14
L_18000539f:
    MOV RDX, RBX
L_1800053a2:
    MOV R8, RDI
L_1800053a5:
    MOV R9, RSI
L_1800053a8:
    ADD RSP, 028h
L_1800053ac:
    POP RBX
L_1800053ad:
    POP RDI
L_1800053ae:
    POP RSI
L_1800053af:
    POP R14
L_1800053b1:
    JMP RAX
L_1800053b4:
    MOV RCX, qword ptr [OuterState+136]
L_1800053bb:
    TEST RCX, RCX
L_1800053be:
    JZ L_1800053db
L_1800053c0:
    LEA RDX, [name_27]
L_1800053c7:
    CALL qword ptr [__imp_GetProcAddress]
L_1800053cd:
    MOV qword ptr [OuterState+104], RAX
L_1800053d4:
    TEST RAX, RAX
L_1800053d7:
    JNZ L_18000539c
L_1800053d9:
    JMP L_1800053e6
L_1800053db:
    MOV qword ptr [OuterState+104], 00h
L_1800053e6:
    XOR EAX, EAX
L_1800053e8:
    ADD RSP, 028h
L_1800053ec:
    POP RBX
L_1800053ed:
    POP RDI
L_1800053ee:
    POP RSI
L_1800053ef:
    POP R14
L_1800053f1:
    RET
reconstructed_27 ENDP
; VerQueryValueW @ 28; original VA 0x1800053f2
; assembly ev_83a173494e0f6c9f73caa272666ccc127a6a6e643ceae885e257387bea672d9b; pseudocode ev_06cce5fead2e4c331d5cb2358e82feaba0944753f47937349f61655188686d3a
PUBLIC reconstructed_28
reconstructed_28 PROC FRAME
L_1800053f2:
    PUSH R14
    .pushreg R14
L_1800053f4:
    PUSH RSI
    .pushreg RSI
L_1800053f5:
    PUSH RDI
    .pushreg RDI
L_1800053f6:
    PUSH RBX
    .pushreg RBX
L_1800053f7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800053fb:
    MOV RSI, R9
L_1800053fe:
    MOV RDI, R8
L_180005401:
    MOV RBX, RDX
L_180005404:
    MOV R14, RCX
L_180005407:
    MOV RAX, qword ptr [OuterState+112]
L_18000540e:
    TEST RAX, RAX
L_180005411:
    JZ L_18000542b
L_180005413:
    MOV RCX, R14
L_180005416:
    MOV RDX, RBX
L_180005419:
    MOV R8, RDI
L_18000541c:
    MOV R9, RSI
L_18000541f:
    ADD RSP, 028h
L_180005423:
    POP RBX
L_180005424:
    POP RDI
L_180005425:
    POP RSI
L_180005426:
    POP R14
L_180005428:
    JMP RAX
L_18000542b:
    MOV RCX, qword ptr [OuterState+136]
L_180005432:
    TEST RCX, RCX
L_180005435:
    JZ L_180005452
L_180005437:
    LEA RDX, [name_28]
L_18000543e:
    CALL qword ptr [__imp_GetProcAddress]
L_180005444:
    MOV qword ptr [OuterState+112], RAX
L_18000544b:
    TEST RAX, RAX
L_18000544e:
    JNZ L_180005413
L_180005450:
    JMP L_18000545d
L_180005452:
    MOV qword ptr [OuterState+112], 00h
L_18000545d:
    XOR EAX, EAX
L_18000545f:
    ADD RSP, 028h
L_180005463:
    POP RBX
L_180005464:
    POP RDI
L_180005465:
    POP RSI
L_180005466:
    POP R14
L_180005468:
    RET
reconstructed_28 ENDP
; WOWAppExit @ 29; original VA 0x180004cd5
; assembly ev_7f197c4b549b73e4120d3a45e25fb82cdb000e9fb771a252bde1fbefd91a340d; pseudocode ev_6f1a3d979e6e0a2ecf66a1af2bc7ec35567c41e85fdac7ace00cccca89c7e3aa
PUBLIC reconstructed_29
reconstructed_29 PROC FRAME
L_180004cd5:
    PUSH R14
    .pushreg R14
L_180004cd7:
    PUSH RSI
    .pushreg RSI
L_180004cd8:
    PUSH RDI
    .pushreg RDI
L_180004cd9:
    PUSH RBX
    .pushreg RBX
L_180004cda:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004cde:
    MOV R14, RCX
L_180004ce1:
    MOV RCX, qword ptr [OuterState+120]
L_180004ce8:
    TEST RCX, RCX
L_180004ceb:
    JZ L_180004d20
L_180004ced:
    MOV RSI, R9
L_180004cf0:
    MOV RDI, R8
L_180004cf3:
    MOV RBX, RDX
L_180004cf6:
    LEA RDX, [name_29]
L_180004cfd:
    CALL qword ptr [__imp_GetProcAddress]
L_180004d03:
    TEST RAX, RAX
L_180004d06:
    JZ L_180004d20
L_180004d08:
    MOV RCX, R14
L_180004d0b:
    MOV RDX, RBX
L_180004d0e:
    MOV R8, RDI
L_180004d11:
    MOV R9, RSI
L_180004d14:
    ADD RSP, 028h
L_180004d18:
    POP RBX
L_180004d19:
    POP RDI
L_180004d1a:
    POP RSI
L_180004d1b:
    POP R14
L_180004d1d:
    JMP RAX
L_180004d20:
    XOR EAX, EAX
L_180004d22:
    ADD RSP, 028h
L_180004d26:
    POP RBX
L_180004d27:
    POP RDI
L_180004d28:
    POP RSI
L_180004d29:
    POP R14
L_180004d2b:
    RET
reconstructed_29 ENDP
; auxGetDevCapsA @ 30; original VA 0x180001000
; assembly ev_76fec7fa9c576343c2b03e5a92917c7c78e08d4b52c0d07e8c1a64a57d437ab1; pseudocode ev_dcf8e03b22dcd469585b9a641efc6ac979f944c282014e985ad5338040bb790f
PUBLIC reconstructed_30
reconstructed_30 PROC FRAME
L_180001000:
    PUSH R14
    .pushreg R14
L_180001002:
    PUSH RSI
    .pushreg RSI
L_180001003:
    PUSH RDI
    .pushreg RDI
L_180001004:
    PUSH RBX
    .pushreg RBX
L_180001005:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001009:
    MOV R14, RCX
L_18000100c:
    MOV RCX, qword ptr [OuterState+120]
L_180001013:
    TEST RCX, RCX
L_180001016:
    JZ L_18000104b
L_180001018:
    MOV RSI, R9
L_18000101b:
    MOV RDI, R8
L_18000101e:
    MOV RBX, RDX
L_180001021:
    LEA RDX, [name_30]
L_180001028:
    CALL qword ptr [__imp_GetProcAddress]
L_18000102e:
    TEST RAX, RAX
L_180001031:
    JZ L_18000104b
L_180001033:
    MOV RCX, R14
L_180001036:
    MOV RDX, RBX
L_180001039:
    MOV R8, RDI
L_18000103c:
    MOV R9, RSI
L_18000103f:
    ADD RSP, 028h
L_180001043:
    POP RBX
L_180001044:
    POP RDI
L_180001045:
    POP RSI
L_180001046:
    POP R14
L_180001048:
    JMP RAX
L_18000104b:
    XOR EAX, EAX
L_18000104d:
    ADD RSP, 028h
L_180001051:
    POP RBX
L_180001052:
    POP RDI
L_180001053:
    POP RSI
L_180001054:
    POP R14
L_180001056:
    RET
reconstructed_30 ENDP
; auxGetDevCapsW @ 31; original VA 0x180001057
; assembly ev_041e03cadfc94150cae4e5d8a8062850331dc68e4b604fb5be0217c11c940fed; pseudocode ev_64e01628f39c4e18bb92d39a3e9cd191baba264098d40ae7f721e2feadb2e815
PUBLIC reconstructed_31
reconstructed_31 PROC FRAME
L_180001057:
    PUSH R14
    .pushreg R14
L_180001059:
    PUSH RSI
    .pushreg RSI
L_18000105a:
    PUSH RDI
    .pushreg RDI
L_18000105b:
    PUSH RBX
    .pushreg RBX
L_18000105c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001060:
    MOV R14, RCX
L_180001063:
    MOV RCX, qword ptr [OuterState+120]
L_18000106a:
    TEST RCX, RCX
L_18000106d:
    JZ L_1800010a2
L_18000106f:
    MOV RSI, R9
L_180001072:
    MOV RDI, R8
L_180001075:
    MOV RBX, RDX
L_180001078:
    LEA RDX, [name_31]
L_18000107f:
    CALL qword ptr [__imp_GetProcAddress]
L_180001085:
    TEST RAX, RAX
L_180001088:
    JZ L_1800010a2
L_18000108a:
    MOV RCX, R14
L_18000108d:
    MOV RDX, RBX
L_180001090:
    MOV R8, RDI
L_180001093:
    MOV R9, RSI
L_180001096:
    ADD RSP, 028h
L_18000109a:
    POP RBX
L_18000109b:
    POP RDI
L_18000109c:
    POP RSI
L_18000109d:
    POP R14
L_18000109f:
    JMP RAX
L_1800010a2:
    XOR EAX, EAX
L_1800010a4:
    ADD RSP, 028h
L_1800010a8:
    POP RBX
L_1800010a9:
    POP RDI
L_1800010aa:
    POP RSI
L_1800010ab:
    POP R14
L_1800010ad:
    RET
reconstructed_31 ENDP
; auxGetNumDevs @ 32; original VA 0x1800010ae
; assembly ev_f3ccca2912c49b0f6db27f2cfdd250ce00ffde20672a826019847a61b730c305; pseudocode ev_703675b739412fb8ef346b8dcc85834128093130dcba96d88584ca021e062dc4
PUBLIC reconstructed_32
reconstructed_32 PROC FRAME
L_1800010ae:
    PUSH R14
    .pushreg R14
L_1800010b0:
    PUSH RSI
    .pushreg RSI
L_1800010b1:
    PUSH RDI
    .pushreg RDI
L_1800010b2:
    PUSH RBX
    .pushreg RBX
L_1800010b3:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800010b7:
    MOV R14, RCX
L_1800010ba:
    MOV RCX, qword ptr [OuterState+120]
L_1800010c1:
    TEST RCX, RCX
L_1800010c4:
    JZ L_1800010f9
L_1800010c6:
    MOV RSI, R9
L_1800010c9:
    MOV RDI, R8
L_1800010cc:
    MOV RBX, RDX
L_1800010cf:
    LEA RDX, [name_32]
L_1800010d6:
    CALL qword ptr [__imp_GetProcAddress]
L_1800010dc:
    TEST RAX, RAX
L_1800010df:
    JZ L_1800010f9
L_1800010e1:
    MOV RCX, R14
L_1800010e4:
    MOV RDX, RBX
L_1800010e7:
    MOV R8, RDI
L_1800010ea:
    MOV R9, RSI
L_1800010ed:
    ADD RSP, 028h
L_1800010f1:
    POP RBX
L_1800010f2:
    POP RDI
L_1800010f3:
    POP RSI
L_1800010f4:
    POP R14
L_1800010f6:
    JMP RAX
L_1800010f9:
    XOR EAX, EAX
L_1800010fb:
    ADD RSP, 028h
L_1800010ff:
    POP RBX
L_180001100:
    POP RDI
L_180001101:
    POP RSI
L_180001102:
    POP R14
L_180001104:
    RET
reconstructed_32 ENDP
; auxGetVolume @ 33; original VA 0x180001105
; assembly ev_9831d9163a9e558bec4d7bf5ecaa26633ad2db2073abe2a0275cd920e46b265c; pseudocode ev_6e49fc2a02596a721566c6c1cce93f5983b0dd11ffe741549fb1e7efc4b516c9
PUBLIC reconstructed_33
reconstructed_33 PROC FRAME
L_180001105:
    PUSH R14
    .pushreg R14
L_180001107:
    PUSH RSI
    .pushreg RSI
L_180001108:
    PUSH RDI
    .pushreg RDI
L_180001109:
    PUSH RBX
    .pushreg RBX
L_18000110a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000110e:
    MOV R14, RCX
L_180001111:
    MOV RCX, qword ptr [OuterState+120]
L_180001118:
    TEST RCX, RCX
L_18000111b:
    JZ L_180001150
L_18000111d:
    MOV RSI, R9
L_180001120:
    MOV RDI, R8
L_180001123:
    MOV RBX, RDX
L_180001126:
    LEA RDX, [name_33]
L_18000112d:
    CALL qword ptr [__imp_GetProcAddress]
L_180001133:
    TEST RAX, RAX
L_180001136:
    JZ L_180001150
L_180001138:
    MOV RCX, R14
L_18000113b:
    MOV RDX, RBX
L_18000113e:
    MOV R8, RDI
L_180001141:
    MOV R9, RSI
L_180001144:
    ADD RSP, 028h
L_180001148:
    POP RBX
L_180001149:
    POP RDI
L_18000114a:
    POP RSI
L_18000114b:
    POP R14
L_18000114d:
    JMP RAX
L_180001150:
    XOR EAX, EAX
L_180001152:
    ADD RSP, 028h
L_180001156:
    POP RBX
L_180001157:
    POP RDI
L_180001158:
    POP RSI
L_180001159:
    POP R14
L_18000115b:
    RET
reconstructed_33 ENDP
; auxOutMessage @ 34; original VA 0x18000115c
; assembly ev_3b08e08e4ec90c8f32836f3c66b93fc2a118c514054ed66d998d99330552d0df; pseudocode ev_3b32389bed11b4d304d27fe6b4628e682ed26285f7f12f1bd23ba26ee17c9086
PUBLIC reconstructed_34
reconstructed_34 PROC FRAME
L_18000115c:
    PUSH R14
    .pushreg R14
L_18000115e:
    PUSH RSI
    .pushreg RSI
L_18000115f:
    PUSH RDI
    .pushreg RDI
L_180001160:
    PUSH RBX
    .pushreg RBX
L_180001161:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001165:
    MOV R14, RCX
L_180001168:
    MOV RCX, qword ptr [OuterState+120]
L_18000116f:
    TEST RCX, RCX
L_180001172:
    JZ L_1800011a7
L_180001174:
    MOV RSI, R9
L_180001177:
    MOV RDI, R8
L_18000117a:
    MOV RBX, RDX
L_18000117d:
    LEA RDX, [name_34]
L_180001184:
    CALL qword ptr [__imp_GetProcAddress]
L_18000118a:
    TEST RAX, RAX
L_18000118d:
    JZ L_1800011a7
L_18000118f:
    MOV RCX, R14
L_180001192:
    MOV RDX, RBX
L_180001195:
    MOV R8, RDI
L_180001198:
    MOV R9, RSI
L_18000119b:
    ADD RSP, 028h
L_18000119f:
    POP RBX
L_1800011a0:
    POP RDI
L_1800011a1:
    POP RSI
L_1800011a2:
    POP R14
L_1800011a4:
    JMP RAX
L_1800011a7:
    XOR EAX, EAX
L_1800011a9:
    ADD RSP, 028h
L_1800011ad:
    POP RBX
L_1800011ae:
    POP RDI
L_1800011af:
    POP RSI
L_1800011b0:
    POP R14
L_1800011b2:
    RET
reconstructed_34 ENDP
; auxSetVolume @ 35; original VA 0x1800011b3
; assembly ev_b343cbda8b529a37c1ca5a7276fdcf654965c6b822e8b59931a41654d3c268b8; pseudocode ev_984dd3a7fde48eb2d474392223786b9e8e64bc3dd3caf88573e1a0af3c47fb9f
PUBLIC reconstructed_35
reconstructed_35 PROC FRAME
L_1800011b3:
    PUSH R14
    .pushreg R14
L_1800011b5:
    PUSH RSI
    .pushreg RSI
L_1800011b6:
    PUSH RDI
    .pushreg RDI
L_1800011b7:
    PUSH RBX
    .pushreg RBX
L_1800011b8:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800011bc:
    MOV R14, RCX
L_1800011bf:
    MOV RCX, qword ptr [OuterState+120]
L_1800011c6:
    TEST RCX, RCX
L_1800011c9:
    JZ L_1800011fe
L_1800011cb:
    MOV RSI, R9
L_1800011ce:
    MOV RDI, R8
L_1800011d1:
    MOV RBX, RDX
L_1800011d4:
    LEA RDX, [name_35]
L_1800011db:
    CALL qword ptr [__imp_GetProcAddress]
L_1800011e1:
    TEST RAX, RAX
L_1800011e4:
    JZ L_1800011fe
L_1800011e6:
    MOV RCX, R14
L_1800011e9:
    MOV RDX, RBX
L_1800011ec:
    MOV R8, RDI
L_1800011ef:
    MOV R9, RSI
L_1800011f2:
    ADD RSP, 028h
L_1800011f6:
    POP RBX
L_1800011f7:
    POP RDI
L_1800011f8:
    POP RSI
L_1800011f9:
    POP R14
L_1800011fb:
    JMP RAX
L_1800011fe:
    XOR EAX, EAX
L_180001200:
    ADD RSP, 028h
L_180001204:
    POP RBX
L_180001205:
    POP RDI
L_180001206:
    POP RSI
L_180001207:
    POP R14
L_180001209:
    RET
reconstructed_35 ENDP
; joyConfigChanged @ 36; original VA 0x1800013bd
; assembly ev_e8d1e5bdbca934128c97d2d3cd85a193724abd3ced4a6e798d093ee00bd0d806; pseudocode ev_e0dafc3b632f0a3081d9a9db1071bd61d811a387fca2411b964abe707ace5c9d
PUBLIC reconstructed_36
reconstructed_36 PROC FRAME
L_1800013bd:
    PUSH R14
    .pushreg R14
L_1800013bf:
    PUSH RSI
    .pushreg RSI
L_1800013c0:
    PUSH RDI
    .pushreg RDI
L_1800013c1:
    PUSH RBX
    .pushreg RBX
L_1800013c2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800013c6:
    MOV R14, RCX
L_1800013c9:
    MOV RCX, qword ptr [OuterState+120]
L_1800013d0:
    TEST RCX, RCX
L_1800013d3:
    JZ L_180001408
L_1800013d5:
    MOV RSI, R9
L_1800013d8:
    MOV RDI, R8
L_1800013db:
    MOV RBX, RDX
L_1800013de:
    LEA RDX, [name_36]
L_1800013e5:
    CALL qword ptr [__imp_GetProcAddress]
L_1800013eb:
    TEST RAX, RAX
L_1800013ee:
    JZ L_180001408
L_1800013f0:
    MOV RCX, R14
L_1800013f3:
    MOV RDX, RBX
L_1800013f6:
    MOV R8, RDI
L_1800013f9:
    MOV R9, RSI
L_1800013fc:
    ADD RSP, 028h
L_180001400:
    POP RBX
L_180001401:
    POP RDI
L_180001402:
    POP RSI
L_180001403:
    POP R14
L_180001405:
    JMP RAX
L_180001408:
    XOR EAX, EAX
L_18000140a:
    ADD RSP, 028h
L_18000140e:
    POP RBX
L_18000140f:
    POP RDI
L_180001410:
    POP RSI
L_180001411:
    POP R14
L_180001413:
    RET
reconstructed_36 ENDP
; joyGetDevCapsA @ 37; original VA 0x180001414
; assembly ev_38384cf3622df41df17d74ccc635886e5bbdff454ab8cb1d2b27eadd0c29d6a7; pseudocode ev_bbb7dd201c8dc2571b6a36a44ab6bee4ae8ae28a44ad076dbbf8fd16fc4b5b29
PUBLIC reconstructed_37
reconstructed_37 PROC FRAME
L_180001414:
    PUSH R14
    .pushreg R14
L_180001416:
    PUSH RSI
    .pushreg RSI
L_180001417:
    PUSH RDI
    .pushreg RDI
L_180001418:
    PUSH RBX
    .pushreg RBX
L_180001419:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000141d:
    MOV R14, RCX
L_180001420:
    MOV RCX, qword ptr [OuterState+120]
L_180001427:
    TEST RCX, RCX
L_18000142a:
    JZ L_18000145f
L_18000142c:
    MOV RSI, R9
L_18000142f:
    MOV RDI, R8
L_180001432:
    MOV RBX, RDX
L_180001435:
    LEA RDX, [name_37]
L_18000143c:
    CALL qword ptr [__imp_GetProcAddress]
L_180001442:
    TEST RAX, RAX
L_180001445:
    JZ L_18000145f
L_180001447:
    MOV RCX, R14
L_18000144a:
    MOV RDX, RBX
L_18000144d:
    MOV R8, RDI
L_180001450:
    MOV R9, RSI
L_180001453:
    ADD RSP, 028h
L_180001457:
    POP RBX
L_180001458:
    POP RDI
L_180001459:
    POP RSI
L_18000145a:
    POP R14
L_18000145c:
    JMP RAX
L_18000145f:
    XOR EAX, EAX
L_180001461:
    ADD RSP, 028h
L_180001465:
    POP RBX
L_180001466:
    POP RDI
L_180001467:
    POP RSI
L_180001468:
    POP R14
L_18000146a:
    RET
reconstructed_37 ENDP
; joyGetDevCapsW @ 38; original VA 0x18000146b
; assembly ev_d8d7c20498a4dd543b6cdbfdfb396d7e6c3ff9d76ff37edb7a71962040a3bd49; pseudocode ev_e9dbb0d166d2ff22bb7f0fa8ee628533929ca264c29968d5728d3d2e1bf2b814
PUBLIC reconstructed_38
reconstructed_38 PROC FRAME
L_18000146b:
    PUSH R14
    .pushreg R14
L_18000146d:
    PUSH RSI
    .pushreg RSI
L_18000146e:
    PUSH RDI
    .pushreg RDI
L_18000146f:
    PUSH RBX
    .pushreg RBX
L_180001470:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001474:
    MOV R14, RCX
L_180001477:
    MOV RCX, qword ptr [OuterState+120]
L_18000147e:
    TEST RCX, RCX
L_180001481:
    JZ L_1800014b6
L_180001483:
    MOV RSI, R9
L_180001486:
    MOV RDI, R8
L_180001489:
    MOV RBX, RDX
L_18000148c:
    LEA RDX, [name_38]
L_180001493:
    CALL qword ptr [__imp_GetProcAddress]
L_180001499:
    TEST RAX, RAX
L_18000149c:
    JZ L_1800014b6
L_18000149e:
    MOV RCX, R14
L_1800014a1:
    MOV RDX, RBX
L_1800014a4:
    MOV R8, RDI
L_1800014a7:
    MOV R9, RSI
L_1800014aa:
    ADD RSP, 028h
L_1800014ae:
    POP RBX
L_1800014af:
    POP RDI
L_1800014b0:
    POP RSI
L_1800014b1:
    POP R14
L_1800014b3:
    JMP RAX
L_1800014b6:
    XOR EAX, EAX
L_1800014b8:
    ADD RSP, 028h
L_1800014bc:
    POP RBX
L_1800014bd:
    POP RDI
L_1800014be:
    POP RSI
L_1800014bf:
    POP R14
L_1800014c1:
    RET
reconstructed_38 ENDP
; joyGetNumDevs @ 39; original VA 0x1800014c2
; assembly ev_5279adbea34379c223166fdab93bfcbd34a16a55056e7b661aec9061136413f6; pseudocode ev_22285b661b4ce916665fd00df177189352a96a17004a278c7c34cbf7c09beb22
PUBLIC reconstructed_39
reconstructed_39 PROC FRAME
L_1800014c2:
    PUSH R14
    .pushreg R14
L_1800014c4:
    PUSH RSI
    .pushreg RSI
L_1800014c5:
    PUSH RDI
    .pushreg RDI
L_1800014c6:
    PUSH RBX
    .pushreg RBX
L_1800014c7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800014cb:
    MOV R14, RCX
L_1800014ce:
    MOV RCX, qword ptr [OuterState+120]
L_1800014d5:
    TEST RCX, RCX
L_1800014d8:
    JZ L_18000150d
L_1800014da:
    MOV RSI, R9
L_1800014dd:
    MOV RDI, R8
L_1800014e0:
    MOV RBX, RDX
L_1800014e3:
    LEA RDX, [name_39]
L_1800014ea:
    CALL qword ptr [__imp_GetProcAddress]
L_1800014f0:
    TEST RAX, RAX
L_1800014f3:
    JZ L_18000150d
L_1800014f5:
    MOV RCX, R14
L_1800014f8:
    MOV RDX, RBX
L_1800014fb:
    MOV R8, RDI
L_1800014fe:
    MOV R9, RSI
L_180001501:
    ADD RSP, 028h
L_180001505:
    POP RBX
L_180001506:
    POP RDI
L_180001507:
    POP RSI
L_180001508:
    POP R14
L_18000150a:
    JMP RAX
L_18000150d:
    XOR EAX, EAX
L_18000150f:
    ADD RSP, 028h
L_180001513:
    POP RBX
L_180001514:
    POP RDI
L_180001515:
    POP RSI
L_180001516:
    POP R14
L_180001518:
    RET
reconstructed_39 ENDP
; joyGetPos @ 40; original VA 0x180001519
; assembly ev_84674156bb9790776899bcb81a1df090d440effdb4067e4e3f62e43ecb88c680; pseudocode ev_3b9a71c5762bf8edcf7e37e1ebbe9a9f530a9061fc1ae4bbdd4b54f4e772cb11
PUBLIC reconstructed_40
reconstructed_40 PROC FRAME
L_180001519:
    PUSH R14
    .pushreg R14
L_18000151b:
    PUSH RSI
    .pushreg RSI
L_18000151c:
    PUSH RDI
    .pushreg RDI
L_18000151d:
    PUSH RBX
    .pushreg RBX
L_18000151e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001522:
    MOV R14, RCX
L_180001525:
    MOV RCX, qword ptr [OuterState+120]
L_18000152c:
    TEST RCX, RCX
L_18000152f:
    JZ L_180001564
L_180001531:
    MOV RSI, R9
L_180001534:
    MOV RDI, R8
L_180001537:
    MOV RBX, RDX
L_18000153a:
    LEA RDX, [name_40]
L_180001541:
    CALL qword ptr [__imp_GetProcAddress]
L_180001547:
    TEST RAX, RAX
L_18000154a:
    JZ L_180001564
L_18000154c:
    MOV RCX, R14
L_18000154f:
    MOV RDX, RBX
L_180001552:
    MOV R8, RDI
L_180001555:
    MOV R9, RSI
L_180001558:
    ADD RSP, 028h
L_18000155c:
    POP RBX
L_18000155d:
    POP RDI
L_18000155e:
    POP RSI
L_18000155f:
    POP R14
L_180001561:
    JMP RAX
L_180001564:
    XOR EAX, EAX
L_180001566:
    ADD RSP, 028h
L_18000156a:
    POP RBX
L_18000156b:
    POP RDI
L_18000156c:
    POP RSI
L_18000156d:
    POP R14
L_18000156f:
    RET
reconstructed_40 ENDP
; joyGetPosEx @ 41; original VA 0x180001570
; assembly ev_b4a13da8609f667bdc9876576194c843ecb56ebfc06e2319fd345f0f145f7917; pseudocode ev_4a07a3f20852aae119ad45ad1a2436ec7e1ac777ae51335774b0efac6c2539b9
PUBLIC reconstructed_41
reconstructed_41 PROC FRAME
L_180001570:
    PUSH R14
    .pushreg R14
L_180001572:
    PUSH RSI
    .pushreg RSI
L_180001573:
    PUSH RDI
    .pushreg RDI
L_180001574:
    PUSH RBX
    .pushreg RBX
L_180001575:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001579:
    MOV R14, RCX
L_18000157c:
    MOV RCX, qword ptr [OuterState+120]
L_180001583:
    TEST RCX, RCX
L_180001586:
    JZ L_1800015bb
L_180001588:
    MOV RSI, R9
L_18000158b:
    MOV RDI, R8
L_18000158e:
    MOV RBX, RDX
L_180001591:
    LEA RDX, [name_41]
L_180001598:
    CALL qword ptr [__imp_GetProcAddress]
L_18000159e:
    TEST RAX, RAX
L_1800015a1:
    JZ L_1800015bb
L_1800015a3:
    MOV RCX, R14
L_1800015a6:
    MOV RDX, RBX
L_1800015a9:
    MOV R8, RDI
L_1800015ac:
    MOV R9, RSI
L_1800015af:
    ADD RSP, 028h
L_1800015b3:
    POP RBX
L_1800015b4:
    POP RDI
L_1800015b5:
    POP RSI
L_1800015b6:
    POP R14
L_1800015b8:
    JMP RAX
L_1800015bb:
    XOR EAX, EAX
L_1800015bd:
    ADD RSP, 028h
L_1800015c1:
    POP RBX
L_1800015c2:
    POP RDI
L_1800015c3:
    POP RSI
L_1800015c4:
    POP R14
L_1800015c6:
    RET
reconstructed_41 ENDP
; joyGetThreshold @ 42; original VA 0x1800015c7
; assembly ev_98b1aa8a28ddf2e0ba53e527b935afdea90d644fd768f1325091efdc52653887; pseudocode ev_fff55e844e8ca4994482841d94ae64cf56659111fd5193f1bf7dbad3a02f2f22
PUBLIC reconstructed_42
reconstructed_42 PROC FRAME
L_1800015c7:
    PUSH R14
    .pushreg R14
L_1800015c9:
    PUSH RSI
    .pushreg RSI
L_1800015ca:
    PUSH RDI
    .pushreg RDI
L_1800015cb:
    PUSH RBX
    .pushreg RBX
L_1800015cc:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800015d0:
    MOV R14, RCX
L_1800015d3:
    MOV RCX, qword ptr [OuterState+120]
L_1800015da:
    TEST RCX, RCX
L_1800015dd:
    JZ L_180001612
L_1800015df:
    MOV RSI, R9
L_1800015e2:
    MOV RDI, R8
L_1800015e5:
    MOV RBX, RDX
L_1800015e8:
    LEA RDX, [name_42]
L_1800015ef:
    CALL qword ptr [__imp_GetProcAddress]
L_1800015f5:
    TEST RAX, RAX
L_1800015f8:
    JZ L_180001612
L_1800015fa:
    MOV RCX, R14
L_1800015fd:
    MOV RDX, RBX
L_180001600:
    MOV R8, RDI
L_180001603:
    MOV R9, RSI
L_180001606:
    ADD RSP, 028h
L_18000160a:
    POP RBX
L_18000160b:
    POP RDI
L_18000160c:
    POP RSI
L_18000160d:
    POP R14
L_18000160f:
    JMP RAX
L_180001612:
    XOR EAX, EAX
L_180001614:
    ADD RSP, 028h
L_180001618:
    POP RBX
L_180001619:
    POP RDI
L_18000161a:
    POP RSI
L_18000161b:
    POP R14
L_18000161d:
    RET
reconstructed_42 ENDP
; joyReleaseCapture @ 43; original VA 0x18000161e
; assembly ev_53f233431838d5edf19180f2a921349d684dc7363e27fc5feb40bdc88d1918e0; pseudocode ev_2dd1be4da4550860f0d98b0dbf30df7b994c3c2633a4aa1a57ec0e72648948f2
PUBLIC reconstructed_43
reconstructed_43 PROC FRAME
L_18000161e:
    PUSH R14
    .pushreg R14
L_180001620:
    PUSH RSI
    .pushreg RSI
L_180001621:
    PUSH RDI
    .pushreg RDI
L_180001622:
    PUSH RBX
    .pushreg RBX
L_180001623:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001627:
    MOV R14, RCX
L_18000162a:
    MOV RCX, qword ptr [OuterState+120]
L_180001631:
    TEST RCX, RCX
L_180001634:
    JZ L_180001669
L_180001636:
    MOV RSI, R9
L_180001639:
    MOV RDI, R8
L_18000163c:
    MOV RBX, RDX
L_18000163f:
    LEA RDX, [name_43]
L_180001646:
    CALL qword ptr [__imp_GetProcAddress]
L_18000164c:
    TEST RAX, RAX
L_18000164f:
    JZ L_180001669
L_180001651:
    MOV RCX, R14
L_180001654:
    MOV RDX, RBX
L_180001657:
    MOV R8, RDI
L_18000165a:
    MOV R9, RSI
L_18000165d:
    ADD RSP, 028h
L_180001661:
    POP RBX
L_180001662:
    POP RDI
L_180001663:
    POP RSI
L_180001664:
    POP R14
L_180001666:
    JMP RAX
L_180001669:
    XOR EAX, EAX
L_18000166b:
    ADD RSP, 028h
L_18000166f:
    POP RBX
L_180001670:
    POP RDI
L_180001671:
    POP RSI
L_180001672:
    POP R14
L_180001674:
    RET
reconstructed_43 ENDP
; joySetCapture @ 44; original VA 0x180001675
; assembly ev_afdc5dca2d2954338b621a6058aad6e3df69c4d3c7f90b4a7781bf84ec9366bc; pseudocode ev_bb49150a46334ed6e7244414ff4aa4c932cf8f0e8cda3bb7ef2dfc663a0948cb
PUBLIC reconstructed_44
reconstructed_44 PROC FRAME
L_180001675:
    PUSH R14
    .pushreg R14
L_180001677:
    PUSH RSI
    .pushreg RSI
L_180001678:
    PUSH RDI
    .pushreg RDI
L_180001679:
    PUSH RBX
    .pushreg RBX
L_18000167a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000167e:
    MOV R14, RCX
L_180001681:
    MOV RCX, qword ptr [OuterState+120]
L_180001688:
    TEST RCX, RCX
L_18000168b:
    JZ L_1800016c0
L_18000168d:
    MOV RSI, R9
L_180001690:
    MOV RDI, R8
L_180001693:
    MOV RBX, RDX
L_180001696:
    LEA RDX, [name_44]
L_18000169d:
    CALL qword ptr [__imp_GetProcAddress]
L_1800016a3:
    TEST RAX, RAX
L_1800016a6:
    JZ L_1800016c0
L_1800016a8:
    MOV RCX, R14
L_1800016ab:
    MOV RDX, RBX
L_1800016ae:
    MOV R8, RDI
L_1800016b1:
    MOV R9, RSI
L_1800016b4:
    ADD RSP, 028h
L_1800016b8:
    POP RBX
L_1800016b9:
    POP RDI
L_1800016ba:
    POP RSI
L_1800016bb:
    POP R14
L_1800016bd:
    JMP RAX
L_1800016c0:
    XOR EAX, EAX
L_1800016c2:
    ADD RSP, 028h
L_1800016c6:
    POP RBX
L_1800016c7:
    POP RDI
L_1800016c8:
    POP RSI
L_1800016c9:
    POP R14
L_1800016cb:
    RET
reconstructed_44 ENDP
; joySetThreshold @ 45; original VA 0x1800016cc
; assembly ev_cde795f643c78ff8337d416b05090e370051fc80d011d0e1bc591480bec83e8a; pseudocode ev_6fd567b59e5d578ef646735a19b33a71f3240d38d7e9eb97abdf046a1c42abc5
PUBLIC reconstructed_45
reconstructed_45 PROC FRAME
L_1800016cc:
    PUSH R14
    .pushreg R14
L_1800016ce:
    PUSH RSI
    .pushreg RSI
L_1800016cf:
    PUSH RDI
    .pushreg RDI
L_1800016d0:
    PUSH RBX
    .pushreg RBX
L_1800016d1:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800016d5:
    MOV R14, RCX
L_1800016d8:
    MOV RCX, qword ptr [OuterState+120]
L_1800016df:
    TEST RCX, RCX
L_1800016e2:
    JZ L_180001717
L_1800016e4:
    MOV RSI, R9
L_1800016e7:
    MOV RDI, R8
L_1800016ea:
    MOV RBX, RDX
L_1800016ed:
    LEA RDX, [name_45]
L_1800016f4:
    CALL qword ptr [__imp_GetProcAddress]
L_1800016fa:
    TEST RAX, RAX
L_1800016fd:
    JZ L_180001717
L_1800016ff:
    MOV RCX, R14
L_180001702:
    MOV RDX, RBX
L_180001705:
    MOV R8, RDI
L_180001708:
    MOV R9, RSI
L_18000170b:
    ADD RSP, 028h
L_18000170f:
    POP RBX
L_180001710:
    POP RDI
L_180001711:
    POP RSI
L_180001712:
    POP R14
L_180001714:
    JMP RAX
L_180001717:
    XOR EAX, EAX
L_180001719:
    ADD RSP, 028h
L_18000171d:
    POP RBX
L_18000171e:
    POP RDI
L_18000171f:
    POP RSI
L_180001720:
    POP R14
L_180001722:
    RET
reconstructed_45 ENDP
; mciDriverNotify @ 46; original VA 0x180001723
; assembly ev_c0094a6de2b940caf0d4d4bfe047d9e2fea13d8cbdb59bbe8b0670b89d60fa47; pseudocode ev_3e1c6faf3b3a59be65b8c6b27f2eca3b6d5911ae77fa92ee2eba961fec76263e
PUBLIC reconstructed_46
reconstructed_46 PROC FRAME
L_180001723:
    PUSH R14
    .pushreg R14
L_180001725:
    PUSH RSI
    .pushreg RSI
L_180001726:
    PUSH RDI
    .pushreg RDI
L_180001727:
    PUSH RBX
    .pushreg RBX
L_180001728:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000172c:
    MOV R14, RCX
L_18000172f:
    MOV RCX, qword ptr [OuterState+120]
L_180001736:
    TEST RCX, RCX
L_180001739:
    JZ L_18000176e
L_18000173b:
    MOV RSI, R9
L_18000173e:
    MOV RDI, R8
L_180001741:
    MOV RBX, RDX
L_180001744:
    LEA RDX, [name_46]
L_18000174b:
    CALL qword ptr [__imp_GetProcAddress]
L_180001751:
    TEST RAX, RAX
L_180001754:
    JZ L_18000176e
L_180001756:
    MOV RCX, R14
L_180001759:
    MOV RDX, RBX
L_18000175c:
    MOV R8, RDI
L_18000175f:
    MOV R9, RSI
L_180001762:
    ADD RSP, 028h
L_180001766:
    POP RBX
L_180001767:
    POP RDI
L_180001768:
    POP RSI
L_180001769:
    POP R14
L_18000176b:
    JMP RAX
L_18000176e:
    XOR EAX, EAX
L_180001770:
    ADD RSP, 028h
L_180001774:
    POP RBX
L_180001775:
    POP RDI
L_180001776:
    POP RSI
L_180001777:
    POP R14
L_180001779:
    RET
reconstructed_46 ENDP
; mciDriverYield @ 47; original VA 0x18000177a
; assembly ev_616450821af38f0bdbd68e1645e8161566e2f7ee4540b7884fb6d8e9259d9773; pseudocode ev_2cf457a4d3005c21480e7ed24fb72958c60f92920e7af6081eb14492048d0f68
PUBLIC reconstructed_47
reconstructed_47 PROC FRAME
L_18000177a:
    PUSH R14
    .pushreg R14
L_18000177c:
    PUSH RSI
    .pushreg RSI
L_18000177d:
    PUSH RDI
    .pushreg RDI
L_18000177e:
    PUSH RBX
    .pushreg RBX
L_18000177f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001783:
    MOV R14, RCX
L_180001786:
    MOV RCX, qword ptr [OuterState+120]
L_18000178d:
    TEST RCX, RCX
L_180001790:
    JZ L_1800017c5
L_180001792:
    MOV RSI, R9
L_180001795:
    MOV RDI, R8
L_180001798:
    MOV RBX, RDX
L_18000179b:
    LEA RDX, [name_47]
L_1800017a2:
    CALL qword ptr [__imp_GetProcAddress]
L_1800017a8:
    TEST RAX, RAX
L_1800017ab:
    JZ L_1800017c5
L_1800017ad:
    MOV RCX, R14
L_1800017b0:
    MOV RDX, RBX
L_1800017b3:
    MOV R8, RDI
L_1800017b6:
    MOV R9, RSI
L_1800017b9:
    ADD RSP, 028h
L_1800017bd:
    POP RBX
L_1800017be:
    POP RDI
L_1800017bf:
    POP RSI
L_1800017c0:
    POP R14
L_1800017c2:
    JMP RAX
L_1800017c5:
    XOR EAX, EAX
L_1800017c7:
    ADD RSP, 028h
L_1800017cb:
    POP RBX
L_1800017cc:
    POP RDI
L_1800017cd:
    POP RSI
L_1800017ce:
    POP R14
L_1800017d0:
    RET
reconstructed_47 ENDP
; mciExecute @ 48; original VA 0x1800017d1
; assembly ev_6a09b38b94aa83cdf80cd9900e64628572970ee4a0fb2cf693ceae34c81f3ab7; pseudocode ev_803122c0434db472dff29054ba556f2e51446fd0555bc1bb27293c770482cb73
PUBLIC reconstructed_48
reconstructed_48 PROC FRAME
L_1800017d1:
    PUSH R14
    .pushreg R14
L_1800017d3:
    PUSH RSI
    .pushreg RSI
L_1800017d4:
    PUSH RDI
    .pushreg RDI
L_1800017d5:
    PUSH RBX
    .pushreg RBX
L_1800017d6:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800017da:
    MOV R14, RCX
L_1800017dd:
    MOV RCX, qword ptr [OuterState+120]
L_1800017e4:
    TEST RCX, RCX
L_1800017e7:
    JZ L_18000181c
L_1800017e9:
    MOV RSI, R9
L_1800017ec:
    MOV RDI, R8
L_1800017ef:
    MOV RBX, RDX
L_1800017f2:
    LEA RDX, [name_48]
L_1800017f9:
    CALL qword ptr [__imp_GetProcAddress]
L_1800017ff:
    TEST RAX, RAX
L_180001802:
    JZ L_18000181c
L_180001804:
    MOV RCX, R14
L_180001807:
    MOV RDX, RBX
L_18000180a:
    MOV R8, RDI
L_18000180d:
    MOV R9, RSI
L_180001810:
    ADD RSP, 028h
L_180001814:
    POP RBX
L_180001815:
    POP RDI
L_180001816:
    POP RSI
L_180001817:
    POP R14
L_180001819:
    JMP RAX
L_18000181c:
    XOR EAX, EAX
L_18000181e:
    ADD RSP, 028h
L_180001822:
    POP RBX
L_180001823:
    POP RDI
L_180001824:
    POP RSI
L_180001825:
    POP R14
L_180001827:
    RET
reconstructed_48 ENDP
; mciFreeCommandResource @ 49; original VA 0x180001828
; assembly ev_40b4a279a8c3cab73c7983f94169cf0eb9c2bc8cac507a2b1843dd2ea4356628; pseudocode ev_1425525678d7edd0d6601bb3bffbaa7d52b26523bf701e9cecf13e23a472fd0b
PUBLIC reconstructed_49
reconstructed_49 PROC FRAME
L_180001828:
    PUSH R14
    .pushreg R14
L_18000182a:
    PUSH RSI
    .pushreg RSI
L_18000182b:
    PUSH RDI
    .pushreg RDI
L_18000182c:
    PUSH RBX
    .pushreg RBX
L_18000182d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001831:
    MOV R14, RCX
L_180001834:
    MOV RCX, qword ptr [OuterState+120]
L_18000183b:
    TEST RCX, RCX
L_18000183e:
    JZ L_180001873
L_180001840:
    MOV RSI, R9
L_180001843:
    MOV RDI, R8
L_180001846:
    MOV RBX, RDX
L_180001849:
    LEA RDX, [name_49]
L_180001850:
    CALL qword ptr [__imp_GetProcAddress]
L_180001856:
    TEST RAX, RAX
L_180001859:
    JZ L_180001873
L_18000185b:
    MOV RCX, R14
L_18000185e:
    MOV RDX, RBX
L_180001861:
    MOV R8, RDI
L_180001864:
    MOV R9, RSI
L_180001867:
    ADD RSP, 028h
L_18000186b:
    POP RBX
L_18000186c:
    POP RDI
L_18000186d:
    POP RSI
L_18000186e:
    POP R14
L_180001870:
    JMP RAX
L_180001873:
    XOR EAX, EAX
L_180001875:
    ADD RSP, 028h
L_180001879:
    POP RBX
L_18000187a:
    POP RDI
L_18000187b:
    POP RSI
L_18000187c:
    POP R14
L_18000187e:
    RET
reconstructed_49 ENDP
; mciGetCreatorTask @ 50; original VA 0x18000187f
; assembly ev_ce5f89bdd8ce5dc1fc06b59f17bb9ef3e618eda9d1275f70b2f7799039d4c4f9; pseudocode ev_50532e83978462c1a8cc701f4f684adc3fd41e601afc0ad52ca901260df3538d
PUBLIC reconstructed_50
reconstructed_50 PROC FRAME
L_18000187f:
    PUSH R14
    .pushreg R14
L_180001881:
    PUSH RSI
    .pushreg RSI
L_180001882:
    PUSH RDI
    .pushreg RDI
L_180001883:
    PUSH RBX
    .pushreg RBX
L_180001884:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001888:
    MOV R14, RCX
L_18000188b:
    MOV RCX, qword ptr [OuterState+120]
L_180001892:
    TEST RCX, RCX
L_180001895:
    JZ L_1800018ca
L_180001897:
    MOV RSI, R9
L_18000189a:
    MOV RDI, R8
L_18000189d:
    MOV RBX, RDX
L_1800018a0:
    LEA RDX, [name_50]
L_1800018a7:
    CALL qword ptr [__imp_GetProcAddress]
L_1800018ad:
    TEST RAX, RAX
L_1800018b0:
    JZ L_1800018ca
L_1800018b2:
    MOV RCX, R14
L_1800018b5:
    MOV RDX, RBX
L_1800018b8:
    MOV R8, RDI
L_1800018bb:
    MOV R9, RSI
L_1800018be:
    ADD RSP, 028h
L_1800018c2:
    POP RBX
L_1800018c3:
    POP RDI
L_1800018c4:
    POP RSI
L_1800018c5:
    POP R14
L_1800018c7:
    JMP RAX
L_1800018ca:
    XOR EAX, EAX
L_1800018cc:
    ADD RSP, 028h
L_1800018d0:
    POP RBX
L_1800018d1:
    POP RDI
L_1800018d2:
    POP RSI
L_1800018d3:
    POP R14
L_1800018d5:
    RET
reconstructed_50 ENDP
; mciGetDeviceIDA @ 51; original VA 0x1800018d6
; assembly ev_55e13aa2f2c58c2561b48bf98070639323d437dc569629e8850aca5ef6ab7d6c; pseudocode ev_6920fa089278f49d1762815df36e0178ae614111e0cabc2217651bf84ebe331b
PUBLIC reconstructed_51
reconstructed_51 PROC FRAME
L_1800018d6:
    PUSH R14
    .pushreg R14
L_1800018d8:
    PUSH RSI
    .pushreg RSI
L_1800018d9:
    PUSH RDI
    .pushreg RDI
L_1800018da:
    PUSH RBX
    .pushreg RBX
L_1800018db:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800018df:
    MOV R14, RCX
L_1800018e2:
    MOV RCX, qword ptr [OuterState+120]
L_1800018e9:
    TEST RCX, RCX
L_1800018ec:
    JZ L_180001921
L_1800018ee:
    MOV RSI, R9
L_1800018f1:
    MOV RDI, R8
L_1800018f4:
    MOV RBX, RDX
L_1800018f7:
    LEA RDX, [name_51]
L_1800018fe:
    CALL qword ptr [__imp_GetProcAddress]
L_180001904:
    TEST RAX, RAX
L_180001907:
    JZ L_180001921
L_180001909:
    MOV RCX, R14
L_18000190c:
    MOV RDX, RBX
L_18000190f:
    MOV R8, RDI
L_180001912:
    MOV R9, RSI
L_180001915:
    ADD RSP, 028h
L_180001919:
    POP RBX
L_18000191a:
    POP RDI
L_18000191b:
    POP RSI
L_18000191c:
    POP R14
L_18000191e:
    JMP RAX
L_180001921:
    XOR EAX, EAX
L_180001923:
    ADD RSP, 028h
L_180001927:
    POP RBX
L_180001928:
    POP RDI
L_180001929:
    POP RSI
L_18000192a:
    POP R14
L_18000192c:
    RET
reconstructed_51 ENDP
; mciGetDeviceIDFromElementIDA @ 52; original VA 0x18000192d
; assembly ev_074594361baac07629aa31e96ffa1ae831d253c6aade071da118d0f77b8ffa01; pseudocode ev_5cff4c72c69b4f3ee933bf169b34d2f0f8843eb314b53d1608fe7ced09f84c9a
PUBLIC reconstructed_52
reconstructed_52 PROC FRAME
L_18000192d:
    PUSH R14
    .pushreg R14
L_18000192f:
    PUSH RSI
    .pushreg RSI
L_180001930:
    PUSH RDI
    .pushreg RDI
L_180001931:
    PUSH RBX
    .pushreg RBX
L_180001932:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001936:
    MOV R14, RCX
L_180001939:
    MOV RCX, qword ptr [OuterState+120]
L_180001940:
    TEST RCX, RCX
L_180001943:
    JZ L_180001978
L_180001945:
    MOV RSI, R9
L_180001948:
    MOV RDI, R8
L_18000194b:
    MOV RBX, RDX
L_18000194e:
    LEA RDX, [name_52]
L_180001955:
    CALL qword ptr [__imp_GetProcAddress]
L_18000195b:
    TEST RAX, RAX
L_18000195e:
    JZ L_180001978
L_180001960:
    MOV RCX, R14
L_180001963:
    MOV RDX, RBX
L_180001966:
    MOV R8, RDI
L_180001969:
    MOV R9, RSI
L_18000196c:
    ADD RSP, 028h
L_180001970:
    POP RBX
L_180001971:
    POP RDI
L_180001972:
    POP RSI
L_180001973:
    POP R14
L_180001975:
    JMP RAX
L_180001978:
    XOR EAX, EAX
L_18000197a:
    ADD RSP, 028h
L_18000197e:
    POP RBX
L_18000197f:
    POP RDI
L_180001980:
    POP RSI
L_180001981:
    POP R14
L_180001983:
    RET
reconstructed_52 ENDP
; mciGetDeviceIDFromElementIDW @ 53; original VA 0x180001984
; assembly ev_13b3666b06a77090e1daf98956df7049017ba426bb9077ac06f49888949d0b0e; pseudocode ev_19a3ad31b1ab519149a9562fd034ff6e9803bd6864e4862431b78943e2498025
PUBLIC reconstructed_53
reconstructed_53 PROC FRAME
L_180001984:
    PUSH R14
    .pushreg R14
L_180001986:
    PUSH RSI
    .pushreg RSI
L_180001987:
    PUSH RDI
    .pushreg RDI
L_180001988:
    PUSH RBX
    .pushreg RBX
L_180001989:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000198d:
    MOV R14, RCX
L_180001990:
    MOV RCX, qword ptr [OuterState+120]
L_180001997:
    TEST RCX, RCX
L_18000199a:
    JZ L_1800019cf
L_18000199c:
    MOV RSI, R9
L_18000199f:
    MOV RDI, R8
L_1800019a2:
    MOV RBX, RDX
L_1800019a5:
    LEA RDX, [name_53]
L_1800019ac:
    CALL qword ptr [__imp_GetProcAddress]
L_1800019b2:
    TEST RAX, RAX
L_1800019b5:
    JZ L_1800019cf
L_1800019b7:
    MOV RCX, R14
L_1800019ba:
    MOV RDX, RBX
L_1800019bd:
    MOV R8, RDI
L_1800019c0:
    MOV R9, RSI
L_1800019c3:
    ADD RSP, 028h
L_1800019c7:
    POP RBX
L_1800019c8:
    POP RDI
L_1800019c9:
    POP RSI
L_1800019ca:
    POP R14
L_1800019cc:
    JMP RAX
L_1800019cf:
    XOR EAX, EAX
L_1800019d1:
    ADD RSP, 028h
L_1800019d5:
    POP RBX
L_1800019d6:
    POP RDI
L_1800019d7:
    POP RSI
L_1800019d8:
    POP R14
L_1800019da:
    RET
reconstructed_53 ENDP
; mciGetDeviceIDW @ 54; original VA 0x1800019db
; assembly ev_102d06d72082919c6e501128b1fcf2d2ae237586dd0e8e189e3661a9b52e1d11; pseudocode ev_abb7e499a8ea64e87bc5f4d8d10034d4f21da9453f11bd5a53f954f1c1eb334c
PUBLIC reconstructed_54
reconstructed_54 PROC FRAME
L_1800019db:
    PUSH R14
    .pushreg R14
L_1800019dd:
    PUSH RSI
    .pushreg RSI
L_1800019de:
    PUSH RDI
    .pushreg RDI
L_1800019df:
    PUSH RBX
    .pushreg RBX
L_1800019e0:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800019e4:
    MOV R14, RCX
L_1800019e7:
    MOV RCX, qword ptr [OuterState+120]
L_1800019ee:
    TEST RCX, RCX
L_1800019f1:
    JZ L_180001a26
L_1800019f3:
    MOV RSI, R9
L_1800019f6:
    MOV RDI, R8
L_1800019f9:
    MOV RBX, RDX
L_1800019fc:
    LEA RDX, [name_54]
L_180001a03:
    CALL qword ptr [__imp_GetProcAddress]
L_180001a09:
    TEST RAX, RAX
L_180001a0c:
    JZ L_180001a26
L_180001a0e:
    MOV RCX, R14
L_180001a11:
    MOV RDX, RBX
L_180001a14:
    MOV R8, RDI
L_180001a17:
    MOV R9, RSI
L_180001a1a:
    ADD RSP, 028h
L_180001a1e:
    POP RBX
L_180001a1f:
    POP RDI
L_180001a20:
    POP RSI
L_180001a21:
    POP R14
L_180001a23:
    JMP RAX
L_180001a26:
    XOR EAX, EAX
L_180001a28:
    ADD RSP, 028h
L_180001a2c:
    POP RBX
L_180001a2d:
    POP RDI
L_180001a2e:
    POP RSI
L_180001a2f:
    POP R14
L_180001a31:
    RET
reconstructed_54 ENDP
; mciGetDriverData @ 55; original VA 0x180001a32
; assembly ev_1e93ee7e715e5f3787e1e85dba8a56d0321aa8fa507a37db5f99d145a35fc8a2; pseudocode ev_e5b0ffdf639c7fa7c3ff4daec57821bfed01c68b0e3fa78ad2f2cda01db32c1f
PUBLIC reconstructed_55
reconstructed_55 PROC FRAME
L_180001a32:
    PUSH R14
    .pushreg R14
L_180001a34:
    PUSH RSI
    .pushreg RSI
L_180001a35:
    PUSH RDI
    .pushreg RDI
L_180001a36:
    PUSH RBX
    .pushreg RBX
L_180001a37:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001a3b:
    MOV R14, RCX
L_180001a3e:
    MOV RCX, qword ptr [OuterState+120]
L_180001a45:
    TEST RCX, RCX
L_180001a48:
    JZ L_180001a7d
L_180001a4a:
    MOV RSI, R9
L_180001a4d:
    MOV RDI, R8
L_180001a50:
    MOV RBX, RDX
L_180001a53:
    LEA RDX, [name_55]
L_180001a5a:
    CALL qword ptr [__imp_GetProcAddress]
L_180001a60:
    TEST RAX, RAX
L_180001a63:
    JZ L_180001a7d
L_180001a65:
    MOV RCX, R14
L_180001a68:
    MOV RDX, RBX
L_180001a6b:
    MOV R8, RDI
L_180001a6e:
    MOV R9, RSI
L_180001a71:
    ADD RSP, 028h
L_180001a75:
    POP RBX
L_180001a76:
    POP RDI
L_180001a77:
    POP RSI
L_180001a78:
    POP R14
L_180001a7a:
    JMP RAX
L_180001a7d:
    XOR EAX, EAX
L_180001a7f:
    ADD RSP, 028h
L_180001a83:
    POP RBX
L_180001a84:
    POP RDI
L_180001a85:
    POP RSI
L_180001a86:
    POP R14
L_180001a88:
    RET
reconstructed_55 ENDP
; mciGetErrorStringA @ 56; original VA 0x180001a89
; assembly ev_d5d8a13abd23125b9e416d1590223fa294ad3bcdd8ffda1e31f2b6f472667db3; pseudocode ev_b9a1cdc97d680615cbb6122cb740d86786d36a9a9022d4f02373181862c5c6c5
PUBLIC reconstructed_56
reconstructed_56 PROC FRAME
L_180001a89:
    PUSH R14
    .pushreg R14
L_180001a8b:
    PUSH RSI
    .pushreg RSI
L_180001a8c:
    PUSH RDI
    .pushreg RDI
L_180001a8d:
    PUSH RBX
    .pushreg RBX
L_180001a8e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001a92:
    MOV R14, RCX
L_180001a95:
    MOV RCX, qword ptr [OuterState+120]
L_180001a9c:
    TEST RCX, RCX
L_180001a9f:
    JZ L_180001ad4
L_180001aa1:
    MOV RSI, R9
L_180001aa4:
    MOV RDI, R8
L_180001aa7:
    MOV RBX, RDX
L_180001aaa:
    LEA RDX, [name_56]
L_180001ab1:
    CALL qword ptr [__imp_GetProcAddress]
L_180001ab7:
    TEST RAX, RAX
L_180001aba:
    JZ L_180001ad4
L_180001abc:
    MOV RCX, R14
L_180001abf:
    MOV RDX, RBX
L_180001ac2:
    MOV R8, RDI
L_180001ac5:
    MOV R9, RSI
L_180001ac8:
    ADD RSP, 028h
L_180001acc:
    POP RBX
L_180001acd:
    POP RDI
L_180001ace:
    POP RSI
L_180001acf:
    POP R14
L_180001ad1:
    JMP RAX
L_180001ad4:
    XOR EAX, EAX
L_180001ad6:
    ADD RSP, 028h
L_180001ada:
    POP RBX
L_180001adb:
    POP RDI
L_180001adc:
    POP RSI
L_180001add:
    POP R14
L_180001adf:
    RET
reconstructed_56 ENDP
; mciGetErrorStringW @ 57; original VA 0x180001ae0
; assembly ev_3ef263bba93f589b681d760a131d9c4f54806a75c6bf6c8d96e10c17cdfc46cb; pseudocode ev_5057cf5d772f93140a567020c57e85b6c6493a40f5dd72f83a087b8d1692ec96
PUBLIC reconstructed_57
reconstructed_57 PROC FRAME
L_180001ae0:
    PUSH R14
    .pushreg R14
L_180001ae2:
    PUSH RSI
    .pushreg RSI
L_180001ae3:
    PUSH RDI
    .pushreg RDI
L_180001ae4:
    PUSH RBX
    .pushreg RBX
L_180001ae5:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001ae9:
    MOV R14, RCX
L_180001aec:
    MOV RCX, qword ptr [OuterState+120]
L_180001af3:
    TEST RCX, RCX
L_180001af6:
    JZ L_180001b2b
L_180001af8:
    MOV RSI, R9
L_180001afb:
    MOV RDI, R8
L_180001afe:
    MOV RBX, RDX
L_180001b01:
    LEA RDX, [name_57]
L_180001b08:
    CALL qword ptr [__imp_GetProcAddress]
L_180001b0e:
    TEST RAX, RAX
L_180001b11:
    JZ L_180001b2b
L_180001b13:
    MOV RCX, R14
L_180001b16:
    MOV RDX, RBX
L_180001b19:
    MOV R8, RDI
L_180001b1c:
    MOV R9, RSI
L_180001b1f:
    ADD RSP, 028h
L_180001b23:
    POP RBX
L_180001b24:
    POP RDI
L_180001b25:
    POP RSI
L_180001b26:
    POP R14
L_180001b28:
    JMP RAX
L_180001b2b:
    XOR EAX, EAX
L_180001b2d:
    ADD RSP, 028h
L_180001b31:
    POP RBX
L_180001b32:
    POP RDI
L_180001b33:
    POP RSI
L_180001b34:
    POP R14
L_180001b36:
    RET
reconstructed_57 ENDP
; mciGetYieldProc @ 58; original VA 0x180001b37
; assembly ev_52545aef83d386d91766e562d9c052bb1bd2815b051b2376ae72a396465bab4f; pseudocode ev_e67cd0c89fe089c4235684e3f112078b6134a5efcee0d8fda371a6847b9767f9
PUBLIC reconstructed_58
reconstructed_58 PROC FRAME
L_180001b37:
    PUSH R14
    .pushreg R14
L_180001b39:
    PUSH RSI
    .pushreg RSI
L_180001b3a:
    PUSH RDI
    .pushreg RDI
L_180001b3b:
    PUSH RBX
    .pushreg RBX
L_180001b3c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001b40:
    MOV R14, RCX
L_180001b43:
    MOV RCX, qword ptr [OuterState+120]
L_180001b4a:
    TEST RCX, RCX
L_180001b4d:
    JZ L_180001b82
L_180001b4f:
    MOV RSI, R9
L_180001b52:
    MOV RDI, R8
L_180001b55:
    MOV RBX, RDX
L_180001b58:
    LEA RDX, [name_58]
L_180001b5f:
    CALL qword ptr [__imp_GetProcAddress]
L_180001b65:
    TEST RAX, RAX
L_180001b68:
    JZ L_180001b82
L_180001b6a:
    MOV RCX, R14
L_180001b6d:
    MOV RDX, RBX
L_180001b70:
    MOV R8, RDI
L_180001b73:
    MOV R9, RSI
L_180001b76:
    ADD RSP, 028h
L_180001b7a:
    POP RBX
L_180001b7b:
    POP RDI
L_180001b7c:
    POP RSI
L_180001b7d:
    POP R14
L_180001b7f:
    JMP RAX
L_180001b82:
    XOR EAX, EAX
L_180001b84:
    ADD RSP, 028h
L_180001b88:
    POP RBX
L_180001b89:
    POP RDI
L_180001b8a:
    POP RSI
L_180001b8b:
    POP R14
L_180001b8d:
    RET
reconstructed_58 ENDP
; mciLoadCommandResource @ 59; original VA 0x180001b8e
; assembly ev_de7cf22a21eb6935495f7194817bb855ea2c3882a4018e71722bd6fba9abae90; pseudocode ev_69a41ede3743b5fbfcc94c74112b6fe686e31cc202d497ea4f2aa977704c50c0
PUBLIC reconstructed_59
reconstructed_59 PROC FRAME
L_180001b8e:
    PUSH R14
    .pushreg R14
L_180001b90:
    PUSH RSI
    .pushreg RSI
L_180001b91:
    PUSH RDI
    .pushreg RDI
L_180001b92:
    PUSH RBX
    .pushreg RBX
L_180001b93:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001b97:
    MOV R14, RCX
L_180001b9a:
    MOV RCX, qword ptr [OuterState+120]
L_180001ba1:
    TEST RCX, RCX
L_180001ba4:
    JZ L_180001bd9
L_180001ba6:
    MOV RSI, R9
L_180001ba9:
    MOV RDI, R8
L_180001bac:
    MOV RBX, RDX
L_180001baf:
    LEA RDX, [name_59]
L_180001bb6:
    CALL qword ptr [__imp_GetProcAddress]
L_180001bbc:
    TEST RAX, RAX
L_180001bbf:
    JZ L_180001bd9
L_180001bc1:
    MOV RCX, R14
L_180001bc4:
    MOV RDX, RBX
L_180001bc7:
    MOV R8, RDI
L_180001bca:
    MOV R9, RSI
L_180001bcd:
    ADD RSP, 028h
L_180001bd1:
    POP RBX
L_180001bd2:
    POP RDI
L_180001bd3:
    POP RSI
L_180001bd4:
    POP R14
L_180001bd6:
    JMP RAX
L_180001bd9:
    XOR EAX, EAX
L_180001bdb:
    ADD RSP, 028h
L_180001bdf:
    POP RBX
L_180001be0:
    POP RDI
L_180001be1:
    POP RSI
L_180001be2:
    POP R14
L_180001be4:
    RET
reconstructed_59 ENDP
; mciSendCommandA @ 60; original VA 0x180001be5
; assembly ev_c99ce2561536205eb1519401fb4736e90ab68a3cc441d1c1453cfb7f8c919a96; pseudocode ev_4de96558ff5e080418d0f4666cd6a904a0a09a12ec8aa34c9535440faae33fe5
PUBLIC reconstructed_60
reconstructed_60 PROC FRAME
L_180001be5:
    PUSH R14
    .pushreg R14
L_180001be7:
    PUSH RSI
    .pushreg RSI
L_180001be8:
    PUSH RDI
    .pushreg RDI
L_180001be9:
    PUSH RBX
    .pushreg RBX
L_180001bea:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001bee:
    MOV R14, RCX
L_180001bf1:
    MOV RCX, qword ptr [OuterState+120]
L_180001bf8:
    TEST RCX, RCX
L_180001bfb:
    JZ L_180001c30
L_180001bfd:
    MOV RSI, R9
L_180001c00:
    MOV RDI, R8
L_180001c03:
    MOV RBX, RDX
L_180001c06:
    LEA RDX, [name_60]
L_180001c0d:
    CALL qword ptr [__imp_GetProcAddress]
L_180001c13:
    TEST RAX, RAX
L_180001c16:
    JZ L_180001c30
L_180001c18:
    MOV RCX, R14
L_180001c1b:
    MOV RDX, RBX
L_180001c1e:
    MOV R8, RDI
L_180001c21:
    MOV R9, RSI
L_180001c24:
    ADD RSP, 028h
L_180001c28:
    POP RBX
L_180001c29:
    POP RDI
L_180001c2a:
    POP RSI
L_180001c2b:
    POP R14
L_180001c2d:
    JMP RAX
L_180001c30:
    XOR EAX, EAX
L_180001c32:
    ADD RSP, 028h
L_180001c36:
    POP RBX
L_180001c37:
    POP RDI
L_180001c38:
    POP RSI
L_180001c39:
    POP R14
L_180001c3b:
    RET
reconstructed_60 ENDP
; mciSendCommandW @ 61; original VA 0x180001c3c
; assembly ev_5deedd977ca87dbc776e5cb2c9d7a8f480e0ae6ee43cc6f83aa42e03e41d8f3a; pseudocode ev_9b5bf14e1757176847cf89b0dccf1925113e4c89d196da0bd43613eea3f4da1e
PUBLIC reconstructed_61
reconstructed_61 PROC FRAME
L_180001c3c:
    PUSH R14
    .pushreg R14
L_180001c3e:
    PUSH RSI
    .pushreg RSI
L_180001c3f:
    PUSH RDI
    .pushreg RDI
L_180001c40:
    PUSH RBX
    .pushreg RBX
L_180001c41:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001c45:
    MOV R14, RCX
L_180001c48:
    MOV RCX, qword ptr [OuterState+120]
L_180001c4f:
    TEST RCX, RCX
L_180001c52:
    JZ L_180001c87
L_180001c54:
    MOV RSI, R9
L_180001c57:
    MOV RDI, R8
L_180001c5a:
    MOV RBX, RDX
L_180001c5d:
    LEA RDX, [name_61]
L_180001c64:
    CALL qword ptr [__imp_GetProcAddress]
L_180001c6a:
    TEST RAX, RAX
L_180001c6d:
    JZ L_180001c87
L_180001c6f:
    MOV RCX, R14
L_180001c72:
    MOV RDX, RBX
L_180001c75:
    MOV R8, RDI
L_180001c78:
    MOV R9, RSI
L_180001c7b:
    ADD RSP, 028h
L_180001c7f:
    POP RBX
L_180001c80:
    POP RDI
L_180001c81:
    POP RSI
L_180001c82:
    POP R14
L_180001c84:
    JMP RAX
L_180001c87:
    XOR EAX, EAX
L_180001c89:
    ADD RSP, 028h
L_180001c8d:
    POP RBX
L_180001c8e:
    POP RDI
L_180001c8f:
    POP RSI
L_180001c90:
    POP R14
L_180001c92:
    RET
reconstructed_61 ENDP
; mciSendStringA @ 62; original VA 0x180001c93
; assembly ev_ecce73ae7f6ebb3e4bffbd249a79fe382fb3e028c321f8a7eb088e9ece700a79; pseudocode ev_37f72c7436a5be541fb43a360e6d79bf56022e44a6012939148f2c34aa0b6480
PUBLIC reconstructed_62
reconstructed_62 PROC FRAME
L_180001c93:
    PUSH R14
    .pushreg R14
L_180001c95:
    PUSH RSI
    .pushreg RSI
L_180001c96:
    PUSH RDI
    .pushreg RDI
L_180001c97:
    PUSH RBX
    .pushreg RBX
L_180001c98:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001c9c:
    MOV R14, RCX
L_180001c9f:
    MOV RCX, qword ptr [OuterState+120]
L_180001ca6:
    TEST RCX, RCX
L_180001ca9:
    JZ L_180001cde
L_180001cab:
    MOV RSI, R9
L_180001cae:
    MOV RDI, R8
L_180001cb1:
    MOV RBX, RDX
L_180001cb4:
    LEA RDX, [name_62]
L_180001cbb:
    CALL qword ptr [__imp_GetProcAddress]
L_180001cc1:
    TEST RAX, RAX
L_180001cc4:
    JZ L_180001cde
L_180001cc6:
    MOV RCX, R14
L_180001cc9:
    MOV RDX, RBX
L_180001ccc:
    MOV R8, RDI
L_180001ccf:
    MOV R9, RSI
L_180001cd2:
    ADD RSP, 028h
L_180001cd6:
    POP RBX
L_180001cd7:
    POP RDI
L_180001cd8:
    POP RSI
L_180001cd9:
    POP R14
L_180001cdb:
    JMP RAX
L_180001cde:
    XOR EAX, EAX
L_180001ce0:
    ADD RSP, 028h
L_180001ce4:
    POP RBX
L_180001ce5:
    POP RDI
L_180001ce6:
    POP RSI
L_180001ce7:
    POP R14
L_180001ce9:
    RET
reconstructed_62 ENDP
; mciSendStringW @ 63; original VA 0x180001cea
; assembly ev_d22144a8a98d7edd81162b381640c3c8c4cb0f676974c31f9d14c39e572b2352; pseudocode ev_78f6dc98aba1f2c07f61f1415a61963db3bee58e76d770f2b645858fd8600efa
PUBLIC reconstructed_63
reconstructed_63 PROC FRAME
L_180001cea:
    PUSH R14
    .pushreg R14
L_180001cec:
    PUSH RSI
    .pushreg RSI
L_180001ced:
    PUSH RDI
    .pushreg RDI
L_180001cee:
    PUSH RBX
    .pushreg RBX
L_180001cef:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001cf3:
    MOV R14, RCX
L_180001cf6:
    MOV RCX, qword ptr [OuterState+120]
L_180001cfd:
    TEST RCX, RCX
L_180001d00:
    JZ L_180001d35
L_180001d02:
    MOV RSI, R9
L_180001d05:
    MOV RDI, R8
L_180001d08:
    MOV RBX, RDX
L_180001d0b:
    LEA RDX, [name_63]
L_180001d12:
    CALL qword ptr [__imp_GetProcAddress]
L_180001d18:
    TEST RAX, RAX
L_180001d1b:
    JZ L_180001d35
L_180001d1d:
    MOV RCX, R14
L_180001d20:
    MOV RDX, RBX
L_180001d23:
    MOV R8, RDI
L_180001d26:
    MOV R9, RSI
L_180001d29:
    ADD RSP, 028h
L_180001d2d:
    POP RBX
L_180001d2e:
    POP RDI
L_180001d2f:
    POP RSI
L_180001d30:
    POP R14
L_180001d32:
    JMP RAX
L_180001d35:
    XOR EAX, EAX
L_180001d37:
    ADD RSP, 028h
L_180001d3b:
    POP RBX
L_180001d3c:
    POP RDI
L_180001d3d:
    POP RSI
L_180001d3e:
    POP R14
L_180001d40:
    RET
reconstructed_63 ENDP
; mciSetDriverData @ 64; original VA 0x180001d41
; assembly ev_bc64394876a18a45a6e69e5782cd0979fde80bf92673d2a51f5f70e33cc88aa2; pseudocode ev_219aa40a2f00e8888e3438e5eb9473b64e8371fbf63b9f070893c074e3e8c60a
PUBLIC reconstructed_64
reconstructed_64 PROC FRAME
L_180001d41:
    PUSH R14
    .pushreg R14
L_180001d43:
    PUSH RSI
    .pushreg RSI
L_180001d44:
    PUSH RDI
    .pushreg RDI
L_180001d45:
    PUSH RBX
    .pushreg RBX
L_180001d46:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001d4a:
    MOV R14, RCX
L_180001d4d:
    MOV RCX, qword ptr [OuterState+120]
L_180001d54:
    TEST RCX, RCX
L_180001d57:
    JZ L_180001d8c
L_180001d59:
    MOV RSI, R9
L_180001d5c:
    MOV RDI, R8
L_180001d5f:
    MOV RBX, RDX
L_180001d62:
    LEA RDX, [name_64]
L_180001d69:
    CALL qword ptr [__imp_GetProcAddress]
L_180001d6f:
    TEST RAX, RAX
L_180001d72:
    JZ L_180001d8c
L_180001d74:
    MOV RCX, R14
L_180001d77:
    MOV RDX, RBX
L_180001d7a:
    MOV R8, RDI
L_180001d7d:
    MOV R9, RSI
L_180001d80:
    ADD RSP, 028h
L_180001d84:
    POP RBX
L_180001d85:
    POP RDI
L_180001d86:
    POP RSI
L_180001d87:
    POP R14
L_180001d89:
    JMP RAX
L_180001d8c:
    XOR EAX, EAX
L_180001d8e:
    ADD RSP, 028h
L_180001d92:
    POP RBX
L_180001d93:
    POP RDI
L_180001d94:
    POP RSI
L_180001d95:
    POP R14
L_180001d97:
    RET
reconstructed_64 ENDP
; mciSetYieldProc @ 65; original VA 0x180001d98
; assembly ev_3bbb5192ee25eb78b8868328a4505c9814cbf6eeb3419ef689054f6a47b37a02; pseudocode ev_5a2600aa2eeb5e9abad478288f3375cf6d14d1bd14abc3a17990415e9ea74757
PUBLIC reconstructed_65
reconstructed_65 PROC FRAME
L_180001d98:
    PUSH R14
    .pushreg R14
L_180001d9a:
    PUSH RSI
    .pushreg RSI
L_180001d9b:
    PUSH RDI
    .pushreg RDI
L_180001d9c:
    PUSH RBX
    .pushreg RBX
L_180001d9d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001da1:
    MOV R14, RCX
L_180001da4:
    MOV RCX, qword ptr [OuterState+120]
L_180001dab:
    TEST RCX, RCX
L_180001dae:
    JZ L_180001de3
L_180001db0:
    MOV RSI, R9
L_180001db3:
    MOV RDI, R8
L_180001db6:
    MOV RBX, RDX
L_180001db9:
    LEA RDX, [name_65]
L_180001dc0:
    CALL qword ptr [__imp_GetProcAddress]
L_180001dc6:
    TEST RAX, RAX
L_180001dc9:
    JZ L_180001de3
L_180001dcb:
    MOV RCX, R14
L_180001dce:
    MOV RDX, RBX
L_180001dd1:
    MOV R8, RDI
L_180001dd4:
    MOV R9, RSI
L_180001dd7:
    ADD RSP, 028h
L_180001ddb:
    POP RBX
L_180001ddc:
    POP RDI
L_180001ddd:
    POP RSI
L_180001dde:
    POP R14
L_180001de0:
    JMP RAX
L_180001de3:
    XOR EAX, EAX
L_180001de5:
    ADD RSP, 028h
L_180001de9:
    POP RBX
L_180001dea:
    POP RDI
L_180001deb:
    POP RSI
L_180001dec:
    POP R14
L_180001dee:
    RET
reconstructed_65 ENDP
; midiConnect @ 66; original VA 0x180001def
; assembly ev_f9785e7d4c12f7a2cd8f293638180e7aae3c6968d5c603bd78ec0a6faa1bede0; pseudocode ev_eafa11dd110aaf8cafa71d4bf73652b44dc34baa2673ccd0e4c2f5eea087c65b
PUBLIC reconstructed_66
reconstructed_66 PROC FRAME
L_180001def:
    PUSH R14
    .pushreg R14
L_180001df1:
    PUSH RSI
    .pushreg RSI
L_180001df2:
    PUSH RDI
    .pushreg RDI
L_180001df3:
    PUSH RBX
    .pushreg RBX
L_180001df4:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001df8:
    MOV R14, RCX
L_180001dfb:
    MOV RCX, qword ptr [OuterState+120]
L_180001e02:
    TEST RCX, RCX
L_180001e05:
    JZ L_180001e3a
L_180001e07:
    MOV RSI, R9
L_180001e0a:
    MOV RDI, R8
L_180001e0d:
    MOV RBX, RDX
L_180001e10:
    LEA RDX, [name_66]
L_180001e17:
    CALL qword ptr [__imp_GetProcAddress]
L_180001e1d:
    TEST RAX, RAX
L_180001e20:
    JZ L_180001e3a
L_180001e22:
    MOV RCX, R14
L_180001e25:
    MOV RDX, RBX
L_180001e28:
    MOV R8, RDI
L_180001e2b:
    MOV R9, RSI
L_180001e2e:
    ADD RSP, 028h
L_180001e32:
    POP RBX
L_180001e33:
    POP RDI
L_180001e34:
    POP RSI
L_180001e35:
    POP R14
L_180001e37:
    JMP RAX
L_180001e3a:
    XOR EAX, EAX
L_180001e3c:
    ADD RSP, 028h
L_180001e40:
    POP RBX
L_180001e41:
    POP RDI
L_180001e42:
    POP RSI
L_180001e43:
    POP R14
L_180001e45:
    RET
reconstructed_66 ENDP
; midiDisconnect @ 67; original VA 0x180001e46
; assembly ev_93d706eac83dcc54edde809f442637cd27a27dca39d9a1253fd64f9c7d2dd6a4; pseudocode ev_ae2a9fc9af0cbb6eb56b2aae514f2f9db9c9241b7f4f6cf72002272749e16786
PUBLIC reconstructed_67
reconstructed_67 PROC FRAME
L_180001e46:
    PUSH R14
    .pushreg R14
L_180001e48:
    PUSH RSI
    .pushreg RSI
L_180001e49:
    PUSH RDI
    .pushreg RDI
L_180001e4a:
    PUSH RBX
    .pushreg RBX
L_180001e4b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001e4f:
    MOV R14, RCX
L_180001e52:
    MOV RCX, qword ptr [OuterState+120]
L_180001e59:
    TEST RCX, RCX
L_180001e5c:
    JZ L_180001e91
L_180001e5e:
    MOV RSI, R9
L_180001e61:
    MOV RDI, R8
L_180001e64:
    MOV RBX, RDX
L_180001e67:
    LEA RDX, [name_67]
L_180001e6e:
    CALL qword ptr [__imp_GetProcAddress]
L_180001e74:
    TEST RAX, RAX
L_180001e77:
    JZ L_180001e91
L_180001e79:
    MOV RCX, R14
L_180001e7c:
    MOV RDX, RBX
L_180001e7f:
    MOV R8, RDI
L_180001e82:
    MOV R9, RSI
L_180001e85:
    ADD RSP, 028h
L_180001e89:
    POP RBX
L_180001e8a:
    POP RDI
L_180001e8b:
    POP RSI
L_180001e8c:
    POP R14
L_180001e8e:
    JMP RAX
L_180001e91:
    XOR EAX, EAX
L_180001e93:
    ADD RSP, 028h
L_180001e97:
    POP RBX
L_180001e98:
    POP RDI
L_180001e99:
    POP RSI
L_180001e9a:
    POP R14
L_180001e9c:
    RET
reconstructed_67 ENDP
; midiInAddBuffer @ 68; original VA 0x180001e9d
; assembly ev_34a417cfff86221bab5699f6fab6c214998103ea10d84132ee7d7aec79d51bfc; pseudocode ev_2d936d719d5f521c1203d0a51a4acdb3ba4c0b2e456b6e90e8934d940d1ad8f3
PUBLIC reconstructed_68
reconstructed_68 PROC FRAME
L_180001e9d:
    PUSH R14
    .pushreg R14
L_180001e9f:
    PUSH RSI
    .pushreg RSI
L_180001ea0:
    PUSH RDI
    .pushreg RDI
L_180001ea1:
    PUSH RBX
    .pushreg RBX
L_180001ea2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001ea6:
    MOV R14, RCX
L_180001ea9:
    MOV RCX, qword ptr [OuterState+120]
L_180001eb0:
    TEST RCX, RCX
L_180001eb3:
    JZ L_180001ee8
L_180001eb5:
    MOV RSI, R9
L_180001eb8:
    MOV RDI, R8
L_180001ebb:
    MOV RBX, RDX
L_180001ebe:
    LEA RDX, [name_68]
L_180001ec5:
    CALL qword ptr [__imp_GetProcAddress]
L_180001ecb:
    TEST RAX, RAX
L_180001ece:
    JZ L_180001ee8
L_180001ed0:
    MOV RCX, R14
L_180001ed3:
    MOV RDX, RBX
L_180001ed6:
    MOV R8, RDI
L_180001ed9:
    MOV R9, RSI
L_180001edc:
    ADD RSP, 028h
L_180001ee0:
    POP RBX
L_180001ee1:
    POP RDI
L_180001ee2:
    POP RSI
L_180001ee3:
    POP R14
L_180001ee5:
    JMP RAX
L_180001ee8:
    XOR EAX, EAX
L_180001eea:
    ADD RSP, 028h
L_180001eee:
    POP RBX
L_180001eef:
    POP RDI
L_180001ef0:
    POP RSI
L_180001ef1:
    POP R14
L_180001ef3:
    RET
reconstructed_68 ENDP
; midiInClose @ 69; original VA 0x180001ef4
; assembly ev_6f585d24e1e05b608a96ca860e4617092ba451d8f3661ea0e1fbe65d214a0c72; pseudocode ev_4596529ece2668464046eab9beabed91b8bb5ea649adda71f8906ad2a20b5bbe
PUBLIC reconstructed_69
reconstructed_69 PROC FRAME
L_180001ef4:
    PUSH R14
    .pushreg R14
L_180001ef6:
    PUSH RSI
    .pushreg RSI
L_180001ef7:
    PUSH RDI
    .pushreg RDI
L_180001ef8:
    PUSH RBX
    .pushreg RBX
L_180001ef9:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001efd:
    MOV R14, RCX
L_180001f00:
    MOV RCX, qword ptr [OuterState+120]
L_180001f07:
    TEST RCX, RCX
L_180001f0a:
    JZ L_180001f3f
L_180001f0c:
    MOV RSI, R9
L_180001f0f:
    MOV RDI, R8
L_180001f12:
    MOV RBX, RDX
L_180001f15:
    LEA RDX, [name_69]
L_180001f1c:
    CALL qword ptr [__imp_GetProcAddress]
L_180001f22:
    TEST RAX, RAX
L_180001f25:
    JZ L_180001f3f
L_180001f27:
    MOV RCX, R14
L_180001f2a:
    MOV RDX, RBX
L_180001f2d:
    MOV R8, RDI
L_180001f30:
    MOV R9, RSI
L_180001f33:
    ADD RSP, 028h
L_180001f37:
    POP RBX
L_180001f38:
    POP RDI
L_180001f39:
    POP RSI
L_180001f3a:
    POP R14
L_180001f3c:
    JMP RAX
L_180001f3f:
    XOR EAX, EAX
L_180001f41:
    ADD RSP, 028h
L_180001f45:
    POP RBX
L_180001f46:
    POP RDI
L_180001f47:
    POP RSI
L_180001f48:
    POP R14
L_180001f4a:
    RET
reconstructed_69 ENDP
; midiInGetDevCapsA @ 70; original VA 0x180001f4b
; assembly ev_89fdb479c6f02dfd9c8765e25951cfc0287d319e6164af269d06f55815e06404; pseudocode ev_7ff02f4b6a38d69f5a4aa80d8e6d21cee8232a8aded2430134c9d0149eb7f30a
PUBLIC reconstructed_70
reconstructed_70 PROC FRAME
L_180001f4b:
    PUSH R14
    .pushreg R14
L_180001f4d:
    PUSH RSI
    .pushreg RSI
L_180001f4e:
    PUSH RDI
    .pushreg RDI
L_180001f4f:
    PUSH RBX
    .pushreg RBX
L_180001f50:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001f54:
    MOV R14, RCX
L_180001f57:
    MOV RCX, qword ptr [OuterState+120]
L_180001f5e:
    TEST RCX, RCX
L_180001f61:
    JZ L_180001f96
L_180001f63:
    MOV RSI, R9
L_180001f66:
    MOV RDI, R8
L_180001f69:
    MOV RBX, RDX
L_180001f6c:
    LEA RDX, [name_70]
L_180001f73:
    CALL qword ptr [__imp_GetProcAddress]
L_180001f79:
    TEST RAX, RAX
L_180001f7c:
    JZ L_180001f96
L_180001f7e:
    MOV RCX, R14
L_180001f81:
    MOV RDX, RBX
L_180001f84:
    MOV R8, RDI
L_180001f87:
    MOV R9, RSI
L_180001f8a:
    ADD RSP, 028h
L_180001f8e:
    POP RBX
L_180001f8f:
    POP RDI
L_180001f90:
    POP RSI
L_180001f91:
    POP R14
L_180001f93:
    JMP RAX
L_180001f96:
    XOR EAX, EAX
L_180001f98:
    ADD RSP, 028h
L_180001f9c:
    POP RBX
L_180001f9d:
    POP RDI
L_180001f9e:
    POP RSI
L_180001f9f:
    POP R14
L_180001fa1:
    RET
reconstructed_70 ENDP
; midiInGetDevCapsW @ 71; original VA 0x180001fa2
; assembly ev_49d498429deccb1a87cb3726b8d0dac7afb4b0cf55490b8c68d23c83859e3e56; pseudocode ev_2787e5a6e3b72722a197af6b2ab0fb8f2e5708911c940c25eb6d617ee1e03c24
PUBLIC reconstructed_71
reconstructed_71 PROC FRAME
L_180001fa2:
    PUSH R14
    .pushreg R14
L_180001fa4:
    PUSH RSI
    .pushreg RSI
L_180001fa5:
    PUSH RDI
    .pushreg RDI
L_180001fa6:
    PUSH RBX
    .pushreg RBX
L_180001fa7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180001fab:
    MOV R14, RCX
L_180001fae:
    MOV RCX, qword ptr [OuterState+120]
L_180001fb5:
    TEST RCX, RCX
L_180001fb8:
    JZ L_180001fed
L_180001fba:
    MOV RSI, R9
L_180001fbd:
    MOV RDI, R8
L_180001fc0:
    MOV RBX, RDX
L_180001fc3:
    LEA RDX, [name_71]
L_180001fca:
    CALL qword ptr [__imp_GetProcAddress]
L_180001fd0:
    TEST RAX, RAX
L_180001fd3:
    JZ L_180001fed
L_180001fd5:
    MOV RCX, R14
L_180001fd8:
    MOV RDX, RBX
L_180001fdb:
    MOV R8, RDI
L_180001fde:
    MOV R9, RSI
L_180001fe1:
    ADD RSP, 028h
L_180001fe5:
    POP RBX
L_180001fe6:
    POP RDI
L_180001fe7:
    POP RSI
L_180001fe8:
    POP R14
L_180001fea:
    JMP RAX
L_180001fed:
    XOR EAX, EAX
L_180001fef:
    ADD RSP, 028h
L_180001ff3:
    POP RBX
L_180001ff4:
    POP RDI
L_180001ff5:
    POP RSI
L_180001ff6:
    POP R14
L_180001ff8:
    RET
reconstructed_71 ENDP
; midiInGetErrorTextA @ 72; original VA 0x180001ff9
; assembly ev_39b008ada6eff812785bb25124df9107ac1472dd03a72ea4e62d0d47c49c4bd7; pseudocode ev_4188fa7ec183b6c8dcf516f7eb27e7a31911cf1e5aaf15a372b1307b7ec8ae51
PUBLIC reconstructed_72
reconstructed_72 PROC FRAME
L_180001ff9:
    PUSH R14
    .pushreg R14
L_180001ffb:
    PUSH RSI
    .pushreg RSI
L_180001ffc:
    PUSH RDI
    .pushreg RDI
L_180001ffd:
    PUSH RBX
    .pushreg RBX
L_180001ffe:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002002:
    MOV R14, RCX
L_180002005:
    MOV RCX, qword ptr [OuterState+120]
L_18000200c:
    TEST RCX, RCX
L_18000200f:
    JZ L_180002044
L_180002011:
    MOV RSI, R9
L_180002014:
    MOV RDI, R8
L_180002017:
    MOV RBX, RDX
L_18000201a:
    LEA RDX, [name_72]
L_180002021:
    CALL qword ptr [__imp_GetProcAddress]
L_180002027:
    TEST RAX, RAX
L_18000202a:
    JZ L_180002044
L_18000202c:
    MOV RCX, R14
L_18000202f:
    MOV RDX, RBX
L_180002032:
    MOV R8, RDI
L_180002035:
    MOV R9, RSI
L_180002038:
    ADD RSP, 028h
L_18000203c:
    POP RBX
L_18000203d:
    POP RDI
L_18000203e:
    POP RSI
L_18000203f:
    POP R14
L_180002041:
    JMP RAX
L_180002044:
    XOR EAX, EAX
L_180002046:
    ADD RSP, 028h
L_18000204a:
    POP RBX
L_18000204b:
    POP RDI
L_18000204c:
    POP RSI
L_18000204d:
    POP R14
L_18000204f:
    RET
reconstructed_72 ENDP
; midiInGetErrorTextW @ 73; original VA 0x180002050
; assembly ev_f0d044be5086a51f5fb2efcf4d5ee76bf462810a7311f16f41be6e62ebbfd6ad; pseudocode ev_3c17d744962c7160d4a8e23e9c24c874594092f8b52fde28dfb1b2eca618be7e
PUBLIC reconstructed_73
reconstructed_73 PROC FRAME
L_180002050:
    PUSH R14
    .pushreg R14
L_180002052:
    PUSH RSI
    .pushreg RSI
L_180002053:
    PUSH RDI
    .pushreg RDI
L_180002054:
    PUSH RBX
    .pushreg RBX
L_180002055:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002059:
    MOV R14, RCX
L_18000205c:
    MOV RCX, qword ptr [OuterState+120]
L_180002063:
    TEST RCX, RCX
L_180002066:
    JZ L_18000209b
L_180002068:
    MOV RSI, R9
L_18000206b:
    MOV RDI, R8
L_18000206e:
    MOV RBX, RDX
L_180002071:
    LEA RDX, [name_73]
L_180002078:
    CALL qword ptr [__imp_GetProcAddress]
L_18000207e:
    TEST RAX, RAX
L_180002081:
    JZ L_18000209b
L_180002083:
    MOV RCX, R14
L_180002086:
    MOV RDX, RBX
L_180002089:
    MOV R8, RDI
L_18000208c:
    MOV R9, RSI
L_18000208f:
    ADD RSP, 028h
L_180002093:
    POP RBX
L_180002094:
    POP RDI
L_180002095:
    POP RSI
L_180002096:
    POP R14
L_180002098:
    JMP RAX
L_18000209b:
    XOR EAX, EAX
L_18000209d:
    ADD RSP, 028h
L_1800020a1:
    POP RBX
L_1800020a2:
    POP RDI
L_1800020a3:
    POP RSI
L_1800020a4:
    POP R14
L_1800020a6:
    RET
reconstructed_73 ENDP
; midiInGetID @ 74; original VA 0x1800020a7
; assembly ev_e352912c72e12c24202836fa113e7793e9764debf5fd5e340e638e6eab438b9c; pseudocode ev_10902760b74deb9e3cf8518892de6ffdbc2aeb90d710d2fbc3f9c6aca13d9aaa
PUBLIC reconstructed_74
reconstructed_74 PROC FRAME
L_1800020a7:
    PUSH R14
    .pushreg R14
L_1800020a9:
    PUSH RSI
    .pushreg RSI
L_1800020aa:
    PUSH RDI
    .pushreg RDI
L_1800020ab:
    PUSH RBX
    .pushreg RBX
L_1800020ac:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800020b0:
    MOV R14, RCX
L_1800020b3:
    MOV RCX, qword ptr [OuterState+120]
L_1800020ba:
    TEST RCX, RCX
L_1800020bd:
    JZ L_1800020f2
L_1800020bf:
    MOV RSI, R9
L_1800020c2:
    MOV RDI, R8
L_1800020c5:
    MOV RBX, RDX
L_1800020c8:
    LEA RDX, [name_74]
L_1800020cf:
    CALL qword ptr [__imp_GetProcAddress]
L_1800020d5:
    TEST RAX, RAX
L_1800020d8:
    JZ L_1800020f2
L_1800020da:
    MOV RCX, R14
L_1800020dd:
    MOV RDX, RBX
L_1800020e0:
    MOV R8, RDI
L_1800020e3:
    MOV R9, RSI
L_1800020e6:
    ADD RSP, 028h
L_1800020ea:
    POP RBX
L_1800020eb:
    POP RDI
L_1800020ec:
    POP RSI
L_1800020ed:
    POP R14
L_1800020ef:
    JMP RAX
L_1800020f2:
    XOR EAX, EAX
L_1800020f4:
    ADD RSP, 028h
L_1800020f8:
    POP RBX
L_1800020f9:
    POP RDI
L_1800020fa:
    POP RSI
L_1800020fb:
    POP R14
L_1800020fd:
    RET
reconstructed_74 ENDP
; midiInGetNumDevs @ 75; original VA 0x1800020fe
; assembly ev_81821cbbf827caa46384e38e3b1b8220c4142eb82ef4ebf5856abd2ac1421cdb; pseudocode ev_c5c49f3f0fd5292e9c22253ac6e2be508cb9dc655055b4aae63a8be9e245c930
PUBLIC reconstructed_75
reconstructed_75 PROC FRAME
L_1800020fe:
    PUSH R14
    .pushreg R14
L_180002100:
    PUSH RSI
    .pushreg RSI
L_180002101:
    PUSH RDI
    .pushreg RDI
L_180002102:
    PUSH RBX
    .pushreg RBX
L_180002103:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002107:
    MOV R14, RCX
L_18000210a:
    MOV RCX, qword ptr [OuterState+120]
L_180002111:
    TEST RCX, RCX
L_180002114:
    JZ L_180002149
L_180002116:
    MOV RSI, R9
L_180002119:
    MOV RDI, R8
L_18000211c:
    MOV RBX, RDX
L_18000211f:
    LEA RDX, [name_75]
L_180002126:
    CALL qword ptr [__imp_GetProcAddress]
L_18000212c:
    TEST RAX, RAX
L_18000212f:
    JZ L_180002149
L_180002131:
    MOV RCX, R14
L_180002134:
    MOV RDX, RBX
L_180002137:
    MOV R8, RDI
L_18000213a:
    MOV R9, RSI
L_18000213d:
    ADD RSP, 028h
L_180002141:
    POP RBX
L_180002142:
    POP RDI
L_180002143:
    POP RSI
L_180002144:
    POP R14
L_180002146:
    JMP RAX
L_180002149:
    XOR EAX, EAX
L_18000214b:
    ADD RSP, 028h
L_18000214f:
    POP RBX
L_180002150:
    POP RDI
L_180002151:
    POP RSI
L_180002152:
    POP R14
L_180002154:
    RET
reconstructed_75 ENDP
; midiInMessage @ 76; original VA 0x180002155
; assembly ev_2d46cea0d90a85cf5bc999f7fd0248309465dccb645ae328ef1e849097ac8b53; pseudocode ev_4b93654863ab2b74766819fe5fa684b4e6a2fc58f2a1c392e191ea6c6cf328fa
PUBLIC reconstructed_76
reconstructed_76 PROC FRAME
L_180002155:
    PUSH R14
    .pushreg R14
L_180002157:
    PUSH RSI
    .pushreg RSI
L_180002158:
    PUSH RDI
    .pushreg RDI
L_180002159:
    PUSH RBX
    .pushreg RBX
L_18000215a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000215e:
    MOV R14, RCX
L_180002161:
    MOV RCX, qword ptr [OuterState+120]
L_180002168:
    TEST RCX, RCX
L_18000216b:
    JZ L_1800021a0
L_18000216d:
    MOV RSI, R9
L_180002170:
    MOV RDI, R8
L_180002173:
    MOV RBX, RDX
L_180002176:
    LEA RDX, [name_76]
L_18000217d:
    CALL qword ptr [__imp_GetProcAddress]
L_180002183:
    TEST RAX, RAX
L_180002186:
    JZ L_1800021a0
L_180002188:
    MOV RCX, R14
L_18000218b:
    MOV RDX, RBX
L_18000218e:
    MOV R8, RDI
L_180002191:
    MOV R9, RSI
L_180002194:
    ADD RSP, 028h
L_180002198:
    POP RBX
L_180002199:
    POP RDI
L_18000219a:
    POP RSI
L_18000219b:
    POP R14
L_18000219d:
    JMP RAX
L_1800021a0:
    XOR EAX, EAX
L_1800021a2:
    ADD RSP, 028h
L_1800021a6:
    POP RBX
L_1800021a7:
    POP RDI
L_1800021a8:
    POP RSI
L_1800021a9:
    POP R14
L_1800021ab:
    RET
reconstructed_76 ENDP
; midiInOpen @ 77; original VA 0x1800021ac
; assembly ev_eccad328fa8202c55e24bc22812295799aa559e4f7f031723bf8c4e2467f5b88; pseudocode ev_11dc5c8a1952aaf6c2eb5b5259fa61db952b65d5e20e9b16464429e377235e20
PUBLIC reconstructed_77
reconstructed_77 PROC FRAME
L_1800021ac:
    PUSH R14
    .pushreg R14
L_1800021ae:
    PUSH RSI
    .pushreg RSI
L_1800021af:
    PUSH RDI
    .pushreg RDI
L_1800021b0:
    PUSH RBX
    .pushreg RBX
L_1800021b1:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800021b5:
    MOV R14, RCX
L_1800021b8:
    MOV RCX, qword ptr [OuterState+120]
L_1800021bf:
    TEST RCX, RCX
L_1800021c2:
    JZ L_1800021f7
L_1800021c4:
    MOV RSI, R9
L_1800021c7:
    MOV RDI, R8
L_1800021ca:
    MOV RBX, RDX
L_1800021cd:
    LEA RDX, [name_77]
L_1800021d4:
    CALL qword ptr [__imp_GetProcAddress]
L_1800021da:
    TEST RAX, RAX
L_1800021dd:
    JZ L_1800021f7
L_1800021df:
    MOV RCX, R14
L_1800021e2:
    MOV RDX, RBX
L_1800021e5:
    MOV R8, RDI
L_1800021e8:
    MOV R9, RSI
L_1800021eb:
    ADD RSP, 028h
L_1800021ef:
    POP RBX
L_1800021f0:
    POP RDI
L_1800021f1:
    POP RSI
L_1800021f2:
    POP R14
L_1800021f4:
    JMP RAX
L_1800021f7:
    XOR EAX, EAX
L_1800021f9:
    ADD RSP, 028h
L_1800021fd:
    POP RBX
L_1800021fe:
    POP RDI
L_1800021ff:
    POP RSI
L_180002200:
    POP R14
L_180002202:
    RET
reconstructed_77 ENDP
; midiInPrepareHeader @ 78; original VA 0x180002203
; assembly ev_b3ccbd68a4b63387b85049ed5581d098864a70f8cfa10f9b1291535073d64ca9; pseudocode ev_b19728ee159c978320e4b94d883aeecbecf7913d29323ea8e932b4d6857051a8
PUBLIC reconstructed_78
reconstructed_78 PROC FRAME
L_180002203:
    PUSH R14
    .pushreg R14
L_180002205:
    PUSH RSI
    .pushreg RSI
L_180002206:
    PUSH RDI
    .pushreg RDI
L_180002207:
    PUSH RBX
    .pushreg RBX
L_180002208:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000220c:
    MOV R14, RCX
L_18000220f:
    MOV RCX, qword ptr [OuterState+120]
L_180002216:
    TEST RCX, RCX
L_180002219:
    JZ L_18000224e
L_18000221b:
    MOV RSI, R9
L_18000221e:
    MOV RDI, R8
L_180002221:
    MOV RBX, RDX
L_180002224:
    LEA RDX, [name_78]
L_18000222b:
    CALL qword ptr [__imp_GetProcAddress]
L_180002231:
    TEST RAX, RAX
L_180002234:
    JZ L_18000224e
L_180002236:
    MOV RCX, R14
L_180002239:
    MOV RDX, RBX
L_18000223c:
    MOV R8, RDI
L_18000223f:
    MOV R9, RSI
L_180002242:
    ADD RSP, 028h
L_180002246:
    POP RBX
L_180002247:
    POP RDI
L_180002248:
    POP RSI
L_180002249:
    POP R14
L_18000224b:
    JMP RAX
L_18000224e:
    XOR EAX, EAX
L_180002250:
    ADD RSP, 028h
L_180002254:
    POP RBX
L_180002255:
    POP RDI
L_180002256:
    POP RSI
L_180002257:
    POP R14
L_180002259:
    RET
reconstructed_78 ENDP
; midiInReset @ 79; original VA 0x18000225a
; assembly ev_80dd18f49c2a1fef67fffa92f678ed044fce3c877fb4fa8c8fec59413266ab99; pseudocode ev_e8041b9bbe2cd9cdececd02c696493218a92ba5323d21b0221db4f7fe23ac4fd
PUBLIC reconstructed_79
reconstructed_79 PROC FRAME
L_18000225a:
    PUSH R14
    .pushreg R14
L_18000225c:
    PUSH RSI
    .pushreg RSI
L_18000225d:
    PUSH RDI
    .pushreg RDI
L_18000225e:
    PUSH RBX
    .pushreg RBX
L_18000225f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002263:
    MOV R14, RCX
L_180002266:
    MOV RCX, qword ptr [OuterState+120]
L_18000226d:
    TEST RCX, RCX
L_180002270:
    JZ L_1800022a5
L_180002272:
    MOV RSI, R9
L_180002275:
    MOV RDI, R8
L_180002278:
    MOV RBX, RDX
L_18000227b:
    LEA RDX, [name_79]
L_180002282:
    CALL qword ptr [__imp_GetProcAddress]
L_180002288:
    TEST RAX, RAX
L_18000228b:
    JZ L_1800022a5
L_18000228d:
    MOV RCX, R14
L_180002290:
    MOV RDX, RBX
L_180002293:
    MOV R8, RDI
L_180002296:
    MOV R9, RSI
L_180002299:
    ADD RSP, 028h
L_18000229d:
    POP RBX
L_18000229e:
    POP RDI
L_18000229f:
    POP RSI
L_1800022a0:
    POP R14
L_1800022a2:
    JMP RAX
L_1800022a5:
    XOR EAX, EAX
L_1800022a7:
    ADD RSP, 028h
L_1800022ab:
    POP RBX
L_1800022ac:
    POP RDI
L_1800022ad:
    POP RSI
L_1800022ae:
    POP R14
L_1800022b0:
    RET
reconstructed_79 ENDP
; midiInStart @ 80; original VA 0x1800022b1
; assembly ev_0562513aa6f034be4cd64e48e260a2492047fc71712ab9507d627f4a0170f811; pseudocode ev_245195e8fd81907a58715812edba65b8c355fdd399c923c0badb95756e4a41e2
PUBLIC reconstructed_80
reconstructed_80 PROC FRAME
L_1800022b1:
    PUSH R14
    .pushreg R14
L_1800022b3:
    PUSH RSI
    .pushreg RSI
L_1800022b4:
    PUSH RDI
    .pushreg RDI
L_1800022b5:
    PUSH RBX
    .pushreg RBX
L_1800022b6:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800022ba:
    MOV R14, RCX
L_1800022bd:
    MOV RCX, qword ptr [OuterState+120]
L_1800022c4:
    TEST RCX, RCX
L_1800022c7:
    JZ L_1800022fc
L_1800022c9:
    MOV RSI, R9
L_1800022cc:
    MOV RDI, R8
L_1800022cf:
    MOV RBX, RDX
L_1800022d2:
    LEA RDX, [name_80]
L_1800022d9:
    CALL qword ptr [__imp_GetProcAddress]
L_1800022df:
    TEST RAX, RAX
L_1800022e2:
    JZ L_1800022fc
L_1800022e4:
    MOV RCX, R14
L_1800022e7:
    MOV RDX, RBX
L_1800022ea:
    MOV R8, RDI
L_1800022ed:
    MOV R9, RSI
L_1800022f0:
    ADD RSP, 028h
L_1800022f4:
    POP RBX
L_1800022f5:
    POP RDI
L_1800022f6:
    POP RSI
L_1800022f7:
    POP R14
L_1800022f9:
    JMP RAX
L_1800022fc:
    XOR EAX, EAX
L_1800022fe:
    ADD RSP, 028h
L_180002302:
    POP RBX
L_180002303:
    POP RDI
L_180002304:
    POP RSI
L_180002305:
    POP R14
L_180002307:
    RET
reconstructed_80 ENDP
; midiInStop @ 81; original VA 0x180002308
; assembly ev_a8921b0370bbd99d68aaa0a8df1feca18af6f25fec5c1c418e8c3e2c66c4a197; pseudocode ev_cf283cef33b22ec4adb039167cb1b1fd3def4a635f4510f243285bec8fca755c
PUBLIC reconstructed_81
reconstructed_81 PROC FRAME
L_180002308:
    PUSH R14
    .pushreg R14
L_18000230a:
    PUSH RSI
    .pushreg RSI
L_18000230b:
    PUSH RDI
    .pushreg RDI
L_18000230c:
    PUSH RBX
    .pushreg RBX
L_18000230d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002311:
    MOV R14, RCX
L_180002314:
    MOV RCX, qword ptr [OuterState+120]
L_18000231b:
    TEST RCX, RCX
L_18000231e:
    JZ L_180002353
L_180002320:
    MOV RSI, R9
L_180002323:
    MOV RDI, R8
L_180002326:
    MOV RBX, RDX
L_180002329:
    LEA RDX, [name_81]
L_180002330:
    CALL qword ptr [__imp_GetProcAddress]
L_180002336:
    TEST RAX, RAX
L_180002339:
    JZ L_180002353
L_18000233b:
    MOV RCX, R14
L_18000233e:
    MOV RDX, RBX
L_180002341:
    MOV R8, RDI
L_180002344:
    MOV R9, RSI
L_180002347:
    ADD RSP, 028h
L_18000234b:
    POP RBX
L_18000234c:
    POP RDI
L_18000234d:
    POP RSI
L_18000234e:
    POP R14
L_180002350:
    JMP RAX
L_180002353:
    XOR EAX, EAX
L_180002355:
    ADD RSP, 028h
L_180002359:
    POP RBX
L_18000235a:
    POP RDI
L_18000235b:
    POP RSI
L_18000235c:
    POP R14
L_18000235e:
    RET
reconstructed_81 ENDP
; midiInUnprepareHeader @ 82; original VA 0x18000235f
; assembly ev_d995f7a22969b8f0b802738492c17ed5e421f58cd1f21f0aa7db680fdc31553e; pseudocode ev_43543e553610eba8e2b21190c192627f88e262524595f1ec6ef68dc0305af56d
PUBLIC reconstructed_82
reconstructed_82 PROC FRAME
L_18000235f:
    PUSH R14
    .pushreg R14
L_180002361:
    PUSH RSI
    .pushreg RSI
L_180002362:
    PUSH RDI
    .pushreg RDI
L_180002363:
    PUSH RBX
    .pushreg RBX
L_180002364:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002368:
    MOV R14, RCX
L_18000236b:
    MOV RCX, qword ptr [OuterState+120]
L_180002372:
    TEST RCX, RCX
L_180002375:
    JZ L_1800023aa
L_180002377:
    MOV RSI, R9
L_18000237a:
    MOV RDI, R8
L_18000237d:
    MOV RBX, RDX
L_180002380:
    LEA RDX, [name_82]
L_180002387:
    CALL qword ptr [__imp_GetProcAddress]
L_18000238d:
    TEST RAX, RAX
L_180002390:
    JZ L_1800023aa
L_180002392:
    MOV RCX, R14
L_180002395:
    MOV RDX, RBX
L_180002398:
    MOV R8, RDI
L_18000239b:
    MOV R9, RSI
L_18000239e:
    ADD RSP, 028h
L_1800023a2:
    POP RBX
L_1800023a3:
    POP RDI
L_1800023a4:
    POP RSI
L_1800023a5:
    POP R14
L_1800023a7:
    JMP RAX
L_1800023aa:
    XOR EAX, EAX
L_1800023ac:
    ADD RSP, 028h
L_1800023b0:
    POP RBX
L_1800023b1:
    POP RDI
L_1800023b2:
    POP RSI
L_1800023b3:
    POP R14
L_1800023b5:
    RET
reconstructed_82 ENDP
; midiOutCacheDrumPatches @ 83; original VA 0x1800023b6
; assembly ev_b38a65b63ae1200884181f74a2e295343c2d629b354095d260bbe16b0d610b1b; pseudocode ev_34423c1c19cba444b73721bce8c768737e60c3991f1576e987a36f65b2ec4b60
PUBLIC reconstructed_83
reconstructed_83 PROC FRAME
L_1800023b6:
    PUSH R14
    .pushreg R14
L_1800023b8:
    PUSH RSI
    .pushreg RSI
L_1800023b9:
    PUSH RDI
    .pushreg RDI
L_1800023ba:
    PUSH RBX
    .pushreg RBX
L_1800023bb:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800023bf:
    MOV R14, RCX
L_1800023c2:
    MOV RCX, qword ptr [OuterState+120]
L_1800023c9:
    TEST RCX, RCX
L_1800023cc:
    JZ L_180002401
L_1800023ce:
    MOV RSI, R9
L_1800023d1:
    MOV RDI, R8
L_1800023d4:
    MOV RBX, RDX
L_1800023d7:
    LEA RDX, [name_83]
L_1800023de:
    CALL qword ptr [__imp_GetProcAddress]
L_1800023e4:
    TEST RAX, RAX
L_1800023e7:
    JZ L_180002401
L_1800023e9:
    MOV RCX, R14
L_1800023ec:
    MOV RDX, RBX
L_1800023ef:
    MOV R8, RDI
L_1800023f2:
    MOV R9, RSI
L_1800023f5:
    ADD RSP, 028h
L_1800023f9:
    POP RBX
L_1800023fa:
    POP RDI
L_1800023fb:
    POP RSI
L_1800023fc:
    POP R14
L_1800023fe:
    JMP RAX
L_180002401:
    XOR EAX, EAX
L_180002403:
    ADD RSP, 028h
L_180002407:
    POP RBX
L_180002408:
    POP RDI
L_180002409:
    POP RSI
L_18000240a:
    POP R14
L_18000240c:
    RET
reconstructed_83 ENDP
; midiOutCachePatches @ 84; original VA 0x18000240d
; assembly ev_b1e8d90a962fe29628ec3d9ee913edd17c2691c9a8f15e1acc87bc50b819cd67; pseudocode ev_a83195ded9b1b3927be5a57642c4646c48f7a6469be4456b5db08b756fba1b63
PUBLIC reconstructed_84
reconstructed_84 PROC FRAME
L_18000240d:
    PUSH R14
    .pushreg R14
L_18000240f:
    PUSH RSI
    .pushreg RSI
L_180002410:
    PUSH RDI
    .pushreg RDI
L_180002411:
    PUSH RBX
    .pushreg RBX
L_180002412:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002416:
    MOV R14, RCX
L_180002419:
    MOV RCX, qword ptr [OuterState+120]
L_180002420:
    TEST RCX, RCX
L_180002423:
    JZ L_180002458
L_180002425:
    MOV RSI, R9
L_180002428:
    MOV RDI, R8
L_18000242b:
    MOV RBX, RDX
L_18000242e:
    LEA RDX, [name_84]
L_180002435:
    CALL qword ptr [__imp_GetProcAddress]
L_18000243b:
    TEST RAX, RAX
L_18000243e:
    JZ L_180002458
L_180002440:
    MOV RCX, R14
L_180002443:
    MOV RDX, RBX
L_180002446:
    MOV R8, RDI
L_180002449:
    MOV R9, RSI
L_18000244c:
    ADD RSP, 028h
L_180002450:
    POP RBX
L_180002451:
    POP RDI
L_180002452:
    POP RSI
L_180002453:
    POP R14
L_180002455:
    JMP RAX
L_180002458:
    XOR EAX, EAX
L_18000245a:
    ADD RSP, 028h
L_18000245e:
    POP RBX
L_18000245f:
    POP RDI
L_180002460:
    POP RSI
L_180002461:
    POP R14
L_180002463:
    RET
reconstructed_84 ENDP
; midiOutClose @ 85; original VA 0x180002464
; assembly ev_28c589dfa31acf6ce1682c1bb1b2eb3e5001903af1fa0d0109de00f367ce540f; pseudocode ev_48794252e180c75c8d4407d1a0cc0b4937d4b0b9942fded0368a59837365390e
PUBLIC reconstructed_85
reconstructed_85 PROC FRAME
L_180002464:
    PUSH R14
    .pushreg R14
L_180002466:
    PUSH RSI
    .pushreg RSI
L_180002467:
    PUSH RDI
    .pushreg RDI
L_180002468:
    PUSH RBX
    .pushreg RBX
L_180002469:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000246d:
    MOV R14, RCX
L_180002470:
    MOV RCX, qword ptr [OuterState+120]
L_180002477:
    TEST RCX, RCX
L_18000247a:
    JZ L_1800024af
L_18000247c:
    MOV RSI, R9
L_18000247f:
    MOV RDI, R8
L_180002482:
    MOV RBX, RDX
L_180002485:
    LEA RDX, [name_85]
L_18000248c:
    CALL qword ptr [__imp_GetProcAddress]
L_180002492:
    TEST RAX, RAX
L_180002495:
    JZ L_1800024af
L_180002497:
    MOV RCX, R14
L_18000249a:
    MOV RDX, RBX
L_18000249d:
    MOV R8, RDI
L_1800024a0:
    MOV R9, RSI
L_1800024a3:
    ADD RSP, 028h
L_1800024a7:
    POP RBX
L_1800024a8:
    POP RDI
L_1800024a9:
    POP RSI
L_1800024aa:
    POP R14
L_1800024ac:
    JMP RAX
L_1800024af:
    XOR EAX, EAX
L_1800024b1:
    ADD RSP, 028h
L_1800024b5:
    POP RBX
L_1800024b6:
    POP RDI
L_1800024b7:
    POP RSI
L_1800024b8:
    POP R14
L_1800024ba:
    RET
reconstructed_85 ENDP
; midiOutGetDevCapsA @ 86; original VA 0x1800024bb
; assembly ev_271e4dfe549dd37f44e0141cfffa10099e7ccee246d350350a33a6a032d393a4; pseudocode ev_080f372ec886ab56c146e210c7f1fea8ff6a6b69b099bb31489dba06fb335fa7
PUBLIC reconstructed_86
reconstructed_86 PROC FRAME
L_1800024bb:
    PUSH R14
    .pushreg R14
L_1800024bd:
    PUSH RSI
    .pushreg RSI
L_1800024be:
    PUSH RDI
    .pushreg RDI
L_1800024bf:
    PUSH RBX
    .pushreg RBX
L_1800024c0:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800024c4:
    MOV R14, RCX
L_1800024c7:
    MOV RCX, qword ptr [OuterState+120]
L_1800024ce:
    TEST RCX, RCX
L_1800024d1:
    JZ L_180002506
L_1800024d3:
    MOV RSI, R9
L_1800024d6:
    MOV RDI, R8
L_1800024d9:
    MOV RBX, RDX
L_1800024dc:
    LEA RDX, [name_86]
L_1800024e3:
    CALL qword ptr [__imp_GetProcAddress]
L_1800024e9:
    TEST RAX, RAX
L_1800024ec:
    JZ L_180002506
L_1800024ee:
    MOV RCX, R14
L_1800024f1:
    MOV RDX, RBX
L_1800024f4:
    MOV R8, RDI
L_1800024f7:
    MOV R9, RSI
L_1800024fa:
    ADD RSP, 028h
L_1800024fe:
    POP RBX
L_1800024ff:
    POP RDI
L_180002500:
    POP RSI
L_180002501:
    POP R14
L_180002503:
    JMP RAX
L_180002506:
    XOR EAX, EAX
L_180002508:
    ADD RSP, 028h
L_18000250c:
    POP RBX
L_18000250d:
    POP RDI
L_18000250e:
    POP RSI
L_18000250f:
    POP R14
L_180002511:
    RET
reconstructed_86 ENDP
; midiOutGetDevCapsW @ 87; original VA 0x180002512
; assembly ev_1f5e671d0c31e3b1a603a45b0223382d2170a2d39587d442e01f6d2fa2f92810; pseudocode ev_85b84d0fb6801d03f312417c5147593b3bc56ca95fc02f639b6bd8fec9c563b3
PUBLIC reconstructed_87
reconstructed_87 PROC FRAME
L_180002512:
    PUSH R14
    .pushreg R14
L_180002514:
    PUSH RSI
    .pushreg RSI
L_180002515:
    PUSH RDI
    .pushreg RDI
L_180002516:
    PUSH RBX
    .pushreg RBX
L_180002517:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000251b:
    MOV R14, RCX
L_18000251e:
    MOV RCX, qword ptr [OuterState+120]
L_180002525:
    TEST RCX, RCX
L_180002528:
    JZ L_18000255d
L_18000252a:
    MOV RSI, R9
L_18000252d:
    MOV RDI, R8
L_180002530:
    MOV RBX, RDX
L_180002533:
    LEA RDX, [name_87]
L_18000253a:
    CALL qword ptr [__imp_GetProcAddress]
L_180002540:
    TEST RAX, RAX
L_180002543:
    JZ L_18000255d
L_180002545:
    MOV RCX, R14
L_180002548:
    MOV RDX, RBX
L_18000254b:
    MOV R8, RDI
L_18000254e:
    MOV R9, RSI
L_180002551:
    ADD RSP, 028h
L_180002555:
    POP RBX
L_180002556:
    POP RDI
L_180002557:
    POP RSI
L_180002558:
    POP R14
L_18000255a:
    JMP RAX
L_18000255d:
    XOR EAX, EAX
L_18000255f:
    ADD RSP, 028h
L_180002563:
    POP RBX
L_180002564:
    POP RDI
L_180002565:
    POP RSI
L_180002566:
    POP R14
L_180002568:
    RET
reconstructed_87 ENDP
; midiOutGetErrorTextA @ 88; original VA 0x180002569
; assembly ev_f484f540a381096eea601513f25bad06eb262121bd39daecc04768c6c8b550ed; pseudocode ev_ceed456343ab2e1fabbb276a9913e532f506a326f03422811e0053f71949a82a
PUBLIC reconstructed_88
reconstructed_88 PROC FRAME
L_180002569:
    PUSH R14
    .pushreg R14
L_18000256b:
    PUSH RSI
    .pushreg RSI
L_18000256c:
    PUSH RDI
    .pushreg RDI
L_18000256d:
    PUSH RBX
    .pushreg RBX
L_18000256e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002572:
    MOV R14, RCX
L_180002575:
    MOV RCX, qword ptr [OuterState+120]
L_18000257c:
    TEST RCX, RCX
L_18000257f:
    JZ L_1800025b4
L_180002581:
    MOV RSI, R9
L_180002584:
    MOV RDI, R8
L_180002587:
    MOV RBX, RDX
L_18000258a:
    LEA RDX, [name_88]
L_180002591:
    CALL qword ptr [__imp_GetProcAddress]
L_180002597:
    TEST RAX, RAX
L_18000259a:
    JZ L_1800025b4
L_18000259c:
    MOV RCX, R14
L_18000259f:
    MOV RDX, RBX
L_1800025a2:
    MOV R8, RDI
L_1800025a5:
    MOV R9, RSI
L_1800025a8:
    ADD RSP, 028h
L_1800025ac:
    POP RBX
L_1800025ad:
    POP RDI
L_1800025ae:
    POP RSI
L_1800025af:
    POP R14
L_1800025b1:
    JMP RAX
L_1800025b4:
    XOR EAX, EAX
L_1800025b6:
    ADD RSP, 028h
L_1800025ba:
    POP RBX
L_1800025bb:
    POP RDI
L_1800025bc:
    POP RSI
L_1800025bd:
    POP R14
L_1800025bf:
    RET
reconstructed_88 ENDP
; midiOutGetErrorTextW @ 89; original VA 0x1800025c0
; assembly ev_43c354dddd92c452c5f739fd17ea0aa22de01acd9fd578daadd96c680b8f1d75; pseudocode ev_39206cb5c6974f179de1d5a62aeeb5f8812173d405b795dc7060764a9fbf2d0d
PUBLIC reconstructed_89
reconstructed_89 PROC FRAME
L_1800025c0:
    PUSH R14
    .pushreg R14
L_1800025c2:
    PUSH RSI
    .pushreg RSI
L_1800025c3:
    PUSH RDI
    .pushreg RDI
L_1800025c4:
    PUSH RBX
    .pushreg RBX
L_1800025c5:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800025c9:
    MOV R14, RCX
L_1800025cc:
    MOV RCX, qword ptr [OuterState+120]
L_1800025d3:
    TEST RCX, RCX
L_1800025d6:
    JZ L_18000260b
L_1800025d8:
    MOV RSI, R9
L_1800025db:
    MOV RDI, R8
L_1800025de:
    MOV RBX, RDX
L_1800025e1:
    LEA RDX, [name_89]
L_1800025e8:
    CALL qword ptr [__imp_GetProcAddress]
L_1800025ee:
    TEST RAX, RAX
L_1800025f1:
    JZ L_18000260b
L_1800025f3:
    MOV RCX, R14
L_1800025f6:
    MOV RDX, RBX
L_1800025f9:
    MOV R8, RDI
L_1800025fc:
    MOV R9, RSI
L_1800025ff:
    ADD RSP, 028h
L_180002603:
    POP RBX
L_180002604:
    POP RDI
L_180002605:
    POP RSI
L_180002606:
    POP R14
L_180002608:
    JMP RAX
L_18000260b:
    XOR EAX, EAX
L_18000260d:
    ADD RSP, 028h
L_180002611:
    POP RBX
L_180002612:
    POP RDI
L_180002613:
    POP RSI
L_180002614:
    POP R14
L_180002616:
    RET
reconstructed_89 ENDP
; midiOutGetID @ 90; original VA 0x180002617
; assembly ev_b2988feb1ab4af92ee973d77d319ec30ca1bcb64b9c61b227663e1710bc60be1; pseudocode ev_06f620ab463852426ac3e51522fb22f84d91ca5ccbcf5f9c7914123878391f71
PUBLIC reconstructed_90
reconstructed_90 PROC FRAME
L_180002617:
    PUSH R14
    .pushreg R14
L_180002619:
    PUSH RSI
    .pushreg RSI
L_18000261a:
    PUSH RDI
    .pushreg RDI
L_18000261b:
    PUSH RBX
    .pushreg RBX
L_18000261c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002620:
    MOV R14, RCX
L_180002623:
    MOV RCX, qword ptr [OuterState+120]
L_18000262a:
    TEST RCX, RCX
L_18000262d:
    JZ L_180002662
L_18000262f:
    MOV RSI, R9
L_180002632:
    MOV RDI, R8
L_180002635:
    MOV RBX, RDX
L_180002638:
    LEA RDX, [name_90]
L_18000263f:
    CALL qword ptr [__imp_GetProcAddress]
L_180002645:
    TEST RAX, RAX
L_180002648:
    JZ L_180002662
L_18000264a:
    MOV RCX, R14
L_18000264d:
    MOV RDX, RBX
L_180002650:
    MOV R8, RDI
L_180002653:
    MOV R9, RSI
L_180002656:
    ADD RSP, 028h
L_18000265a:
    POP RBX
L_18000265b:
    POP RDI
L_18000265c:
    POP RSI
L_18000265d:
    POP R14
L_18000265f:
    JMP RAX
L_180002662:
    XOR EAX, EAX
L_180002664:
    ADD RSP, 028h
L_180002668:
    POP RBX
L_180002669:
    POP RDI
L_18000266a:
    POP RSI
L_18000266b:
    POP R14
L_18000266d:
    RET
reconstructed_90 ENDP
; midiOutGetNumDevs @ 91; original VA 0x18000266e
; assembly ev_026ecae50072c2aeb5a5a823837affde6ac8b0b76b7469db61dfc1f318d04160; pseudocode ev_3312e75bb19122eade34a1e6abad8fd1d38c8cc637819092556233e4e5129482
PUBLIC reconstructed_91
reconstructed_91 PROC FRAME
L_18000266e:
    PUSH R14
    .pushreg R14
L_180002670:
    PUSH RSI
    .pushreg RSI
L_180002671:
    PUSH RDI
    .pushreg RDI
L_180002672:
    PUSH RBX
    .pushreg RBX
L_180002673:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002677:
    MOV R14, RCX
L_18000267a:
    MOV RCX, qword ptr [OuterState+120]
L_180002681:
    TEST RCX, RCX
L_180002684:
    JZ L_1800026b9
L_180002686:
    MOV RSI, R9
L_180002689:
    MOV RDI, R8
L_18000268c:
    MOV RBX, RDX
L_18000268f:
    LEA RDX, [name_91]
L_180002696:
    CALL qword ptr [__imp_GetProcAddress]
L_18000269c:
    TEST RAX, RAX
L_18000269f:
    JZ L_1800026b9
L_1800026a1:
    MOV RCX, R14
L_1800026a4:
    MOV RDX, RBX
L_1800026a7:
    MOV R8, RDI
L_1800026aa:
    MOV R9, RSI
L_1800026ad:
    ADD RSP, 028h
L_1800026b1:
    POP RBX
L_1800026b2:
    POP RDI
L_1800026b3:
    POP RSI
L_1800026b4:
    POP R14
L_1800026b6:
    JMP RAX
L_1800026b9:
    XOR EAX, EAX
L_1800026bb:
    ADD RSP, 028h
L_1800026bf:
    POP RBX
L_1800026c0:
    POP RDI
L_1800026c1:
    POP RSI
L_1800026c2:
    POP R14
L_1800026c4:
    RET
reconstructed_91 ENDP
; midiOutGetVolume @ 92; original VA 0x1800026c5
; assembly ev_d08f47bc9475d9ddcaaa95d4878d19e37b338274ab767376b8b92c6baf159a7f; pseudocode ev_5f3b6d6f27b415538b3506e9343db676346563a70ad95ae54484a7bb4ac46150
PUBLIC reconstructed_92
reconstructed_92 PROC FRAME
L_1800026c5:
    PUSH R14
    .pushreg R14
L_1800026c7:
    PUSH RSI
    .pushreg RSI
L_1800026c8:
    PUSH RDI
    .pushreg RDI
L_1800026c9:
    PUSH RBX
    .pushreg RBX
L_1800026ca:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800026ce:
    MOV R14, RCX
L_1800026d1:
    MOV RCX, qword ptr [OuterState+120]
L_1800026d8:
    TEST RCX, RCX
L_1800026db:
    JZ L_180002710
L_1800026dd:
    MOV RSI, R9
L_1800026e0:
    MOV RDI, R8
L_1800026e3:
    MOV RBX, RDX
L_1800026e6:
    LEA RDX, [name_92]
L_1800026ed:
    CALL qword ptr [__imp_GetProcAddress]
L_1800026f3:
    TEST RAX, RAX
L_1800026f6:
    JZ L_180002710
L_1800026f8:
    MOV RCX, R14
L_1800026fb:
    MOV RDX, RBX
L_1800026fe:
    MOV R8, RDI
L_180002701:
    MOV R9, RSI
L_180002704:
    ADD RSP, 028h
L_180002708:
    POP RBX
L_180002709:
    POP RDI
L_18000270a:
    POP RSI
L_18000270b:
    POP R14
L_18000270d:
    JMP RAX
L_180002710:
    XOR EAX, EAX
L_180002712:
    ADD RSP, 028h
L_180002716:
    POP RBX
L_180002717:
    POP RDI
L_180002718:
    POP RSI
L_180002719:
    POP R14
L_18000271b:
    RET
reconstructed_92 ENDP
; midiOutLongMsg @ 93; original VA 0x18000271c
; assembly ev_a985b5d0e404d381539e2b12dce6ea7b337a1da515f45c3cd36e00000d0034e8; pseudocode ev_b3b0e61fde855fe994b194bf093399612e66d58ca6bb9c558cf2b5ebb8f2ac5c
PUBLIC reconstructed_93
reconstructed_93 PROC FRAME
L_18000271c:
    PUSH R14
    .pushreg R14
L_18000271e:
    PUSH RSI
    .pushreg RSI
L_18000271f:
    PUSH RDI
    .pushreg RDI
L_180002720:
    PUSH RBX
    .pushreg RBX
L_180002721:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002725:
    MOV R14, RCX
L_180002728:
    MOV RCX, qword ptr [OuterState+120]
L_18000272f:
    TEST RCX, RCX
L_180002732:
    JZ L_180002767
L_180002734:
    MOV RSI, R9
L_180002737:
    MOV RDI, R8
L_18000273a:
    MOV RBX, RDX
L_18000273d:
    LEA RDX, [name_93]
L_180002744:
    CALL qword ptr [__imp_GetProcAddress]
L_18000274a:
    TEST RAX, RAX
L_18000274d:
    JZ L_180002767
L_18000274f:
    MOV RCX, R14
L_180002752:
    MOV RDX, RBX
L_180002755:
    MOV R8, RDI
L_180002758:
    MOV R9, RSI
L_18000275b:
    ADD RSP, 028h
L_18000275f:
    POP RBX
L_180002760:
    POP RDI
L_180002761:
    POP RSI
L_180002762:
    POP R14
L_180002764:
    JMP RAX
L_180002767:
    XOR EAX, EAX
L_180002769:
    ADD RSP, 028h
L_18000276d:
    POP RBX
L_18000276e:
    POP RDI
L_18000276f:
    POP RSI
L_180002770:
    POP R14
L_180002772:
    RET
reconstructed_93 ENDP
; midiOutMessage @ 94; original VA 0x180002773
; assembly ev_0d04cb82330754b3d0d1cfb76a97d523e4f3a3b29590de06efac5feb371aee8a; pseudocode ev_4a8bc7faf3e2d9b2fb4b2928b3c8c58889e8a6f76ea85ad7f681d65b352b71cb
PUBLIC reconstructed_94
reconstructed_94 PROC FRAME
L_180002773:
    PUSH R14
    .pushreg R14
L_180002775:
    PUSH RSI
    .pushreg RSI
L_180002776:
    PUSH RDI
    .pushreg RDI
L_180002777:
    PUSH RBX
    .pushreg RBX
L_180002778:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000277c:
    MOV R14, RCX
L_18000277f:
    MOV RCX, qword ptr [OuterState+120]
L_180002786:
    TEST RCX, RCX
L_180002789:
    JZ L_1800027be
L_18000278b:
    MOV RSI, R9
L_18000278e:
    MOV RDI, R8
L_180002791:
    MOV RBX, RDX
L_180002794:
    LEA RDX, [name_94]
L_18000279b:
    CALL qword ptr [__imp_GetProcAddress]
L_1800027a1:
    TEST RAX, RAX
L_1800027a4:
    JZ L_1800027be
L_1800027a6:
    MOV RCX, R14
L_1800027a9:
    MOV RDX, RBX
L_1800027ac:
    MOV R8, RDI
L_1800027af:
    MOV R9, RSI
L_1800027b2:
    ADD RSP, 028h
L_1800027b6:
    POP RBX
L_1800027b7:
    POP RDI
L_1800027b8:
    POP RSI
L_1800027b9:
    POP R14
L_1800027bb:
    JMP RAX
L_1800027be:
    XOR EAX, EAX
L_1800027c0:
    ADD RSP, 028h
L_1800027c4:
    POP RBX
L_1800027c5:
    POP RDI
L_1800027c6:
    POP RSI
L_1800027c7:
    POP R14
L_1800027c9:
    RET
reconstructed_94 ENDP
; midiOutOpen @ 95; original VA 0x1800027ca
; assembly ev_b8b5f692818a6ed9124226820c73ace6173a822508ae1fa47ad052091e97c9f5; pseudocode ev_3f2933b3cb5065f6b6d041ff4e3fba3c47ffe75ef5f3540eee80d1a17c8c65b0
PUBLIC reconstructed_95
reconstructed_95 PROC FRAME
L_1800027ca:
    PUSH R14
    .pushreg R14
L_1800027cc:
    PUSH RSI
    .pushreg RSI
L_1800027cd:
    PUSH RDI
    .pushreg RDI
L_1800027ce:
    PUSH RBX
    .pushreg RBX
L_1800027cf:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800027d3:
    MOV R14, RCX
L_1800027d6:
    MOV RCX, qword ptr [OuterState+120]
L_1800027dd:
    TEST RCX, RCX
L_1800027e0:
    JZ L_180002815
L_1800027e2:
    MOV RSI, R9
L_1800027e5:
    MOV RDI, R8
L_1800027e8:
    MOV RBX, RDX
L_1800027eb:
    LEA RDX, [name_95]
L_1800027f2:
    CALL qword ptr [__imp_GetProcAddress]
L_1800027f8:
    TEST RAX, RAX
L_1800027fb:
    JZ L_180002815
L_1800027fd:
    MOV RCX, R14
L_180002800:
    MOV RDX, RBX
L_180002803:
    MOV R8, RDI
L_180002806:
    MOV R9, RSI
L_180002809:
    ADD RSP, 028h
L_18000280d:
    POP RBX
L_18000280e:
    POP RDI
L_18000280f:
    POP RSI
L_180002810:
    POP R14
L_180002812:
    JMP RAX
L_180002815:
    XOR EAX, EAX
L_180002817:
    ADD RSP, 028h
L_18000281b:
    POP RBX
L_18000281c:
    POP RDI
L_18000281d:
    POP RSI
L_18000281e:
    POP R14
L_180002820:
    RET
reconstructed_95 ENDP
; midiOutPrepareHeader @ 96; original VA 0x180002821
; assembly ev_a357a313de2b94fb5315becb87d4fec2dfbfa57a61074d6c2800438dcd6c4ffc; pseudocode ev_9e9d62bce254aa553f93c80d6f7caed4a70abd4dd330fdd57faf7afc0adceb48
PUBLIC reconstructed_96
reconstructed_96 PROC FRAME
L_180002821:
    PUSH R14
    .pushreg R14
L_180002823:
    PUSH RSI
    .pushreg RSI
L_180002824:
    PUSH RDI
    .pushreg RDI
L_180002825:
    PUSH RBX
    .pushreg RBX
L_180002826:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000282a:
    MOV R14, RCX
L_18000282d:
    MOV RCX, qword ptr [OuterState+120]
L_180002834:
    TEST RCX, RCX
L_180002837:
    JZ L_18000286c
L_180002839:
    MOV RSI, R9
L_18000283c:
    MOV RDI, R8
L_18000283f:
    MOV RBX, RDX
L_180002842:
    LEA RDX, [name_96]
L_180002849:
    CALL qword ptr [__imp_GetProcAddress]
L_18000284f:
    TEST RAX, RAX
L_180002852:
    JZ L_18000286c
L_180002854:
    MOV RCX, R14
L_180002857:
    MOV RDX, RBX
L_18000285a:
    MOV R8, RDI
L_18000285d:
    MOV R9, RSI
L_180002860:
    ADD RSP, 028h
L_180002864:
    POP RBX
L_180002865:
    POP RDI
L_180002866:
    POP RSI
L_180002867:
    POP R14
L_180002869:
    JMP RAX
L_18000286c:
    XOR EAX, EAX
L_18000286e:
    ADD RSP, 028h
L_180002872:
    POP RBX
L_180002873:
    POP RDI
L_180002874:
    POP RSI
L_180002875:
    POP R14
L_180002877:
    RET
reconstructed_96 ENDP
; midiOutReset @ 97; original VA 0x180002878
; assembly ev_70f5c1ea22ba587fc3449c687365db9871759dd4d86092164a26375f7b04e1ed; pseudocode ev_04a4d62a9d94b9c383bb17eb479f64021c1c20f1910d3f8a0cd0b867711d1b77
PUBLIC reconstructed_97
reconstructed_97 PROC FRAME
L_180002878:
    PUSH R14
    .pushreg R14
L_18000287a:
    PUSH RSI
    .pushreg RSI
L_18000287b:
    PUSH RDI
    .pushreg RDI
L_18000287c:
    PUSH RBX
    .pushreg RBX
L_18000287d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002881:
    MOV R14, RCX
L_180002884:
    MOV RCX, qword ptr [OuterState+120]
L_18000288b:
    TEST RCX, RCX
L_18000288e:
    JZ L_1800028c3
L_180002890:
    MOV RSI, R9
L_180002893:
    MOV RDI, R8
L_180002896:
    MOV RBX, RDX
L_180002899:
    LEA RDX, [name_97]
L_1800028a0:
    CALL qword ptr [__imp_GetProcAddress]
L_1800028a6:
    TEST RAX, RAX
L_1800028a9:
    JZ L_1800028c3
L_1800028ab:
    MOV RCX, R14
L_1800028ae:
    MOV RDX, RBX
L_1800028b1:
    MOV R8, RDI
L_1800028b4:
    MOV R9, RSI
L_1800028b7:
    ADD RSP, 028h
L_1800028bb:
    POP RBX
L_1800028bc:
    POP RDI
L_1800028bd:
    POP RSI
L_1800028be:
    POP R14
L_1800028c0:
    JMP RAX
L_1800028c3:
    XOR EAX, EAX
L_1800028c5:
    ADD RSP, 028h
L_1800028c9:
    POP RBX
L_1800028ca:
    POP RDI
L_1800028cb:
    POP RSI
L_1800028cc:
    POP R14
L_1800028ce:
    RET
reconstructed_97 ENDP
; midiOutSetVolume @ 98; original VA 0x1800028cf
; assembly ev_48d100577adc7f25f60b1ebe5b0cb65bf75578102cdd02d984fc60b43cfa4abd; pseudocode ev_0db3c94f45343739b3cd523a1eb3380c77408cfd2f35ec95bb168a62af49dc7f
PUBLIC reconstructed_98
reconstructed_98 PROC FRAME
L_1800028cf:
    PUSH R14
    .pushreg R14
L_1800028d1:
    PUSH RSI
    .pushreg RSI
L_1800028d2:
    PUSH RDI
    .pushreg RDI
L_1800028d3:
    PUSH RBX
    .pushreg RBX
L_1800028d4:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800028d8:
    MOV R14, RCX
L_1800028db:
    MOV RCX, qword ptr [OuterState+120]
L_1800028e2:
    TEST RCX, RCX
L_1800028e5:
    JZ L_18000291a
L_1800028e7:
    MOV RSI, R9
L_1800028ea:
    MOV RDI, R8
L_1800028ed:
    MOV RBX, RDX
L_1800028f0:
    LEA RDX, [name_98]
L_1800028f7:
    CALL qword ptr [__imp_GetProcAddress]
L_1800028fd:
    TEST RAX, RAX
L_180002900:
    JZ L_18000291a
L_180002902:
    MOV RCX, R14
L_180002905:
    MOV RDX, RBX
L_180002908:
    MOV R8, RDI
L_18000290b:
    MOV R9, RSI
L_18000290e:
    ADD RSP, 028h
L_180002912:
    POP RBX
L_180002913:
    POP RDI
L_180002914:
    POP RSI
L_180002915:
    POP R14
L_180002917:
    JMP RAX
L_18000291a:
    XOR EAX, EAX
L_18000291c:
    ADD RSP, 028h
L_180002920:
    POP RBX
L_180002921:
    POP RDI
L_180002922:
    POP RSI
L_180002923:
    POP R14
L_180002925:
    RET
reconstructed_98 ENDP
; midiOutShortMsg @ 99; original VA 0x180002926
; assembly ev_870664a9d6386472b2fbc5bc68715bab751d63aec421cb58937302e6f82fb850; pseudocode ev_55e27cf769d0c9d7614d38e4891caa1cd91613f00de1f04681de143c2c8337d1
PUBLIC reconstructed_99
reconstructed_99 PROC FRAME
L_180002926:
    PUSH R14
    .pushreg R14
L_180002928:
    PUSH RSI
    .pushreg RSI
L_180002929:
    PUSH RDI
    .pushreg RDI
L_18000292a:
    PUSH RBX
    .pushreg RBX
L_18000292b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000292f:
    MOV R14, RCX
L_180002932:
    MOV RCX, qword ptr [OuterState+120]
L_180002939:
    TEST RCX, RCX
L_18000293c:
    JZ L_180002971
L_18000293e:
    MOV RSI, R9
L_180002941:
    MOV RDI, R8
L_180002944:
    MOV RBX, RDX
L_180002947:
    LEA RDX, [name_99]
L_18000294e:
    CALL qword ptr [__imp_GetProcAddress]
L_180002954:
    TEST RAX, RAX
L_180002957:
    JZ L_180002971
L_180002959:
    MOV RCX, R14
L_18000295c:
    MOV RDX, RBX
L_18000295f:
    MOV R8, RDI
L_180002962:
    MOV R9, RSI
L_180002965:
    ADD RSP, 028h
L_180002969:
    POP RBX
L_18000296a:
    POP RDI
L_18000296b:
    POP RSI
L_18000296c:
    POP R14
L_18000296e:
    JMP RAX
L_180002971:
    XOR EAX, EAX
L_180002973:
    ADD RSP, 028h
L_180002977:
    POP RBX
L_180002978:
    POP RDI
L_180002979:
    POP RSI
L_18000297a:
    POP R14
L_18000297c:
    RET
reconstructed_99 ENDP
; midiOutUnprepareHeader @ 100; original VA 0x18000297d
; assembly ev_40af6d0c70848e32eef2b86b19746e8322e93c2a84068bd4005ae52ae533a02c; pseudocode ev_f1de334adad51f2fcc21dc90d8bc692edd9fa5056b3e993030551697f12a83d4
PUBLIC reconstructed_100
reconstructed_100 PROC FRAME
L_18000297d:
    PUSH R14
    .pushreg R14
L_18000297f:
    PUSH RSI
    .pushreg RSI
L_180002980:
    PUSH RDI
    .pushreg RDI
L_180002981:
    PUSH RBX
    .pushreg RBX
L_180002982:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002986:
    MOV R14, RCX
L_180002989:
    MOV RCX, qword ptr [OuterState+120]
L_180002990:
    TEST RCX, RCX
L_180002993:
    JZ L_1800029c8
L_180002995:
    MOV RSI, R9
L_180002998:
    MOV RDI, R8
L_18000299b:
    MOV RBX, RDX
L_18000299e:
    LEA RDX, [name_100]
L_1800029a5:
    CALL qword ptr [__imp_GetProcAddress]
L_1800029ab:
    TEST RAX, RAX
L_1800029ae:
    JZ L_1800029c8
L_1800029b0:
    MOV RCX, R14
L_1800029b3:
    MOV RDX, RBX
L_1800029b6:
    MOV R8, RDI
L_1800029b9:
    MOV R9, RSI
L_1800029bc:
    ADD RSP, 028h
L_1800029c0:
    POP RBX
L_1800029c1:
    POP RDI
L_1800029c2:
    POP RSI
L_1800029c3:
    POP R14
L_1800029c5:
    JMP RAX
L_1800029c8:
    XOR EAX, EAX
L_1800029ca:
    ADD RSP, 028h
L_1800029ce:
    POP RBX
L_1800029cf:
    POP RDI
L_1800029d0:
    POP RSI
L_1800029d1:
    POP R14
L_1800029d3:
    RET
reconstructed_100 ENDP
; midiStreamClose @ 101; original VA 0x1800029d4
; assembly ev_e5e01e42185b96eaff4094f0acd226dc91aaef079a1a4f68481f6969bae4ba3f; pseudocode ev_60ec224e34ff5ba0a0cf8526a7f154d3661121e9a42c33043bbf7d758e3f50b2
PUBLIC reconstructed_101
reconstructed_101 PROC FRAME
L_1800029d4:
    PUSH R14
    .pushreg R14
L_1800029d6:
    PUSH RSI
    .pushreg RSI
L_1800029d7:
    PUSH RDI
    .pushreg RDI
L_1800029d8:
    PUSH RBX
    .pushreg RBX
L_1800029d9:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800029dd:
    MOV R14, RCX
L_1800029e0:
    MOV RCX, qword ptr [OuterState+120]
L_1800029e7:
    TEST RCX, RCX
L_1800029ea:
    JZ L_180002a1f
L_1800029ec:
    MOV RSI, R9
L_1800029ef:
    MOV RDI, R8
L_1800029f2:
    MOV RBX, RDX
L_1800029f5:
    LEA RDX, [name_101]
L_1800029fc:
    CALL qword ptr [__imp_GetProcAddress]
L_180002a02:
    TEST RAX, RAX
L_180002a05:
    JZ L_180002a1f
L_180002a07:
    MOV RCX, R14
L_180002a0a:
    MOV RDX, RBX
L_180002a0d:
    MOV R8, RDI
L_180002a10:
    MOV R9, RSI
L_180002a13:
    ADD RSP, 028h
L_180002a17:
    POP RBX
L_180002a18:
    POP RDI
L_180002a19:
    POP RSI
L_180002a1a:
    POP R14
L_180002a1c:
    JMP RAX
L_180002a1f:
    XOR EAX, EAX
L_180002a21:
    ADD RSP, 028h
L_180002a25:
    POP RBX
L_180002a26:
    POP RDI
L_180002a27:
    POP RSI
L_180002a28:
    POP R14
L_180002a2a:
    RET
reconstructed_101 ENDP
; midiStreamOpen @ 102; original VA 0x180002a2b
; assembly ev_3a7ee9ec493753cc2148e676bb6eb1cccf389ab16d908454d431cce2022b7b11; pseudocode ev_5729163f5f9ec8e8ade90a04dc1193e19d18746374742558d56770326b295b53
PUBLIC reconstructed_102
reconstructed_102 PROC FRAME
L_180002a2b:
    PUSH R14
    .pushreg R14
L_180002a2d:
    PUSH RSI
    .pushreg RSI
L_180002a2e:
    PUSH RDI
    .pushreg RDI
L_180002a2f:
    PUSH RBX
    .pushreg RBX
L_180002a30:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002a34:
    MOV R14, RCX
L_180002a37:
    MOV RCX, qword ptr [OuterState+120]
L_180002a3e:
    TEST RCX, RCX
L_180002a41:
    JZ L_180002a76
L_180002a43:
    MOV RSI, R9
L_180002a46:
    MOV RDI, R8
L_180002a49:
    MOV RBX, RDX
L_180002a4c:
    LEA RDX, [name_102]
L_180002a53:
    CALL qword ptr [__imp_GetProcAddress]
L_180002a59:
    TEST RAX, RAX
L_180002a5c:
    JZ L_180002a76
L_180002a5e:
    MOV RCX, R14
L_180002a61:
    MOV RDX, RBX
L_180002a64:
    MOV R8, RDI
L_180002a67:
    MOV R9, RSI
L_180002a6a:
    ADD RSP, 028h
L_180002a6e:
    POP RBX
L_180002a6f:
    POP RDI
L_180002a70:
    POP RSI
L_180002a71:
    POP R14
L_180002a73:
    JMP RAX
L_180002a76:
    XOR EAX, EAX
L_180002a78:
    ADD RSP, 028h
L_180002a7c:
    POP RBX
L_180002a7d:
    POP RDI
L_180002a7e:
    POP RSI
L_180002a7f:
    POP R14
L_180002a81:
    RET
reconstructed_102 ENDP
; midiStreamOut @ 103; original VA 0x180002a82
; assembly ev_668d6096aa74ed1b9179d2077a4d3357f8f2c29ae471885ad86f3b45e261fc2c; pseudocode ev_e03f1ec59032bb0e1bfc887935a80ce615a3c92022a58adb630ce10cac50c42d
PUBLIC reconstructed_103
reconstructed_103 PROC FRAME
L_180002a82:
    PUSH R14
    .pushreg R14
L_180002a84:
    PUSH RSI
    .pushreg RSI
L_180002a85:
    PUSH RDI
    .pushreg RDI
L_180002a86:
    PUSH RBX
    .pushreg RBX
L_180002a87:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002a8b:
    MOV R14, RCX
L_180002a8e:
    MOV RCX, qword ptr [OuterState+120]
L_180002a95:
    TEST RCX, RCX
L_180002a98:
    JZ L_180002acd
L_180002a9a:
    MOV RSI, R9
L_180002a9d:
    MOV RDI, R8
L_180002aa0:
    MOV RBX, RDX
L_180002aa3:
    LEA RDX, [name_103]
L_180002aaa:
    CALL qword ptr [__imp_GetProcAddress]
L_180002ab0:
    TEST RAX, RAX
L_180002ab3:
    JZ L_180002acd
L_180002ab5:
    MOV RCX, R14
L_180002ab8:
    MOV RDX, RBX
L_180002abb:
    MOV R8, RDI
L_180002abe:
    MOV R9, RSI
L_180002ac1:
    ADD RSP, 028h
L_180002ac5:
    POP RBX
L_180002ac6:
    POP RDI
L_180002ac7:
    POP RSI
L_180002ac8:
    POP R14
L_180002aca:
    JMP RAX
L_180002acd:
    XOR EAX, EAX
L_180002acf:
    ADD RSP, 028h
L_180002ad3:
    POP RBX
L_180002ad4:
    POP RDI
L_180002ad5:
    POP RSI
L_180002ad6:
    POP R14
L_180002ad8:
    RET
reconstructed_103 ENDP
; midiStreamPause @ 104; original VA 0x180002ad9
; assembly ev_c4540600f694bb5844168ced53b06a58742f5f61a61e86b44c49ad0b76bb251f; pseudocode ev_18a7bb8f0e83a1e4454c0ef4522eaa787c9aa56a7153fc06f33036e75da8a06d
PUBLIC reconstructed_104
reconstructed_104 PROC FRAME
L_180002ad9:
    PUSH R14
    .pushreg R14
L_180002adb:
    PUSH RSI
    .pushreg RSI
L_180002adc:
    PUSH RDI
    .pushreg RDI
L_180002add:
    PUSH RBX
    .pushreg RBX
L_180002ade:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002ae2:
    MOV R14, RCX
L_180002ae5:
    MOV RCX, qword ptr [OuterState+120]
L_180002aec:
    TEST RCX, RCX
L_180002aef:
    JZ L_180002b24
L_180002af1:
    MOV RSI, R9
L_180002af4:
    MOV RDI, R8
L_180002af7:
    MOV RBX, RDX
L_180002afa:
    LEA RDX, [name_104]
L_180002b01:
    CALL qword ptr [__imp_GetProcAddress]
L_180002b07:
    TEST RAX, RAX
L_180002b0a:
    JZ L_180002b24
L_180002b0c:
    MOV RCX, R14
L_180002b0f:
    MOV RDX, RBX
L_180002b12:
    MOV R8, RDI
L_180002b15:
    MOV R9, RSI
L_180002b18:
    ADD RSP, 028h
L_180002b1c:
    POP RBX
L_180002b1d:
    POP RDI
L_180002b1e:
    POP RSI
L_180002b1f:
    POP R14
L_180002b21:
    JMP RAX
L_180002b24:
    XOR EAX, EAX
L_180002b26:
    ADD RSP, 028h
L_180002b2a:
    POP RBX
L_180002b2b:
    POP RDI
L_180002b2c:
    POP RSI
L_180002b2d:
    POP R14
L_180002b2f:
    RET
reconstructed_104 ENDP
; midiStreamPosition @ 105; original VA 0x180002b30
; assembly ev_6e27b53ee956a4b986adf1d2d1dc86a043335372ea0a35617b6164b02a92ec92; pseudocode ev_a8d5141d710b57ff7efb93e29c81829ea0b4701a27aaf0c3586ee07628f16bf8
PUBLIC reconstructed_105
reconstructed_105 PROC FRAME
L_180002b30:
    PUSH R14
    .pushreg R14
L_180002b32:
    PUSH RSI
    .pushreg RSI
L_180002b33:
    PUSH RDI
    .pushreg RDI
L_180002b34:
    PUSH RBX
    .pushreg RBX
L_180002b35:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002b39:
    MOV R14, RCX
L_180002b3c:
    MOV RCX, qword ptr [OuterState+120]
L_180002b43:
    TEST RCX, RCX
L_180002b46:
    JZ L_180002b7b
L_180002b48:
    MOV RSI, R9
L_180002b4b:
    MOV RDI, R8
L_180002b4e:
    MOV RBX, RDX
L_180002b51:
    LEA RDX, [name_105]
L_180002b58:
    CALL qword ptr [__imp_GetProcAddress]
L_180002b5e:
    TEST RAX, RAX
L_180002b61:
    JZ L_180002b7b
L_180002b63:
    MOV RCX, R14
L_180002b66:
    MOV RDX, RBX
L_180002b69:
    MOV R8, RDI
L_180002b6c:
    MOV R9, RSI
L_180002b6f:
    ADD RSP, 028h
L_180002b73:
    POP RBX
L_180002b74:
    POP RDI
L_180002b75:
    POP RSI
L_180002b76:
    POP R14
L_180002b78:
    JMP RAX
L_180002b7b:
    XOR EAX, EAX
L_180002b7d:
    ADD RSP, 028h
L_180002b81:
    POP RBX
L_180002b82:
    POP RDI
L_180002b83:
    POP RSI
L_180002b84:
    POP R14
L_180002b86:
    RET
reconstructed_105 ENDP
; midiStreamProperty @ 106; original VA 0x180002b87
; assembly ev_2417d4831910bfc89b74c9cb61a71d789c9845bcf164084cd9a41491aa2e83ca; pseudocode ev_644d4ab16165b14f51395c8f2f0497d6d203d39a2c52887e533e24ec39aead7c
PUBLIC reconstructed_106
reconstructed_106 PROC FRAME
L_180002b87:
    PUSH R14
    .pushreg R14
L_180002b89:
    PUSH RSI
    .pushreg RSI
L_180002b8a:
    PUSH RDI
    .pushreg RDI
L_180002b8b:
    PUSH RBX
    .pushreg RBX
L_180002b8c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002b90:
    MOV R14, RCX
L_180002b93:
    MOV RCX, qword ptr [OuterState+120]
L_180002b9a:
    TEST RCX, RCX
L_180002b9d:
    JZ L_180002bd2
L_180002b9f:
    MOV RSI, R9
L_180002ba2:
    MOV RDI, R8
L_180002ba5:
    MOV RBX, RDX
L_180002ba8:
    LEA RDX, [name_106]
L_180002baf:
    CALL qword ptr [__imp_GetProcAddress]
L_180002bb5:
    TEST RAX, RAX
L_180002bb8:
    JZ L_180002bd2
L_180002bba:
    MOV RCX, R14
L_180002bbd:
    MOV RDX, RBX
L_180002bc0:
    MOV R8, RDI
L_180002bc3:
    MOV R9, RSI
L_180002bc6:
    ADD RSP, 028h
L_180002bca:
    POP RBX
L_180002bcb:
    POP RDI
L_180002bcc:
    POP RSI
L_180002bcd:
    POP R14
L_180002bcf:
    JMP RAX
L_180002bd2:
    XOR EAX, EAX
L_180002bd4:
    ADD RSP, 028h
L_180002bd8:
    POP RBX
L_180002bd9:
    POP RDI
L_180002bda:
    POP RSI
L_180002bdb:
    POP R14
L_180002bdd:
    RET
reconstructed_106 ENDP
; midiStreamRestart @ 107; original VA 0x180002bde
; assembly ev_8b38d2471445bd68f241302e9bf0abdc5d0f8dc3a6d2711c84cf1f62143e1508; pseudocode ev_6c383e918db067dcdaafeb718759222516e89f0937aab61483353842c99d095b
PUBLIC reconstructed_107
reconstructed_107 PROC FRAME
L_180002bde:
    PUSH R14
    .pushreg R14
L_180002be0:
    PUSH RSI
    .pushreg RSI
L_180002be1:
    PUSH RDI
    .pushreg RDI
L_180002be2:
    PUSH RBX
    .pushreg RBX
L_180002be3:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002be7:
    MOV R14, RCX
L_180002bea:
    MOV RCX, qword ptr [OuterState+120]
L_180002bf1:
    TEST RCX, RCX
L_180002bf4:
    JZ L_180002c29
L_180002bf6:
    MOV RSI, R9
L_180002bf9:
    MOV RDI, R8
L_180002bfc:
    MOV RBX, RDX
L_180002bff:
    LEA RDX, [name_107]
L_180002c06:
    CALL qword ptr [__imp_GetProcAddress]
L_180002c0c:
    TEST RAX, RAX
L_180002c0f:
    JZ L_180002c29
L_180002c11:
    MOV RCX, R14
L_180002c14:
    MOV RDX, RBX
L_180002c17:
    MOV R8, RDI
L_180002c1a:
    MOV R9, RSI
L_180002c1d:
    ADD RSP, 028h
L_180002c21:
    POP RBX
L_180002c22:
    POP RDI
L_180002c23:
    POP RSI
L_180002c24:
    POP R14
L_180002c26:
    JMP RAX
L_180002c29:
    XOR EAX, EAX
L_180002c2b:
    ADD RSP, 028h
L_180002c2f:
    POP RBX
L_180002c30:
    POP RDI
L_180002c31:
    POP RSI
L_180002c32:
    POP R14
L_180002c34:
    RET
reconstructed_107 ENDP
; midiStreamStop @ 108; original VA 0x180002c35
; assembly ev_778af688bb4ee78cd3068aec3d874f8749b13885b816b16a505d100fcfb41ea1; pseudocode ev_429524af90df5a2e217f186b3ebad99fba10a80d4a9dd0cbf9c61037b1aa371f
PUBLIC reconstructed_108
reconstructed_108 PROC FRAME
L_180002c35:
    PUSH R14
    .pushreg R14
L_180002c37:
    PUSH RSI
    .pushreg RSI
L_180002c38:
    PUSH RDI
    .pushreg RDI
L_180002c39:
    PUSH RBX
    .pushreg RBX
L_180002c3a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002c3e:
    MOV R14, RCX
L_180002c41:
    MOV RCX, qword ptr [OuterState+120]
L_180002c48:
    TEST RCX, RCX
L_180002c4b:
    JZ L_180002c80
L_180002c4d:
    MOV RSI, R9
L_180002c50:
    MOV RDI, R8
L_180002c53:
    MOV RBX, RDX
L_180002c56:
    LEA RDX, [name_108]
L_180002c5d:
    CALL qword ptr [__imp_GetProcAddress]
L_180002c63:
    TEST RAX, RAX
L_180002c66:
    JZ L_180002c80
L_180002c68:
    MOV RCX, R14
L_180002c6b:
    MOV RDX, RBX
L_180002c6e:
    MOV R8, RDI
L_180002c71:
    MOV R9, RSI
L_180002c74:
    ADD RSP, 028h
L_180002c78:
    POP RBX
L_180002c79:
    POP RDI
L_180002c7a:
    POP RSI
L_180002c7b:
    POP R14
L_180002c7d:
    JMP RAX
L_180002c80:
    XOR EAX, EAX
L_180002c82:
    ADD RSP, 028h
L_180002c86:
    POP RBX
L_180002c87:
    POP RDI
L_180002c88:
    POP RSI
L_180002c89:
    POP R14
L_180002c8b:
    RET
reconstructed_108 ENDP
; mixerClose @ 109; original VA 0x180002c8c
; assembly ev_305cd40072f2152bb12d42e6be6500afff6523fe2aa48968193d565018595880; pseudocode ev_ae7ab0454557221d27d2895c3c44105af29392912b9941a2b7dc7392ee14c86b
PUBLIC reconstructed_109
reconstructed_109 PROC FRAME
L_180002c8c:
    PUSH R14
    .pushreg R14
L_180002c8e:
    PUSH RSI
    .pushreg RSI
L_180002c8f:
    PUSH RDI
    .pushreg RDI
L_180002c90:
    PUSH RBX
    .pushreg RBX
L_180002c91:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002c95:
    MOV R14, RCX
L_180002c98:
    MOV RCX, qword ptr [OuterState+120]
L_180002c9f:
    TEST RCX, RCX
L_180002ca2:
    JZ L_180002cd7
L_180002ca4:
    MOV RSI, R9
L_180002ca7:
    MOV RDI, R8
L_180002caa:
    MOV RBX, RDX
L_180002cad:
    LEA RDX, [name_109]
L_180002cb4:
    CALL qword ptr [__imp_GetProcAddress]
L_180002cba:
    TEST RAX, RAX
L_180002cbd:
    JZ L_180002cd7
L_180002cbf:
    MOV RCX, R14
L_180002cc2:
    MOV RDX, RBX
L_180002cc5:
    MOV R8, RDI
L_180002cc8:
    MOV R9, RSI
L_180002ccb:
    ADD RSP, 028h
L_180002ccf:
    POP RBX
L_180002cd0:
    POP RDI
L_180002cd1:
    POP RSI
L_180002cd2:
    POP R14
L_180002cd4:
    JMP RAX
L_180002cd7:
    XOR EAX, EAX
L_180002cd9:
    ADD RSP, 028h
L_180002cdd:
    POP RBX
L_180002cde:
    POP RDI
L_180002cdf:
    POP RSI
L_180002ce0:
    POP R14
L_180002ce2:
    RET
reconstructed_109 ENDP
; mixerGetControlDetailsA @ 110; original VA 0x180002ce3
; assembly ev_fa4c05edc77508c95cec8ce4fdaa9dd2cab7a388eca417dbc0299ce1cd82466f; pseudocode ev_b61a1e7fbb20682804dd8a880526a20119a0bf76d608f0ce6ed716121b5e240b
PUBLIC reconstructed_110
reconstructed_110 PROC FRAME
L_180002ce3:
    PUSH R14
    .pushreg R14
L_180002ce5:
    PUSH RSI
    .pushreg RSI
L_180002ce6:
    PUSH RDI
    .pushreg RDI
L_180002ce7:
    PUSH RBX
    .pushreg RBX
L_180002ce8:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002cec:
    MOV R14, RCX
L_180002cef:
    MOV RCX, qword ptr [OuterState+120]
L_180002cf6:
    TEST RCX, RCX
L_180002cf9:
    JZ L_180002d2e
L_180002cfb:
    MOV RSI, R9
L_180002cfe:
    MOV RDI, R8
L_180002d01:
    MOV RBX, RDX
L_180002d04:
    LEA RDX, [name_110]
L_180002d0b:
    CALL qword ptr [__imp_GetProcAddress]
L_180002d11:
    TEST RAX, RAX
L_180002d14:
    JZ L_180002d2e
L_180002d16:
    MOV RCX, R14
L_180002d19:
    MOV RDX, RBX
L_180002d1c:
    MOV R8, RDI
L_180002d1f:
    MOV R9, RSI
L_180002d22:
    ADD RSP, 028h
L_180002d26:
    POP RBX
L_180002d27:
    POP RDI
L_180002d28:
    POP RSI
L_180002d29:
    POP R14
L_180002d2b:
    JMP RAX
L_180002d2e:
    XOR EAX, EAX
L_180002d30:
    ADD RSP, 028h
L_180002d34:
    POP RBX
L_180002d35:
    POP RDI
L_180002d36:
    POP RSI
L_180002d37:
    POP R14
L_180002d39:
    RET
reconstructed_110 ENDP
; mixerGetControlDetailsW @ 111; original VA 0x180002d3a
; assembly ev_9381ef2ddc0d1547bccaafd25138d25edcbd6b65dbdc584cf320dd75abe80d0b; pseudocode ev_14d0014c91ca2a83ffedb9a553bebeaac43ed5a1810fbb1eb5429af5648c9832
PUBLIC reconstructed_111
reconstructed_111 PROC FRAME
L_180002d3a:
    PUSH R14
    .pushreg R14
L_180002d3c:
    PUSH RSI
    .pushreg RSI
L_180002d3d:
    PUSH RDI
    .pushreg RDI
L_180002d3e:
    PUSH RBX
    .pushreg RBX
L_180002d3f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002d43:
    MOV R14, RCX
L_180002d46:
    MOV RCX, qword ptr [OuterState+120]
L_180002d4d:
    TEST RCX, RCX
L_180002d50:
    JZ L_180002d85
L_180002d52:
    MOV RSI, R9
L_180002d55:
    MOV RDI, R8
L_180002d58:
    MOV RBX, RDX
L_180002d5b:
    LEA RDX, [name_111]
L_180002d62:
    CALL qword ptr [__imp_GetProcAddress]
L_180002d68:
    TEST RAX, RAX
L_180002d6b:
    JZ L_180002d85
L_180002d6d:
    MOV RCX, R14
L_180002d70:
    MOV RDX, RBX
L_180002d73:
    MOV R8, RDI
L_180002d76:
    MOV R9, RSI
L_180002d79:
    ADD RSP, 028h
L_180002d7d:
    POP RBX
L_180002d7e:
    POP RDI
L_180002d7f:
    POP RSI
L_180002d80:
    POP R14
L_180002d82:
    JMP RAX
L_180002d85:
    XOR EAX, EAX
L_180002d87:
    ADD RSP, 028h
L_180002d8b:
    POP RBX
L_180002d8c:
    POP RDI
L_180002d8d:
    POP RSI
L_180002d8e:
    POP R14
L_180002d90:
    RET
reconstructed_111 ENDP
; mixerGetDevCapsA @ 112; original VA 0x180002d91
; assembly ev_2cc7b9f1cbc90e9356430867e4d3ae040ffcecca58c34d1428bbc77415512c18; pseudocode ev_1d3fb7c5efe094cdb3f03c35ce0962cc1698ad9648e2103cb439b085a12b1bfe
PUBLIC reconstructed_112
reconstructed_112 PROC FRAME
L_180002d91:
    PUSH R14
    .pushreg R14
L_180002d93:
    PUSH RSI
    .pushreg RSI
L_180002d94:
    PUSH RDI
    .pushreg RDI
L_180002d95:
    PUSH RBX
    .pushreg RBX
L_180002d96:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002d9a:
    MOV R14, RCX
L_180002d9d:
    MOV RCX, qword ptr [OuterState+120]
L_180002da4:
    TEST RCX, RCX
L_180002da7:
    JZ L_180002ddc
L_180002da9:
    MOV RSI, R9
L_180002dac:
    MOV RDI, R8
L_180002daf:
    MOV RBX, RDX
L_180002db2:
    LEA RDX, [name_112]
L_180002db9:
    CALL qword ptr [__imp_GetProcAddress]
L_180002dbf:
    TEST RAX, RAX
L_180002dc2:
    JZ L_180002ddc
L_180002dc4:
    MOV RCX, R14
L_180002dc7:
    MOV RDX, RBX
L_180002dca:
    MOV R8, RDI
L_180002dcd:
    MOV R9, RSI
L_180002dd0:
    ADD RSP, 028h
L_180002dd4:
    POP RBX
L_180002dd5:
    POP RDI
L_180002dd6:
    POP RSI
L_180002dd7:
    POP R14
L_180002dd9:
    JMP RAX
L_180002ddc:
    XOR EAX, EAX
L_180002dde:
    ADD RSP, 028h
L_180002de2:
    POP RBX
L_180002de3:
    POP RDI
L_180002de4:
    POP RSI
L_180002de5:
    POP R14
L_180002de7:
    RET
reconstructed_112 ENDP
; mixerGetDevCapsW @ 113; original VA 0x180002de8
; assembly ev_2368b78a5b9f903960ec44bbfaa5062448fefd8d88b44a9c42de2c6e8b3f03bc; pseudocode ev_5914752db3d6d37e03e8037a81554e3031d78eaf82679df3554b7e90b23da38c
PUBLIC reconstructed_113
reconstructed_113 PROC FRAME
L_180002de8:
    PUSH R14
    .pushreg R14
L_180002dea:
    PUSH RSI
    .pushreg RSI
L_180002deb:
    PUSH RDI
    .pushreg RDI
L_180002dec:
    PUSH RBX
    .pushreg RBX
L_180002ded:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002df1:
    MOV R14, RCX
L_180002df4:
    MOV RCX, qword ptr [OuterState+120]
L_180002dfb:
    TEST RCX, RCX
L_180002dfe:
    JZ L_180002e33
L_180002e00:
    MOV RSI, R9
L_180002e03:
    MOV RDI, R8
L_180002e06:
    MOV RBX, RDX
L_180002e09:
    LEA RDX, [name_113]
L_180002e10:
    CALL qword ptr [__imp_GetProcAddress]
L_180002e16:
    TEST RAX, RAX
L_180002e19:
    JZ L_180002e33
L_180002e1b:
    MOV RCX, R14
L_180002e1e:
    MOV RDX, RBX
L_180002e21:
    MOV R8, RDI
L_180002e24:
    MOV R9, RSI
L_180002e27:
    ADD RSP, 028h
L_180002e2b:
    POP RBX
L_180002e2c:
    POP RDI
L_180002e2d:
    POP RSI
L_180002e2e:
    POP R14
L_180002e30:
    JMP RAX
L_180002e33:
    XOR EAX, EAX
L_180002e35:
    ADD RSP, 028h
L_180002e39:
    POP RBX
L_180002e3a:
    POP RDI
L_180002e3b:
    POP RSI
L_180002e3c:
    POP R14
L_180002e3e:
    RET
reconstructed_113 ENDP
; mixerGetID @ 114; original VA 0x180002e3f
; assembly ev_180943c65f4a2197b27ad4833f682b344a86cc52beb3fc718c765618bbe90f9f; pseudocode ev_28416ac1f91de430c07a307b905030731b1cc301b44046fd8fccb207fc2a8bfa
PUBLIC reconstructed_114
reconstructed_114 PROC FRAME
L_180002e3f:
    PUSH R14
    .pushreg R14
L_180002e41:
    PUSH RSI
    .pushreg RSI
L_180002e42:
    PUSH RDI
    .pushreg RDI
L_180002e43:
    PUSH RBX
    .pushreg RBX
L_180002e44:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002e48:
    MOV R14, RCX
L_180002e4b:
    MOV RCX, qword ptr [OuterState+120]
L_180002e52:
    TEST RCX, RCX
L_180002e55:
    JZ L_180002e8a
L_180002e57:
    MOV RSI, R9
L_180002e5a:
    MOV RDI, R8
L_180002e5d:
    MOV RBX, RDX
L_180002e60:
    LEA RDX, [name_114]
L_180002e67:
    CALL qword ptr [__imp_GetProcAddress]
L_180002e6d:
    TEST RAX, RAX
L_180002e70:
    JZ L_180002e8a
L_180002e72:
    MOV RCX, R14
L_180002e75:
    MOV RDX, RBX
L_180002e78:
    MOV R8, RDI
L_180002e7b:
    MOV R9, RSI
L_180002e7e:
    ADD RSP, 028h
L_180002e82:
    POP RBX
L_180002e83:
    POP RDI
L_180002e84:
    POP RSI
L_180002e85:
    POP R14
L_180002e87:
    JMP RAX
L_180002e8a:
    XOR EAX, EAX
L_180002e8c:
    ADD RSP, 028h
L_180002e90:
    POP RBX
L_180002e91:
    POP RDI
L_180002e92:
    POP RSI
L_180002e93:
    POP R14
L_180002e95:
    RET
reconstructed_114 ENDP
; mixerGetLineControlsA @ 115; original VA 0x180002e96
; assembly ev_b3d4845c4b2131d96c064ae9e126a3b0d5f2b5c28c38a15ca968947765399873; pseudocode ev_773f96b95d63a0c03da859ed9fdbcee2d8e66a56524209ba0c5eb7a8f73e2143
PUBLIC reconstructed_115
reconstructed_115 PROC FRAME
L_180002e96:
    PUSH R14
    .pushreg R14
L_180002e98:
    PUSH RSI
    .pushreg RSI
L_180002e99:
    PUSH RDI
    .pushreg RDI
L_180002e9a:
    PUSH RBX
    .pushreg RBX
L_180002e9b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002e9f:
    MOV R14, RCX
L_180002ea2:
    MOV RCX, qword ptr [OuterState+120]
L_180002ea9:
    TEST RCX, RCX
L_180002eac:
    JZ L_180002ee1
L_180002eae:
    MOV RSI, R9
L_180002eb1:
    MOV RDI, R8
L_180002eb4:
    MOV RBX, RDX
L_180002eb7:
    LEA RDX, [name_115]
L_180002ebe:
    CALL qword ptr [__imp_GetProcAddress]
L_180002ec4:
    TEST RAX, RAX
L_180002ec7:
    JZ L_180002ee1
L_180002ec9:
    MOV RCX, R14
L_180002ecc:
    MOV RDX, RBX
L_180002ecf:
    MOV R8, RDI
L_180002ed2:
    MOV R9, RSI
L_180002ed5:
    ADD RSP, 028h
L_180002ed9:
    POP RBX
L_180002eda:
    POP RDI
L_180002edb:
    POP RSI
L_180002edc:
    POP R14
L_180002ede:
    JMP RAX
L_180002ee1:
    XOR EAX, EAX
L_180002ee3:
    ADD RSP, 028h
L_180002ee7:
    POP RBX
L_180002ee8:
    POP RDI
L_180002ee9:
    POP RSI
L_180002eea:
    POP R14
L_180002eec:
    RET
reconstructed_115 ENDP
; mixerGetLineControlsW @ 116; original VA 0x180002eed
; assembly ev_009bb21c7824748ec990f207c64d371dc372d3a5501246f5001a9dc17dc341c8; pseudocode ev_aa870feed068d5f436e5c09bfd31b624f0f31cbe4ea0196348ca03385619562a
PUBLIC reconstructed_116
reconstructed_116 PROC FRAME
L_180002eed:
    PUSH R14
    .pushreg R14
L_180002eef:
    PUSH RSI
    .pushreg RSI
L_180002ef0:
    PUSH RDI
    .pushreg RDI
L_180002ef1:
    PUSH RBX
    .pushreg RBX
L_180002ef2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002ef6:
    MOV R14, RCX
L_180002ef9:
    MOV RCX, qword ptr [OuterState+120]
L_180002f00:
    TEST RCX, RCX
L_180002f03:
    JZ L_180002f38
L_180002f05:
    MOV RSI, R9
L_180002f08:
    MOV RDI, R8
L_180002f0b:
    MOV RBX, RDX
L_180002f0e:
    LEA RDX, [name_116]
L_180002f15:
    CALL qword ptr [__imp_GetProcAddress]
L_180002f1b:
    TEST RAX, RAX
L_180002f1e:
    JZ L_180002f38
L_180002f20:
    MOV RCX, R14
L_180002f23:
    MOV RDX, RBX
L_180002f26:
    MOV R8, RDI
L_180002f29:
    MOV R9, RSI
L_180002f2c:
    ADD RSP, 028h
L_180002f30:
    POP RBX
L_180002f31:
    POP RDI
L_180002f32:
    POP RSI
L_180002f33:
    POP R14
L_180002f35:
    JMP RAX
L_180002f38:
    XOR EAX, EAX
L_180002f3a:
    ADD RSP, 028h
L_180002f3e:
    POP RBX
L_180002f3f:
    POP RDI
L_180002f40:
    POP RSI
L_180002f41:
    POP R14
L_180002f43:
    RET
reconstructed_116 ENDP
; mixerGetLineInfoA @ 117; original VA 0x180002f44
; assembly ev_6036e9c28436b456608c8d15f72c615f04e83770204f4ad95a273cc34d89d920; pseudocode ev_7c26b20a478be1b9aac17b77d9c70166a699b97a36cb51dce008156f4641d654
PUBLIC reconstructed_117
reconstructed_117 PROC FRAME
L_180002f44:
    PUSH R14
    .pushreg R14
L_180002f46:
    PUSH RSI
    .pushreg RSI
L_180002f47:
    PUSH RDI
    .pushreg RDI
L_180002f48:
    PUSH RBX
    .pushreg RBX
L_180002f49:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002f4d:
    MOV R14, RCX
L_180002f50:
    MOV RCX, qword ptr [OuterState+120]
L_180002f57:
    TEST RCX, RCX
L_180002f5a:
    JZ L_180002f8f
L_180002f5c:
    MOV RSI, R9
L_180002f5f:
    MOV RDI, R8
L_180002f62:
    MOV RBX, RDX
L_180002f65:
    LEA RDX, [name_117]
L_180002f6c:
    CALL qword ptr [__imp_GetProcAddress]
L_180002f72:
    TEST RAX, RAX
L_180002f75:
    JZ L_180002f8f
L_180002f77:
    MOV RCX, R14
L_180002f7a:
    MOV RDX, RBX
L_180002f7d:
    MOV R8, RDI
L_180002f80:
    MOV R9, RSI
L_180002f83:
    ADD RSP, 028h
L_180002f87:
    POP RBX
L_180002f88:
    POP RDI
L_180002f89:
    POP RSI
L_180002f8a:
    POP R14
L_180002f8c:
    JMP RAX
L_180002f8f:
    XOR EAX, EAX
L_180002f91:
    ADD RSP, 028h
L_180002f95:
    POP RBX
L_180002f96:
    POP RDI
L_180002f97:
    POP RSI
L_180002f98:
    POP R14
L_180002f9a:
    RET
reconstructed_117 ENDP
; mixerGetLineInfoW @ 118; original VA 0x180002f9b
; assembly ev_b4e6a5d1e4d03403e8199913e75ab3853ae541963e08f508f9fcfacb613a18a1; pseudocode ev_3c5f976d29f06072dc732ed839ed400321aaa724d5bd95719c85eb13beeade55
PUBLIC reconstructed_118
reconstructed_118 PROC FRAME
L_180002f9b:
    PUSH R14
    .pushreg R14
L_180002f9d:
    PUSH RSI
    .pushreg RSI
L_180002f9e:
    PUSH RDI
    .pushreg RDI
L_180002f9f:
    PUSH RBX
    .pushreg RBX
L_180002fa0:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002fa4:
    MOV R14, RCX
L_180002fa7:
    MOV RCX, qword ptr [OuterState+120]
L_180002fae:
    TEST RCX, RCX
L_180002fb1:
    JZ L_180002fe6
L_180002fb3:
    MOV RSI, R9
L_180002fb6:
    MOV RDI, R8
L_180002fb9:
    MOV RBX, RDX
L_180002fbc:
    LEA RDX, [name_118]
L_180002fc3:
    CALL qword ptr [__imp_GetProcAddress]
L_180002fc9:
    TEST RAX, RAX
L_180002fcc:
    JZ L_180002fe6
L_180002fce:
    MOV RCX, R14
L_180002fd1:
    MOV RDX, RBX
L_180002fd4:
    MOV R8, RDI
L_180002fd7:
    MOV R9, RSI
L_180002fda:
    ADD RSP, 028h
L_180002fde:
    POP RBX
L_180002fdf:
    POP RDI
L_180002fe0:
    POP RSI
L_180002fe1:
    POP R14
L_180002fe3:
    JMP RAX
L_180002fe6:
    XOR EAX, EAX
L_180002fe8:
    ADD RSP, 028h
L_180002fec:
    POP RBX
L_180002fed:
    POP RDI
L_180002fee:
    POP RSI
L_180002fef:
    POP R14
L_180002ff1:
    RET
reconstructed_118 ENDP
; mixerGetNumDevs @ 119; original VA 0x180002ff2
; assembly ev_29a204535efc1590d16633a4d9d61ebde0046ce891d6d510e60feb94465648f3; pseudocode ev_f8dd8bd38f2d37eb81519600074232d7309b0d5fd43976e06cdb63d4a4da1d1a
PUBLIC reconstructed_119
reconstructed_119 PROC FRAME
L_180002ff2:
    PUSH R14
    .pushreg R14
L_180002ff4:
    PUSH RSI
    .pushreg RSI
L_180002ff5:
    PUSH RDI
    .pushreg RDI
L_180002ff6:
    PUSH RBX
    .pushreg RBX
L_180002ff7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180002ffb:
    MOV R14, RCX
L_180002ffe:
    MOV RCX, qword ptr [OuterState+120]
L_180003005:
    TEST RCX, RCX
L_180003008:
    JZ L_18000303d
L_18000300a:
    MOV RSI, R9
L_18000300d:
    MOV RDI, R8
L_180003010:
    MOV RBX, RDX
L_180003013:
    LEA RDX, [name_119]
L_18000301a:
    CALL qword ptr [__imp_GetProcAddress]
L_180003020:
    TEST RAX, RAX
L_180003023:
    JZ L_18000303d
L_180003025:
    MOV RCX, R14
L_180003028:
    MOV RDX, RBX
L_18000302b:
    MOV R8, RDI
L_18000302e:
    MOV R9, RSI
L_180003031:
    ADD RSP, 028h
L_180003035:
    POP RBX
L_180003036:
    POP RDI
L_180003037:
    POP RSI
L_180003038:
    POP R14
L_18000303a:
    JMP RAX
L_18000303d:
    XOR EAX, EAX
L_18000303f:
    ADD RSP, 028h
L_180003043:
    POP RBX
L_180003044:
    POP RDI
L_180003045:
    POP RSI
L_180003046:
    POP R14
L_180003048:
    RET
reconstructed_119 ENDP
; mixerMessage @ 120; original VA 0x180003049
; assembly ev_c25ed4b0a782283d061e3edd09295b869e6286c5943ac0cef4213bc603f66244; pseudocode ev_5deddf0dc6652cd2da26e8a1171aad295cded03bb4a4d55bb17b077ec95b3be2
PUBLIC reconstructed_120
reconstructed_120 PROC FRAME
L_180003049:
    PUSH R14
    .pushreg R14
L_18000304b:
    PUSH RSI
    .pushreg RSI
L_18000304c:
    PUSH RDI
    .pushreg RDI
L_18000304d:
    PUSH RBX
    .pushreg RBX
L_18000304e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003052:
    MOV R14, RCX
L_180003055:
    MOV RCX, qword ptr [OuterState+120]
L_18000305c:
    TEST RCX, RCX
L_18000305f:
    JZ L_180003094
L_180003061:
    MOV RSI, R9
L_180003064:
    MOV RDI, R8
L_180003067:
    MOV RBX, RDX
L_18000306a:
    LEA RDX, [name_120]
L_180003071:
    CALL qword ptr [__imp_GetProcAddress]
L_180003077:
    TEST RAX, RAX
L_18000307a:
    JZ L_180003094
L_18000307c:
    MOV RCX, R14
L_18000307f:
    MOV RDX, RBX
L_180003082:
    MOV R8, RDI
L_180003085:
    MOV R9, RSI
L_180003088:
    ADD RSP, 028h
L_18000308c:
    POP RBX
L_18000308d:
    POP RDI
L_18000308e:
    POP RSI
L_18000308f:
    POP R14
L_180003091:
    JMP RAX
L_180003094:
    XOR EAX, EAX
L_180003096:
    ADD RSP, 028h
L_18000309a:
    POP RBX
L_18000309b:
    POP RDI
L_18000309c:
    POP RSI
L_18000309d:
    POP R14
L_18000309f:
    RET
reconstructed_120 ENDP
; mixerOpen @ 121; original VA 0x1800030a0
; assembly ev_dc395fae04a58596a74bee4ebc66409ac650e0c6a24f3e6e9ccfa9cd0243c834; pseudocode ev_db9ea45e40347f8b1eb247deda2f529e4088fd06fc69ce80633e9cbae1fbfe44
PUBLIC reconstructed_121
reconstructed_121 PROC FRAME
L_1800030a0:
    PUSH R14
    .pushreg R14
L_1800030a2:
    PUSH RSI
    .pushreg RSI
L_1800030a3:
    PUSH RDI
    .pushreg RDI
L_1800030a4:
    PUSH RBX
    .pushreg RBX
L_1800030a5:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800030a9:
    MOV R14, RCX
L_1800030ac:
    MOV RCX, qword ptr [OuterState+120]
L_1800030b3:
    TEST RCX, RCX
L_1800030b6:
    JZ L_1800030eb
L_1800030b8:
    MOV RSI, R9
L_1800030bb:
    MOV RDI, R8
L_1800030be:
    MOV RBX, RDX
L_1800030c1:
    LEA RDX, [name_121]
L_1800030c8:
    CALL qword ptr [__imp_GetProcAddress]
L_1800030ce:
    TEST RAX, RAX
L_1800030d1:
    JZ L_1800030eb
L_1800030d3:
    MOV RCX, R14
L_1800030d6:
    MOV RDX, RBX
L_1800030d9:
    MOV R8, RDI
L_1800030dc:
    MOV R9, RSI
L_1800030df:
    ADD RSP, 028h
L_1800030e3:
    POP RBX
L_1800030e4:
    POP RDI
L_1800030e5:
    POP RSI
L_1800030e6:
    POP R14
L_1800030e8:
    JMP RAX
L_1800030eb:
    XOR EAX, EAX
L_1800030ed:
    ADD RSP, 028h
L_1800030f1:
    POP RBX
L_1800030f2:
    POP RDI
L_1800030f3:
    POP RSI
L_1800030f4:
    POP R14
L_1800030f6:
    RET
reconstructed_121 ENDP
; mixerSetControlDetails @ 122; original VA 0x1800030f7
; assembly ev_bb1656a4a2cc8f20e0cbf497f20bc1399f30b2a70f8e730e766b096508201300; pseudocode ev_e2433fb907a28646bae8b6a2463cf12f17eab71450a7198d28d77c197d5e95fa
PUBLIC reconstructed_122
reconstructed_122 PROC FRAME
L_1800030f7:
    PUSH R14
    .pushreg R14
L_1800030f9:
    PUSH RSI
    .pushreg RSI
L_1800030fa:
    PUSH RDI
    .pushreg RDI
L_1800030fb:
    PUSH RBX
    .pushreg RBX
L_1800030fc:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003100:
    MOV R14, RCX
L_180003103:
    MOV RCX, qword ptr [OuterState+120]
L_18000310a:
    TEST RCX, RCX
L_18000310d:
    JZ L_180003142
L_18000310f:
    MOV RSI, R9
L_180003112:
    MOV RDI, R8
L_180003115:
    MOV RBX, RDX
L_180003118:
    LEA RDX, [name_122]
L_18000311f:
    CALL qword ptr [__imp_GetProcAddress]
L_180003125:
    TEST RAX, RAX
L_180003128:
    JZ L_180003142
L_18000312a:
    MOV RCX, R14
L_18000312d:
    MOV RDX, RBX
L_180003130:
    MOV R8, RDI
L_180003133:
    MOV R9, RSI
L_180003136:
    ADD RSP, 028h
L_18000313a:
    POP RBX
L_18000313b:
    POP RDI
L_18000313c:
    POP RSI
L_18000313d:
    POP R14
L_18000313f:
    JMP RAX
L_180003142:
    XOR EAX, EAX
L_180003144:
    ADD RSP, 028h
L_180003148:
    POP RBX
L_180003149:
    POP RDI
L_18000314a:
    POP RSI
L_18000314b:
    POP R14
L_18000314d:
    RET
reconstructed_122 ENDP
; mmDrvInstall @ 123; original VA 0x18000314e
; assembly ev_8b442aee6e76063e77639666ab785f86b486d54e4529b0e36c10cc25377bfce9; pseudocode ev_4ca7fecbacf72e8280becdf4a3a20859c0a897cf734a85d6e5d7769a192a51b1
PUBLIC reconstructed_123
reconstructed_123 PROC FRAME
L_18000314e:
    PUSH R14
    .pushreg R14
L_180003150:
    PUSH RSI
    .pushreg RSI
L_180003151:
    PUSH RDI
    .pushreg RDI
L_180003152:
    PUSH RBX
    .pushreg RBX
L_180003153:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003157:
    MOV R14, RCX
L_18000315a:
    MOV RCX, qword ptr [OuterState+120]
L_180003161:
    TEST RCX, RCX
L_180003164:
    JZ L_180003199
L_180003166:
    MOV RSI, R9
L_180003169:
    MOV RDI, R8
L_18000316c:
    MOV RBX, RDX
L_18000316f:
    LEA RDX, [name_123]
L_180003176:
    CALL qword ptr [__imp_GetProcAddress]
L_18000317c:
    TEST RAX, RAX
L_18000317f:
    JZ L_180003199
L_180003181:
    MOV RCX, R14
L_180003184:
    MOV RDX, RBX
L_180003187:
    MOV R8, RDI
L_18000318a:
    MOV R9, RSI
L_18000318d:
    ADD RSP, 028h
L_180003191:
    POP RBX
L_180003192:
    POP RDI
L_180003193:
    POP RSI
L_180003194:
    POP R14
L_180003196:
    JMP RAX
L_180003199:
    XOR EAX, EAX
L_18000319b:
    ADD RSP, 028h
L_18000319f:
    POP RBX
L_1800031a0:
    POP RDI
L_1800031a1:
    POP RSI
L_1800031a2:
    POP R14
L_1800031a4:
    RET
reconstructed_123 ENDP
; mmGetCurrentTask @ 124; original VA 0x1800031a5
; assembly ev_7ab0ed13e7999f5ab5ca13940d2eac4fbfff010ec9944d1cd4e74d332030a708; pseudocode ev_8597491c32023e3f4448155c659ab3f8635466d27f8ee92188f5c089ee17e319
PUBLIC reconstructed_124
reconstructed_124 PROC FRAME
L_1800031a5:
    PUSH R14
    .pushreg R14
L_1800031a7:
    PUSH RSI
    .pushreg RSI
L_1800031a8:
    PUSH RDI
    .pushreg RDI
L_1800031a9:
    PUSH RBX
    .pushreg RBX
L_1800031aa:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800031ae:
    MOV R14, RCX
L_1800031b1:
    MOV RCX, qword ptr [OuterState+120]
L_1800031b8:
    TEST RCX, RCX
L_1800031bb:
    JZ L_1800031f0
L_1800031bd:
    MOV RSI, R9
L_1800031c0:
    MOV RDI, R8
L_1800031c3:
    MOV RBX, RDX
L_1800031c6:
    LEA RDX, [name_124]
L_1800031cd:
    CALL qword ptr [__imp_GetProcAddress]
L_1800031d3:
    TEST RAX, RAX
L_1800031d6:
    JZ L_1800031f0
L_1800031d8:
    MOV RCX, R14
L_1800031db:
    MOV RDX, RBX
L_1800031de:
    MOV R8, RDI
L_1800031e1:
    MOV R9, RSI
L_1800031e4:
    ADD RSP, 028h
L_1800031e8:
    POP RBX
L_1800031e9:
    POP RDI
L_1800031ea:
    POP RSI
L_1800031eb:
    POP R14
L_1800031ed:
    JMP RAX
L_1800031f0:
    XOR EAX, EAX
L_1800031f2:
    ADD RSP, 028h
L_1800031f6:
    POP RBX
L_1800031f7:
    POP RDI
L_1800031f8:
    POP RSI
L_1800031f9:
    POP R14
L_1800031fb:
    RET
reconstructed_124 ENDP
; mmTaskBlock @ 125; original VA 0x180003976
; assembly ev_8a201c66613ab0ac4d8f9e2c4096948e33df56e91f34bab4df960a342e0400ce; pseudocode ev_efe07a7d7f0310f42e26422121db96cf344938ea0fdb91570eb72cb8e9cc295d
PUBLIC reconstructed_125
reconstructed_125 PROC FRAME
L_180003976:
    PUSH R14
    .pushreg R14
L_180003978:
    PUSH RSI
    .pushreg RSI
L_180003979:
    PUSH RDI
    .pushreg RDI
L_18000397a:
    PUSH RBX
    .pushreg RBX
L_18000397b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000397f:
    MOV R14, RCX
L_180003982:
    MOV RCX, qword ptr [OuterState+120]
L_180003989:
    TEST RCX, RCX
L_18000398c:
    JZ L_1800039c1
L_18000398e:
    MOV RSI, R9
L_180003991:
    MOV RDI, R8
L_180003994:
    MOV RBX, RDX
L_180003997:
    LEA RDX, [name_125]
L_18000399e:
    CALL qword ptr [__imp_GetProcAddress]
L_1800039a4:
    TEST RAX, RAX
L_1800039a7:
    JZ L_1800039c1
L_1800039a9:
    MOV RCX, R14
L_1800039ac:
    MOV RDX, RBX
L_1800039af:
    MOV R8, RDI
L_1800039b2:
    MOV R9, RSI
L_1800039b5:
    ADD RSP, 028h
L_1800039b9:
    POP RBX
L_1800039ba:
    POP RDI
L_1800039bb:
    POP RSI
L_1800039bc:
    POP R14
L_1800039be:
    JMP RAX
L_1800039c1:
    XOR EAX, EAX
L_1800039c3:
    ADD RSP, 028h
L_1800039c7:
    POP RBX
L_1800039c8:
    POP RDI
L_1800039c9:
    POP RSI
L_1800039ca:
    POP R14
L_1800039cc:
    RET
reconstructed_125 ENDP
; mmTaskCreate @ 126; original VA 0x1800039cd
; assembly ev_9b6a4b23be6f3995779c686f86dff992b7e91fb2093d2c7271292d041b76820f; pseudocode ev_732cc662a83f395ed49f93fbcabc86ed69b180137df2bdcb54942c10c5da48c6
PUBLIC reconstructed_126
reconstructed_126 PROC FRAME
L_1800039cd:
    PUSH R14
    .pushreg R14
L_1800039cf:
    PUSH RSI
    .pushreg RSI
L_1800039d0:
    PUSH RDI
    .pushreg RDI
L_1800039d1:
    PUSH RBX
    .pushreg RBX
L_1800039d2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800039d6:
    MOV R14, RCX
L_1800039d9:
    MOV RCX, qword ptr [OuterState+120]
L_1800039e0:
    TEST RCX, RCX
L_1800039e3:
    JZ L_180003a18
L_1800039e5:
    MOV RSI, R9
L_1800039e8:
    MOV RDI, R8
L_1800039eb:
    MOV RBX, RDX
L_1800039ee:
    LEA RDX, [name_126]
L_1800039f5:
    CALL qword ptr [__imp_GetProcAddress]
L_1800039fb:
    TEST RAX, RAX
L_1800039fe:
    JZ L_180003a18
L_180003a00:
    MOV RCX, R14
L_180003a03:
    MOV RDX, RBX
L_180003a06:
    MOV R8, RDI
L_180003a09:
    MOV R9, RSI
L_180003a0c:
    ADD RSP, 028h
L_180003a10:
    POP RBX
L_180003a11:
    POP RDI
L_180003a12:
    POP RSI
L_180003a13:
    POP R14
L_180003a15:
    JMP RAX
L_180003a18:
    XOR EAX, EAX
L_180003a1a:
    ADD RSP, 028h
L_180003a1e:
    POP RBX
L_180003a1f:
    POP RDI
L_180003a20:
    POP RSI
L_180003a21:
    POP R14
L_180003a23:
    RET
reconstructed_126 ENDP
; mmTaskSignal @ 127; original VA 0x180003a24
; assembly ev_cbfb738a7452c29fa8f999b73c3cc4db9c517e4330f191e5304c44bf55056f61; pseudocode ev_b7985f92fa5d4ba60cb464b57019f53b9692d44e6af6c96a1676f04abbc6434f
PUBLIC reconstructed_127
reconstructed_127 PROC FRAME
L_180003a24:
    PUSH R14
    .pushreg R14
L_180003a26:
    PUSH RSI
    .pushreg RSI
L_180003a27:
    PUSH RDI
    .pushreg RDI
L_180003a28:
    PUSH RBX
    .pushreg RBX
L_180003a29:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003a2d:
    MOV R14, RCX
L_180003a30:
    MOV RCX, qword ptr [OuterState+120]
L_180003a37:
    TEST RCX, RCX
L_180003a3a:
    JZ L_180003a6f
L_180003a3c:
    MOV RSI, R9
L_180003a3f:
    MOV RDI, R8
L_180003a42:
    MOV RBX, RDX
L_180003a45:
    LEA RDX, [name_127]
L_180003a4c:
    CALL qword ptr [__imp_GetProcAddress]
L_180003a52:
    TEST RAX, RAX
L_180003a55:
    JZ L_180003a6f
L_180003a57:
    MOV RCX, R14
L_180003a5a:
    MOV RDX, RBX
L_180003a5d:
    MOV R8, RDI
L_180003a60:
    MOV R9, RSI
L_180003a63:
    ADD RSP, 028h
L_180003a67:
    POP RBX
L_180003a68:
    POP RDI
L_180003a69:
    POP RSI
L_180003a6a:
    POP R14
L_180003a6c:
    JMP RAX
L_180003a6f:
    XOR EAX, EAX
L_180003a71:
    ADD RSP, 028h
L_180003a75:
    POP RBX
L_180003a76:
    POP RDI
L_180003a77:
    POP RSI
L_180003a78:
    POP R14
L_180003a7a:
    RET
reconstructed_127 ENDP
; mmTaskYield @ 128; original VA 0x180003a7b
; assembly ev_fee164bd75ed141ba4df7dfdbb4dc24d193e755f3d5609d946dc8447a209149a; pseudocode ev_6d1b107e4806f38173623dfc07b4d32afdb783672a2c5c02768bf0d489be23cf
PUBLIC reconstructed_128
reconstructed_128 PROC FRAME
L_180003a7b:
    PUSH R14
    .pushreg R14
L_180003a7d:
    PUSH RSI
    .pushreg RSI
L_180003a7e:
    PUSH RDI
    .pushreg RDI
L_180003a7f:
    PUSH RBX
    .pushreg RBX
L_180003a80:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003a84:
    MOV R14, RCX
L_180003a87:
    MOV RCX, qword ptr [OuterState+120]
L_180003a8e:
    TEST RCX, RCX
L_180003a91:
    JZ L_180003ac6
L_180003a93:
    MOV RSI, R9
L_180003a96:
    MOV RDI, R8
L_180003a99:
    MOV RBX, RDX
L_180003a9c:
    LEA RDX, [name_128]
L_180003aa3:
    CALL qword ptr [__imp_GetProcAddress]
L_180003aa9:
    TEST RAX, RAX
L_180003aac:
    JZ L_180003ac6
L_180003aae:
    MOV RCX, R14
L_180003ab1:
    MOV RDX, RBX
L_180003ab4:
    MOV R8, RDI
L_180003ab7:
    MOV R9, RSI
L_180003aba:
    ADD RSP, 028h
L_180003abe:
    POP RBX
L_180003abf:
    POP RDI
L_180003ac0:
    POP RSI
L_180003ac1:
    POP R14
L_180003ac3:
    JMP RAX
L_180003ac6:
    XOR EAX, EAX
L_180003ac8:
    ADD RSP, 028h
L_180003acc:
    POP RBX
L_180003acd:
    POP RDI
L_180003ace:
    POP RSI
L_180003acf:
    POP R14
L_180003ad1:
    RET
reconstructed_128 ENDP
; mmioAdvance @ 129; original VA 0x1800031fc
; assembly ev_e72194e73b6043e3338987d82ea4b57aa2770fe6647ac8b0985951c04b79ee9b; pseudocode ev_26d170be02fe1fb4508d0cdf1acededb648e64af7af82e9fd39f93a28f9524d9
PUBLIC reconstructed_129
reconstructed_129 PROC FRAME
L_1800031fc:
    PUSH R14
    .pushreg R14
L_1800031fe:
    PUSH RSI
    .pushreg RSI
L_1800031ff:
    PUSH RDI
    .pushreg RDI
L_180003200:
    PUSH RBX
    .pushreg RBX
L_180003201:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003205:
    MOV R14, RCX
L_180003208:
    MOV RCX, qword ptr [OuterState+120]
L_18000320f:
    TEST RCX, RCX
L_180003212:
    JZ L_180003247
L_180003214:
    MOV RSI, R9
L_180003217:
    MOV RDI, R8
L_18000321a:
    MOV RBX, RDX
L_18000321d:
    LEA RDX, [name_129]
L_180003224:
    CALL qword ptr [__imp_GetProcAddress]
L_18000322a:
    TEST RAX, RAX
L_18000322d:
    JZ L_180003247
L_18000322f:
    MOV RCX, R14
L_180003232:
    MOV RDX, RBX
L_180003235:
    MOV R8, RDI
L_180003238:
    MOV R9, RSI
L_18000323b:
    ADD RSP, 028h
L_18000323f:
    POP RBX
L_180003240:
    POP RDI
L_180003241:
    POP RSI
L_180003242:
    POP R14
L_180003244:
    JMP RAX
L_180003247:
    XOR EAX, EAX
L_180003249:
    ADD RSP, 028h
L_18000324d:
    POP RBX
L_18000324e:
    POP RDI
L_18000324f:
    POP RSI
L_180003250:
    POP R14
L_180003252:
    RET
reconstructed_129 ENDP
; mmioAscend @ 130; original VA 0x180003253
; assembly ev_9fa2f1f72e6ad90a9b1385d9a262c3ac876fda4f106eac59848fc8ba81f9104a; pseudocode ev_52592bf14816ab73c7742b21b3adfbfa8ce132629ef2a11970aafb78bd60c1ea
PUBLIC reconstructed_130
reconstructed_130 PROC FRAME
L_180003253:
    PUSH R14
    .pushreg R14
L_180003255:
    PUSH RSI
    .pushreg RSI
L_180003256:
    PUSH RDI
    .pushreg RDI
L_180003257:
    PUSH RBX
    .pushreg RBX
L_180003258:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000325c:
    MOV R14, RCX
L_18000325f:
    MOV RCX, qword ptr [OuterState+120]
L_180003266:
    TEST RCX, RCX
L_180003269:
    JZ L_18000329e
L_18000326b:
    MOV RSI, R9
L_18000326e:
    MOV RDI, R8
L_180003271:
    MOV RBX, RDX
L_180003274:
    LEA RDX, [name_130]
L_18000327b:
    CALL qword ptr [__imp_GetProcAddress]
L_180003281:
    TEST RAX, RAX
L_180003284:
    JZ L_18000329e
L_180003286:
    MOV RCX, R14
L_180003289:
    MOV RDX, RBX
L_18000328c:
    MOV R8, RDI
L_18000328f:
    MOV R9, RSI
L_180003292:
    ADD RSP, 028h
L_180003296:
    POP RBX
L_180003297:
    POP RDI
L_180003298:
    POP RSI
L_180003299:
    POP R14
L_18000329b:
    JMP RAX
L_18000329e:
    XOR EAX, EAX
L_1800032a0:
    ADD RSP, 028h
L_1800032a4:
    POP RBX
L_1800032a5:
    POP RDI
L_1800032a6:
    POP RSI
L_1800032a7:
    POP R14
L_1800032a9:
    RET
reconstructed_130 ENDP
; mmioClose @ 131; original VA 0x1800032aa
; assembly ev_ee6d2de13af57d9b438855e94449dba74e0c108897985f99eecb7c2ae895fbc2; pseudocode ev_2ae54e4506d67ba229744dc8f9cf04926391a617f67282177dafa53371492a41
PUBLIC reconstructed_131
reconstructed_131 PROC FRAME
L_1800032aa:
    PUSH R14
    .pushreg R14
L_1800032ac:
    PUSH RSI
    .pushreg RSI
L_1800032ad:
    PUSH RDI
    .pushreg RDI
L_1800032ae:
    PUSH RBX
    .pushreg RBX
L_1800032af:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800032b3:
    MOV R14, RCX
L_1800032b6:
    MOV RCX, qword ptr [OuterState+120]
L_1800032bd:
    TEST RCX, RCX
L_1800032c0:
    JZ L_1800032f5
L_1800032c2:
    MOV RSI, R9
L_1800032c5:
    MOV RDI, R8
L_1800032c8:
    MOV RBX, RDX
L_1800032cb:
    LEA RDX, [name_131]
L_1800032d2:
    CALL qword ptr [__imp_GetProcAddress]
L_1800032d8:
    TEST RAX, RAX
L_1800032db:
    JZ L_1800032f5
L_1800032dd:
    MOV RCX, R14
L_1800032e0:
    MOV RDX, RBX
L_1800032e3:
    MOV R8, RDI
L_1800032e6:
    MOV R9, RSI
L_1800032e9:
    ADD RSP, 028h
L_1800032ed:
    POP RBX
L_1800032ee:
    POP RDI
L_1800032ef:
    POP RSI
L_1800032f0:
    POP R14
L_1800032f2:
    JMP RAX
L_1800032f5:
    XOR EAX, EAX
L_1800032f7:
    ADD RSP, 028h
L_1800032fb:
    POP RBX
L_1800032fc:
    POP RDI
L_1800032fd:
    POP RSI
L_1800032fe:
    POP R14
L_180003300:
    RET
reconstructed_131 ENDP
; mmioCreateChunk @ 132; original VA 0x180003301
; assembly ev_3431f769d17b832e5442872f69c5d9e423529117e30a938764f4035f25955259; pseudocode ev_47b99b0c16d9f7e87687d2c02befa5401aab7db74b96af98d94055887337397d
PUBLIC reconstructed_132
reconstructed_132 PROC FRAME
L_180003301:
    PUSH R14
    .pushreg R14
L_180003303:
    PUSH RSI
    .pushreg RSI
L_180003304:
    PUSH RDI
    .pushreg RDI
L_180003305:
    PUSH RBX
    .pushreg RBX
L_180003306:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000330a:
    MOV R14, RCX
L_18000330d:
    MOV RCX, qword ptr [OuterState+120]
L_180003314:
    TEST RCX, RCX
L_180003317:
    JZ L_18000334c
L_180003319:
    MOV RSI, R9
L_18000331c:
    MOV RDI, R8
L_18000331f:
    MOV RBX, RDX
L_180003322:
    LEA RDX, [name_132]
L_180003329:
    CALL qword ptr [__imp_GetProcAddress]
L_18000332f:
    TEST RAX, RAX
L_180003332:
    JZ L_18000334c
L_180003334:
    MOV RCX, R14
L_180003337:
    MOV RDX, RBX
L_18000333a:
    MOV R8, RDI
L_18000333d:
    MOV R9, RSI
L_180003340:
    ADD RSP, 028h
L_180003344:
    POP RBX
L_180003345:
    POP RDI
L_180003346:
    POP RSI
L_180003347:
    POP R14
L_180003349:
    JMP RAX
L_18000334c:
    XOR EAX, EAX
L_18000334e:
    ADD RSP, 028h
L_180003352:
    POP RBX
L_180003353:
    POP RDI
L_180003354:
    POP RSI
L_180003355:
    POP R14
L_180003357:
    RET
reconstructed_132 ENDP
; mmioDescend @ 133; original VA 0x180003358
; assembly ev_4b8f691c91d587ec0946299586e15228638609ac7cd62a85dc112b781200d83e; pseudocode ev_e573202ba01a230d303ca93f93302437cca4c3dfbfcdece95e137f1d53d42d3d
PUBLIC reconstructed_133
reconstructed_133 PROC FRAME
L_180003358:
    PUSH R14
    .pushreg R14
L_18000335a:
    PUSH RSI
    .pushreg RSI
L_18000335b:
    PUSH RDI
    .pushreg RDI
L_18000335c:
    PUSH RBX
    .pushreg RBX
L_18000335d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003361:
    MOV R14, RCX
L_180003364:
    MOV RCX, qword ptr [OuterState+120]
L_18000336b:
    TEST RCX, RCX
L_18000336e:
    JZ L_1800033a3
L_180003370:
    MOV RSI, R9
L_180003373:
    MOV RDI, R8
L_180003376:
    MOV RBX, RDX
L_180003379:
    LEA RDX, [name_133]
L_180003380:
    CALL qword ptr [__imp_GetProcAddress]
L_180003386:
    TEST RAX, RAX
L_180003389:
    JZ L_1800033a3
L_18000338b:
    MOV RCX, R14
L_18000338e:
    MOV RDX, RBX
L_180003391:
    MOV R8, RDI
L_180003394:
    MOV R9, RSI
L_180003397:
    ADD RSP, 028h
L_18000339b:
    POP RBX
L_18000339c:
    POP RDI
L_18000339d:
    POP RSI
L_18000339e:
    POP R14
L_1800033a0:
    JMP RAX
L_1800033a3:
    XOR EAX, EAX
L_1800033a5:
    ADD RSP, 028h
L_1800033a9:
    POP RBX
L_1800033aa:
    POP RDI
L_1800033ab:
    POP RSI
L_1800033ac:
    POP R14
L_1800033ae:
    RET
reconstructed_133 ENDP
; mmioFlush @ 134; original VA 0x1800033af
; assembly ev_8fb940d62e031f8e996b7151873dbe547e52de800459d258442cb37d55020f9b; pseudocode ev_2f3bc4c1a58f0fcaef3c6d9a1428492b6d3cef308a389d6b9870d4e5bf350104
PUBLIC reconstructed_134
reconstructed_134 PROC FRAME
L_1800033af:
    PUSH R14
    .pushreg R14
L_1800033b1:
    PUSH RSI
    .pushreg RSI
L_1800033b2:
    PUSH RDI
    .pushreg RDI
L_1800033b3:
    PUSH RBX
    .pushreg RBX
L_1800033b4:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800033b8:
    MOV R14, RCX
L_1800033bb:
    MOV RCX, qword ptr [OuterState+120]
L_1800033c2:
    TEST RCX, RCX
L_1800033c5:
    JZ L_1800033fa
L_1800033c7:
    MOV RSI, R9
L_1800033ca:
    MOV RDI, R8
L_1800033cd:
    MOV RBX, RDX
L_1800033d0:
    LEA RDX, [name_134]
L_1800033d7:
    CALL qword ptr [__imp_GetProcAddress]
L_1800033dd:
    TEST RAX, RAX
L_1800033e0:
    JZ L_1800033fa
L_1800033e2:
    MOV RCX, R14
L_1800033e5:
    MOV RDX, RBX
L_1800033e8:
    MOV R8, RDI
L_1800033eb:
    MOV R9, RSI
L_1800033ee:
    ADD RSP, 028h
L_1800033f2:
    POP RBX
L_1800033f3:
    POP RDI
L_1800033f4:
    POP RSI
L_1800033f5:
    POP R14
L_1800033f7:
    JMP RAX
L_1800033fa:
    XOR EAX, EAX
L_1800033fc:
    ADD RSP, 028h
L_180003400:
    POP RBX
L_180003401:
    POP RDI
L_180003402:
    POP RSI
L_180003403:
    POP R14
L_180003405:
    RET
reconstructed_134 ENDP
; mmioGetInfo @ 135; original VA 0x180003406
; assembly ev_01239d4fb8b86333e8323321634ca6d8085998ea91d44dc70d0991ac88506a45; pseudocode ev_911374564fb32c98a9dd2db3c275c2e9c6267bd52a67ffae680113507d9d1b3b
PUBLIC reconstructed_135
reconstructed_135 PROC FRAME
L_180003406:
    PUSH R14
    .pushreg R14
L_180003408:
    PUSH RSI
    .pushreg RSI
L_180003409:
    PUSH RDI
    .pushreg RDI
L_18000340a:
    PUSH RBX
    .pushreg RBX
L_18000340b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000340f:
    MOV R14, RCX
L_180003412:
    MOV RCX, qword ptr [OuterState+120]
L_180003419:
    TEST RCX, RCX
L_18000341c:
    JZ L_180003451
L_18000341e:
    MOV RSI, R9
L_180003421:
    MOV RDI, R8
L_180003424:
    MOV RBX, RDX
L_180003427:
    LEA RDX, [name_135]
L_18000342e:
    CALL qword ptr [__imp_GetProcAddress]
L_180003434:
    TEST RAX, RAX
L_180003437:
    JZ L_180003451
L_180003439:
    MOV RCX, R14
L_18000343c:
    MOV RDX, RBX
L_18000343f:
    MOV R8, RDI
L_180003442:
    MOV R9, RSI
L_180003445:
    ADD RSP, 028h
L_180003449:
    POP RBX
L_18000344a:
    POP RDI
L_18000344b:
    POP RSI
L_18000344c:
    POP R14
L_18000344e:
    JMP RAX
L_180003451:
    XOR EAX, EAX
L_180003453:
    ADD RSP, 028h
L_180003457:
    POP RBX
L_180003458:
    POP RDI
L_180003459:
    POP RSI
L_18000345a:
    POP R14
L_18000345c:
    RET
reconstructed_135 ENDP
; mmioInstallIOProcA @ 136; original VA 0x18000345d
; assembly ev_eee44002ea17b08aed43ec62084ca37b9749ae72ebb74da31e4c2cdbd5ae6a69; pseudocode ev_7393890a6b1bbc790c3fbbb82fa034375606d246e1a2607abfd311c4e4ad7e93
PUBLIC reconstructed_136
reconstructed_136 PROC FRAME
L_18000345d:
    PUSH R14
    .pushreg R14
L_18000345f:
    PUSH RSI
    .pushreg RSI
L_180003460:
    PUSH RDI
    .pushreg RDI
L_180003461:
    PUSH RBX
    .pushreg RBX
L_180003462:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003466:
    MOV R14, RCX
L_180003469:
    MOV RCX, qword ptr [OuterState+120]
L_180003470:
    TEST RCX, RCX
L_180003473:
    JZ L_1800034a8
L_180003475:
    MOV RSI, R9
L_180003478:
    MOV RDI, R8
L_18000347b:
    MOV RBX, RDX
L_18000347e:
    LEA RDX, [name_136]
L_180003485:
    CALL qword ptr [__imp_GetProcAddress]
L_18000348b:
    TEST RAX, RAX
L_18000348e:
    JZ L_1800034a8
L_180003490:
    MOV RCX, R14
L_180003493:
    MOV RDX, RBX
L_180003496:
    MOV R8, RDI
L_180003499:
    MOV R9, RSI
L_18000349c:
    ADD RSP, 028h
L_1800034a0:
    POP RBX
L_1800034a1:
    POP RDI
L_1800034a2:
    POP RSI
L_1800034a3:
    POP R14
L_1800034a5:
    JMP RAX
L_1800034a8:
    XOR EAX, EAX
L_1800034aa:
    ADD RSP, 028h
L_1800034ae:
    POP RBX
L_1800034af:
    POP RDI
L_1800034b0:
    POP RSI
L_1800034b1:
    POP R14
L_1800034b3:
    RET
reconstructed_136 ENDP
; mmioInstallIOProcW @ 137; original VA 0x1800034b4
; assembly ev_f349f645561582370f936a3b48a2fc3ac4fd35f1af66313c09669a271ec422c0; pseudocode ev_71fefdd11f0f50cc1f828499fdd8efd94ee974f5525169c5a355024dc01f2028
PUBLIC reconstructed_137
reconstructed_137 PROC FRAME
L_1800034b4:
    PUSH R14
    .pushreg R14
L_1800034b6:
    PUSH RSI
    .pushreg RSI
L_1800034b7:
    PUSH RDI
    .pushreg RDI
L_1800034b8:
    PUSH RBX
    .pushreg RBX
L_1800034b9:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800034bd:
    MOV R14, RCX
L_1800034c0:
    MOV RCX, qword ptr [OuterState+120]
L_1800034c7:
    TEST RCX, RCX
L_1800034ca:
    JZ L_1800034ff
L_1800034cc:
    MOV RSI, R9
L_1800034cf:
    MOV RDI, R8
L_1800034d2:
    MOV RBX, RDX
L_1800034d5:
    LEA RDX, [name_137]
L_1800034dc:
    CALL qword ptr [__imp_GetProcAddress]
L_1800034e2:
    TEST RAX, RAX
L_1800034e5:
    JZ L_1800034ff
L_1800034e7:
    MOV RCX, R14
L_1800034ea:
    MOV RDX, RBX
L_1800034ed:
    MOV R8, RDI
L_1800034f0:
    MOV R9, RSI
L_1800034f3:
    ADD RSP, 028h
L_1800034f7:
    POP RBX
L_1800034f8:
    POP RDI
L_1800034f9:
    POP RSI
L_1800034fa:
    POP R14
L_1800034fc:
    JMP RAX
L_1800034ff:
    XOR EAX, EAX
L_180003501:
    ADD RSP, 028h
L_180003505:
    POP RBX
L_180003506:
    POP RDI
L_180003507:
    POP RSI
L_180003508:
    POP R14
L_18000350a:
    RET
reconstructed_137 ENDP
; mmioOpenA @ 138; original VA 0x18000350b
; assembly ev_ecb79973c7c952a39f5f1859e6a862d0b3699ee7f072831ea75b0e45fc35eb4f; pseudocode ev_e7ed44c46d0ab65a05607a5926f9d561c4bda4bbb7a9c695aa4cf0b1a7b43a92
PUBLIC reconstructed_138
reconstructed_138 PROC FRAME
L_18000350b:
    PUSH R14
    .pushreg R14
L_18000350d:
    PUSH RSI
    .pushreg RSI
L_18000350e:
    PUSH RDI
    .pushreg RDI
L_18000350f:
    PUSH RBX
    .pushreg RBX
L_180003510:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003514:
    MOV R14, RCX
L_180003517:
    MOV RCX, qword ptr [OuterState+120]
L_18000351e:
    TEST RCX, RCX
L_180003521:
    JZ L_180003556
L_180003523:
    MOV RSI, R9
L_180003526:
    MOV RDI, R8
L_180003529:
    MOV RBX, RDX
L_18000352c:
    LEA RDX, [name_138]
L_180003533:
    CALL qword ptr [__imp_GetProcAddress]
L_180003539:
    TEST RAX, RAX
L_18000353c:
    JZ L_180003556
L_18000353e:
    MOV RCX, R14
L_180003541:
    MOV RDX, RBX
L_180003544:
    MOV R8, RDI
L_180003547:
    MOV R9, RSI
L_18000354a:
    ADD RSP, 028h
L_18000354e:
    POP RBX
L_18000354f:
    POP RDI
L_180003550:
    POP RSI
L_180003551:
    POP R14
L_180003553:
    JMP RAX
L_180003556:
    XOR EAX, EAX
L_180003558:
    ADD RSP, 028h
L_18000355c:
    POP RBX
L_18000355d:
    POP RDI
L_18000355e:
    POP RSI
L_18000355f:
    POP R14
L_180003561:
    RET
reconstructed_138 ENDP
; mmioOpenW @ 139; original VA 0x180003562
; assembly ev_225e0f380c365fc6284dc9551712e5905e4bf8315b94dd97df7ab770df5337e1; pseudocode ev_25afb503ce2d23e9ca2a28201446a236f5bafd9a8c608eab93c50bc28fc86aca
PUBLIC reconstructed_139
reconstructed_139 PROC FRAME
L_180003562:
    PUSH R14
    .pushreg R14
L_180003564:
    PUSH RSI
    .pushreg RSI
L_180003565:
    PUSH RDI
    .pushreg RDI
L_180003566:
    PUSH RBX
    .pushreg RBX
L_180003567:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000356b:
    MOV R14, RCX
L_18000356e:
    MOV RCX, qword ptr [OuterState+120]
L_180003575:
    TEST RCX, RCX
L_180003578:
    JZ L_1800035ad
L_18000357a:
    MOV RSI, R9
L_18000357d:
    MOV RDI, R8
L_180003580:
    MOV RBX, RDX
L_180003583:
    LEA RDX, [name_139]
L_18000358a:
    CALL qword ptr [__imp_GetProcAddress]
L_180003590:
    TEST RAX, RAX
L_180003593:
    JZ L_1800035ad
L_180003595:
    MOV RCX, R14
L_180003598:
    MOV RDX, RBX
L_18000359b:
    MOV R8, RDI
L_18000359e:
    MOV R9, RSI
L_1800035a1:
    ADD RSP, 028h
L_1800035a5:
    POP RBX
L_1800035a6:
    POP RDI
L_1800035a7:
    POP RSI
L_1800035a8:
    POP R14
L_1800035aa:
    JMP RAX
L_1800035ad:
    XOR EAX, EAX
L_1800035af:
    ADD RSP, 028h
L_1800035b3:
    POP RBX
L_1800035b4:
    POP RDI
L_1800035b5:
    POP RSI
L_1800035b6:
    POP R14
L_1800035b8:
    RET
reconstructed_139 ENDP
; mmioRead @ 140; original VA 0x1800035b9
; assembly ev_5b27c930a65fdd4f58b600eafcb7662c00109b81fb48178cc8a5200b2d877d96; pseudocode ev_2c558205f31f3c9ddc3c867e0a031ec66de2edaee85bb5e97748e5917a62094b
PUBLIC reconstructed_140
reconstructed_140 PROC FRAME
L_1800035b9:
    PUSH R14
    .pushreg R14
L_1800035bb:
    PUSH RSI
    .pushreg RSI
L_1800035bc:
    PUSH RDI
    .pushreg RDI
L_1800035bd:
    PUSH RBX
    .pushreg RBX
L_1800035be:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800035c2:
    MOV R14, RCX
L_1800035c5:
    MOV RCX, qword ptr [OuterState+120]
L_1800035cc:
    TEST RCX, RCX
L_1800035cf:
    JZ L_180003604
L_1800035d1:
    MOV RSI, R9
L_1800035d4:
    MOV RDI, R8
L_1800035d7:
    MOV RBX, RDX
L_1800035da:
    LEA RDX, [name_140]
L_1800035e1:
    CALL qword ptr [__imp_GetProcAddress]
L_1800035e7:
    TEST RAX, RAX
L_1800035ea:
    JZ L_180003604
L_1800035ec:
    MOV RCX, R14
L_1800035ef:
    MOV RDX, RBX
L_1800035f2:
    MOV R8, RDI
L_1800035f5:
    MOV R9, RSI
L_1800035f8:
    ADD RSP, 028h
L_1800035fc:
    POP RBX
L_1800035fd:
    POP RDI
L_1800035fe:
    POP RSI
L_1800035ff:
    POP R14
L_180003601:
    JMP RAX
L_180003604:
    XOR EAX, EAX
L_180003606:
    ADD RSP, 028h
L_18000360a:
    POP RBX
L_18000360b:
    POP RDI
L_18000360c:
    POP RSI
L_18000360d:
    POP R14
L_18000360f:
    RET
reconstructed_140 ENDP
; mmioRenameA @ 141; original VA 0x180003610
; assembly ev_33792285940d0f0f72bf7578c07d6a02cf38b350d191285b2c4eb5a09b8ade26; pseudocode ev_23c8af7f9d03efdf11746a5d6d717e18d396def38b9a06b4f88cbec3b8d77f04
PUBLIC reconstructed_141
reconstructed_141 PROC FRAME
L_180003610:
    PUSH R14
    .pushreg R14
L_180003612:
    PUSH RSI
    .pushreg RSI
L_180003613:
    PUSH RDI
    .pushreg RDI
L_180003614:
    PUSH RBX
    .pushreg RBX
L_180003615:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003619:
    MOV R14, RCX
L_18000361c:
    MOV RCX, qword ptr [OuterState+120]
L_180003623:
    TEST RCX, RCX
L_180003626:
    JZ L_18000365b
L_180003628:
    MOV RSI, R9
L_18000362b:
    MOV RDI, R8
L_18000362e:
    MOV RBX, RDX
L_180003631:
    LEA RDX, [name_141]
L_180003638:
    CALL qword ptr [__imp_GetProcAddress]
L_18000363e:
    TEST RAX, RAX
L_180003641:
    JZ L_18000365b
L_180003643:
    MOV RCX, R14
L_180003646:
    MOV RDX, RBX
L_180003649:
    MOV R8, RDI
L_18000364c:
    MOV R9, RSI
L_18000364f:
    ADD RSP, 028h
L_180003653:
    POP RBX
L_180003654:
    POP RDI
L_180003655:
    POP RSI
L_180003656:
    POP R14
L_180003658:
    JMP RAX
L_18000365b:
    XOR EAX, EAX
L_18000365d:
    ADD RSP, 028h
L_180003661:
    POP RBX
L_180003662:
    POP RDI
L_180003663:
    POP RSI
L_180003664:
    POP R14
L_180003666:
    RET
reconstructed_141 ENDP
; mmioRenameW @ 142; original VA 0x180003667
; assembly ev_02f7a9d28da7a604a53edfbf1b0c5d2bb3d214aec94a456a17a17d6159de59e5; pseudocode ev_c24e7014b99f3f41a4efb5b8f3edc5c638f52cf036ef64b98ad69dd838fbcc9d
PUBLIC reconstructed_142
reconstructed_142 PROC FRAME
L_180003667:
    PUSH R14
    .pushreg R14
L_180003669:
    PUSH RSI
    .pushreg RSI
L_18000366a:
    PUSH RDI
    .pushreg RDI
L_18000366b:
    PUSH RBX
    .pushreg RBX
L_18000366c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003670:
    MOV R14, RCX
L_180003673:
    MOV RCX, qword ptr [OuterState+120]
L_18000367a:
    TEST RCX, RCX
L_18000367d:
    JZ L_1800036b2
L_18000367f:
    MOV RSI, R9
L_180003682:
    MOV RDI, R8
L_180003685:
    MOV RBX, RDX
L_180003688:
    LEA RDX, [name_142]
L_18000368f:
    CALL qword ptr [__imp_GetProcAddress]
L_180003695:
    TEST RAX, RAX
L_180003698:
    JZ L_1800036b2
L_18000369a:
    MOV RCX, R14
L_18000369d:
    MOV RDX, RBX
L_1800036a0:
    MOV R8, RDI
L_1800036a3:
    MOV R9, RSI
L_1800036a6:
    ADD RSP, 028h
L_1800036aa:
    POP RBX
L_1800036ab:
    POP RDI
L_1800036ac:
    POP RSI
L_1800036ad:
    POP R14
L_1800036af:
    JMP RAX
L_1800036b2:
    XOR EAX, EAX
L_1800036b4:
    ADD RSP, 028h
L_1800036b8:
    POP RBX
L_1800036b9:
    POP RDI
L_1800036ba:
    POP RSI
L_1800036bb:
    POP R14
L_1800036bd:
    RET
reconstructed_142 ENDP
; mmioSeek @ 143; original VA 0x1800036be
; assembly ev_dc3dd3916b9ff69ea6d4d3ad574b325c2ccb70c9a061b4312a9d5661322ffbf4; pseudocode ev_861ca8b861f65a8dea46c1ee97dbd4d5f1254f523aba1ec95768542bc8e7bbdf
PUBLIC reconstructed_143
reconstructed_143 PROC FRAME
L_1800036be:
    PUSH R14
    .pushreg R14
L_1800036c0:
    PUSH RSI
    .pushreg RSI
L_1800036c1:
    PUSH RDI
    .pushreg RDI
L_1800036c2:
    PUSH RBX
    .pushreg RBX
L_1800036c3:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800036c7:
    MOV R14, RCX
L_1800036ca:
    MOV RCX, qword ptr [OuterState+120]
L_1800036d1:
    TEST RCX, RCX
L_1800036d4:
    JZ L_180003709
L_1800036d6:
    MOV RSI, R9
L_1800036d9:
    MOV RDI, R8
L_1800036dc:
    MOV RBX, RDX
L_1800036df:
    LEA RDX, [name_143]
L_1800036e6:
    CALL qword ptr [__imp_GetProcAddress]
L_1800036ec:
    TEST RAX, RAX
L_1800036ef:
    JZ L_180003709
L_1800036f1:
    MOV RCX, R14
L_1800036f4:
    MOV RDX, RBX
L_1800036f7:
    MOV R8, RDI
L_1800036fa:
    MOV R9, RSI
L_1800036fd:
    ADD RSP, 028h
L_180003701:
    POP RBX
L_180003702:
    POP RDI
L_180003703:
    POP RSI
L_180003704:
    POP R14
L_180003706:
    JMP RAX
L_180003709:
    XOR EAX, EAX
L_18000370b:
    ADD RSP, 028h
L_18000370f:
    POP RBX
L_180003710:
    POP RDI
L_180003711:
    POP RSI
L_180003712:
    POP R14
L_180003714:
    RET
reconstructed_143 ENDP
; mmioSendMessage @ 144; original VA 0x180003715
; assembly ev_7d19c92d63eb77a1a458833b6a266ff73e0cd3968b58a3cc31c3ca5abac7ac47; pseudocode ev_1b9b4e16555801f1571ce276aa33de6ade259e42cad91be8d03877653ff22ae5
PUBLIC reconstructed_144
reconstructed_144 PROC FRAME
L_180003715:
    PUSH R14
    .pushreg R14
L_180003717:
    PUSH RSI
    .pushreg RSI
L_180003718:
    PUSH RDI
    .pushreg RDI
L_180003719:
    PUSH RBX
    .pushreg RBX
L_18000371a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000371e:
    MOV R14, RCX
L_180003721:
    MOV RCX, qword ptr [OuterState+120]
L_180003728:
    TEST RCX, RCX
L_18000372b:
    JZ L_180003760
L_18000372d:
    MOV RSI, R9
L_180003730:
    MOV RDI, R8
L_180003733:
    MOV RBX, RDX
L_180003736:
    LEA RDX, [name_144]
L_18000373d:
    CALL qword ptr [__imp_GetProcAddress]
L_180003743:
    TEST RAX, RAX
L_180003746:
    JZ L_180003760
L_180003748:
    MOV RCX, R14
L_18000374b:
    MOV RDX, RBX
L_18000374e:
    MOV R8, RDI
L_180003751:
    MOV R9, RSI
L_180003754:
    ADD RSP, 028h
L_180003758:
    POP RBX
L_180003759:
    POP RDI
L_18000375a:
    POP RSI
L_18000375b:
    POP R14
L_18000375d:
    JMP RAX
L_180003760:
    XOR EAX, EAX
L_180003762:
    ADD RSP, 028h
L_180003766:
    POP RBX
L_180003767:
    POP RDI
L_180003768:
    POP RSI
L_180003769:
    POP R14
L_18000376b:
    RET
reconstructed_144 ENDP
; mmioSetBuffer @ 145; original VA 0x18000376c
; assembly ev_00c35f891e072a123fefd19d504d7ae278ac29b42c0bcf7abd9a27e976433ec6; pseudocode ev_6f8f81bc129bf508f9e015e3d7cd6484849bf13ac8afee4b2e44a0fcc09dcabe
PUBLIC reconstructed_145
reconstructed_145 PROC FRAME
L_18000376c:
    PUSH R14
    .pushreg R14
L_18000376e:
    PUSH RSI
    .pushreg RSI
L_18000376f:
    PUSH RDI
    .pushreg RDI
L_180003770:
    PUSH RBX
    .pushreg RBX
L_180003771:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003775:
    MOV R14, RCX
L_180003778:
    MOV RCX, qword ptr [OuterState+120]
L_18000377f:
    TEST RCX, RCX
L_180003782:
    JZ L_1800037b7
L_180003784:
    MOV RSI, R9
L_180003787:
    MOV RDI, R8
L_18000378a:
    MOV RBX, RDX
L_18000378d:
    LEA RDX, [name_145]
L_180003794:
    CALL qword ptr [__imp_GetProcAddress]
L_18000379a:
    TEST RAX, RAX
L_18000379d:
    JZ L_1800037b7
L_18000379f:
    MOV RCX, R14
L_1800037a2:
    MOV RDX, RBX
L_1800037a5:
    MOV R8, RDI
L_1800037a8:
    MOV R9, RSI
L_1800037ab:
    ADD RSP, 028h
L_1800037af:
    POP RBX
L_1800037b0:
    POP RDI
L_1800037b1:
    POP RSI
L_1800037b2:
    POP R14
L_1800037b4:
    JMP RAX
L_1800037b7:
    XOR EAX, EAX
L_1800037b9:
    ADD RSP, 028h
L_1800037bd:
    POP RBX
L_1800037be:
    POP RDI
L_1800037bf:
    POP RSI
L_1800037c0:
    POP R14
L_1800037c2:
    RET
reconstructed_145 ENDP
; mmioSetInfo @ 146; original VA 0x1800037c3
; assembly ev_4ea7b74c28f2c475a7ee71d9f73cc43129695f90245ab4e6cdda3efd0c006820; pseudocode ev_e5f26f62d3e702f3457016ca030c870401d3fd201c435ccf11f1c5b7c33513b5
PUBLIC reconstructed_146
reconstructed_146 PROC FRAME
L_1800037c3:
    PUSH R14
    .pushreg R14
L_1800037c5:
    PUSH RSI
    .pushreg RSI
L_1800037c6:
    PUSH RDI
    .pushreg RDI
L_1800037c7:
    PUSH RBX
    .pushreg RBX
L_1800037c8:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800037cc:
    MOV R14, RCX
L_1800037cf:
    MOV RCX, qword ptr [OuterState+120]
L_1800037d6:
    TEST RCX, RCX
L_1800037d9:
    JZ L_18000380e
L_1800037db:
    MOV RSI, R9
L_1800037de:
    MOV RDI, R8
L_1800037e1:
    MOV RBX, RDX
L_1800037e4:
    LEA RDX, [name_146]
L_1800037eb:
    CALL qword ptr [__imp_GetProcAddress]
L_1800037f1:
    TEST RAX, RAX
L_1800037f4:
    JZ L_18000380e
L_1800037f6:
    MOV RCX, R14
L_1800037f9:
    MOV RDX, RBX
L_1800037fc:
    MOV R8, RDI
L_1800037ff:
    MOV R9, RSI
L_180003802:
    ADD RSP, 028h
L_180003806:
    POP RBX
L_180003807:
    POP RDI
L_180003808:
    POP RSI
L_180003809:
    POP R14
L_18000380b:
    JMP RAX
L_18000380e:
    XOR EAX, EAX
L_180003810:
    ADD RSP, 028h
L_180003814:
    POP RBX
L_180003815:
    POP RDI
L_180003816:
    POP RSI
L_180003817:
    POP R14
L_180003819:
    RET
reconstructed_146 ENDP
; mmioStringToFOURCCA @ 147; original VA 0x18000381a
; assembly ev_f2b1d4dbeb739debd41dc6645df103bf26dfa52c368e80a3aa40acbc59d07b32; pseudocode ev_bd48b5f9e2c603677ccc862726bce9b45a847953e590f02dbd81e1feca453f22
PUBLIC reconstructed_147
reconstructed_147 PROC FRAME
L_18000381a:
    PUSH R14
    .pushreg R14
L_18000381c:
    PUSH RSI
    .pushreg RSI
L_18000381d:
    PUSH RDI
    .pushreg RDI
L_18000381e:
    PUSH RBX
    .pushreg RBX
L_18000381f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003823:
    MOV R14, RCX
L_180003826:
    MOV RCX, qword ptr [OuterState+120]
L_18000382d:
    TEST RCX, RCX
L_180003830:
    JZ L_180003865
L_180003832:
    MOV RSI, R9
L_180003835:
    MOV RDI, R8
L_180003838:
    MOV RBX, RDX
L_18000383b:
    LEA RDX, [name_147]
L_180003842:
    CALL qword ptr [__imp_GetProcAddress]
L_180003848:
    TEST RAX, RAX
L_18000384b:
    JZ L_180003865
L_18000384d:
    MOV RCX, R14
L_180003850:
    MOV RDX, RBX
L_180003853:
    MOV R8, RDI
L_180003856:
    MOV R9, RSI
L_180003859:
    ADD RSP, 028h
L_18000385d:
    POP RBX
L_18000385e:
    POP RDI
L_18000385f:
    POP RSI
L_180003860:
    POP R14
L_180003862:
    JMP RAX
L_180003865:
    XOR EAX, EAX
L_180003867:
    ADD RSP, 028h
L_18000386b:
    POP RBX
L_18000386c:
    POP RDI
L_18000386d:
    POP RSI
L_18000386e:
    POP R14
L_180003870:
    RET
reconstructed_147 ENDP
; mmioStringToFOURCCW @ 148; original VA 0x180003871
; assembly ev_05935c014ac305efa4b855b9358c084a492c671c3b2e311c8a4eb0f2c989905a; pseudocode ev_d21e705ea745f638cb7f908f121a431da9f5667d28a7711eeea0671043e336d2
PUBLIC reconstructed_148
reconstructed_148 PROC FRAME
L_180003871:
    PUSH R14
    .pushreg R14
L_180003873:
    PUSH RSI
    .pushreg RSI
L_180003874:
    PUSH RDI
    .pushreg RDI
L_180003875:
    PUSH RBX
    .pushreg RBX
L_180003876:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000387a:
    MOV R14, RCX
L_18000387d:
    MOV RCX, qword ptr [OuterState+120]
L_180003884:
    TEST RCX, RCX
L_180003887:
    JZ L_1800038bc
L_180003889:
    MOV RSI, R9
L_18000388c:
    MOV RDI, R8
L_18000388f:
    MOV RBX, RDX
L_180003892:
    LEA RDX, [name_148]
L_180003899:
    CALL qword ptr [__imp_GetProcAddress]
L_18000389f:
    TEST RAX, RAX
L_1800038a2:
    JZ L_1800038bc
L_1800038a4:
    MOV RCX, R14
L_1800038a7:
    MOV RDX, RBX
L_1800038aa:
    MOV R8, RDI
L_1800038ad:
    MOV R9, RSI
L_1800038b0:
    ADD RSP, 028h
L_1800038b4:
    POP RBX
L_1800038b5:
    POP RDI
L_1800038b6:
    POP RSI
L_1800038b7:
    POP R14
L_1800038b9:
    JMP RAX
L_1800038bc:
    XOR EAX, EAX
L_1800038be:
    ADD RSP, 028h
L_1800038c2:
    POP RBX
L_1800038c3:
    POP RDI
L_1800038c4:
    POP RSI
L_1800038c5:
    POP R14
L_1800038c7:
    RET
reconstructed_148 ENDP
; mmioWrite @ 149; original VA 0x1800038c8
; assembly ev_0721ef45d46b7d42a8cc63520b4651d86ea5b2ad4d84f6619c7c872467df8788; pseudocode ev_5166082cb67386b122fac0a1e761eb306ba8b1b23282b37e715b6113113d6825
PUBLIC reconstructed_149
reconstructed_149 PROC FRAME
L_1800038c8:
    PUSH R14
    .pushreg R14
L_1800038ca:
    PUSH RSI
    .pushreg RSI
L_1800038cb:
    PUSH RDI
    .pushreg RDI
L_1800038cc:
    PUSH RBX
    .pushreg RBX
L_1800038cd:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800038d1:
    MOV R14, RCX
L_1800038d4:
    MOV RCX, qword ptr [OuterState+120]
L_1800038db:
    TEST RCX, RCX
L_1800038de:
    JZ L_180003913
L_1800038e0:
    MOV RSI, R9
L_1800038e3:
    MOV RDI, R8
L_1800038e6:
    MOV RBX, RDX
L_1800038e9:
    LEA RDX, [name_149]
L_1800038f0:
    CALL qword ptr [__imp_GetProcAddress]
L_1800038f6:
    TEST RAX, RAX
L_1800038f9:
    JZ L_180003913
L_1800038fb:
    MOV RCX, R14
L_1800038fe:
    MOV RDX, RBX
L_180003901:
    MOV R8, RDI
L_180003904:
    MOV R9, RSI
L_180003907:
    ADD RSP, 028h
L_18000390b:
    POP RBX
L_18000390c:
    POP RDI
L_18000390d:
    POP RSI
L_18000390e:
    POP R14
L_180003910:
    JMP RAX
L_180003913:
    XOR EAX, EAX
L_180003915:
    ADD RSP, 028h
L_180003919:
    POP RBX
L_18000391a:
    POP RDI
L_18000391b:
    POP RSI
L_18000391c:
    POP R14
L_18000391e:
    RET
reconstructed_149 ENDP
; mmsystemGetVersion @ 150; original VA 0x18000391f
; assembly ev_138db8b5429101c411f0d5ab5025992a3961a831daf6400fc0a36ad15653baeb; pseudocode ev_d2b948e92b6907e196c3fcba667af7935ee975c6e3d5ac214bc4a8d4a1d220e9
PUBLIC reconstructed_150
reconstructed_150 PROC FRAME
L_18000391f:
    PUSH R14
    .pushreg R14
L_180003921:
    PUSH RSI
    .pushreg RSI
L_180003922:
    PUSH RDI
    .pushreg RDI
L_180003923:
    PUSH RBX
    .pushreg RBX
L_180003924:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003928:
    MOV R14, RCX
L_18000392b:
    MOV RCX, qword ptr [OuterState+120]
L_180003932:
    TEST RCX, RCX
L_180003935:
    JZ L_18000396a
L_180003937:
    MOV RSI, R9
L_18000393a:
    MOV RDI, R8
L_18000393d:
    MOV RBX, RDX
L_180003940:
    LEA RDX, [name_150]
L_180003947:
    CALL qword ptr [__imp_GetProcAddress]
L_18000394d:
    TEST RAX, RAX
L_180003950:
    JZ L_18000396a
L_180003952:
    MOV RCX, R14
L_180003955:
    MOV RDX, RBX
L_180003958:
    MOV R8, RDI
L_18000395b:
    MOV R9, RSI
L_18000395e:
    ADD RSP, 028h
L_180003962:
    POP RBX
L_180003963:
    POP RDI
L_180003964:
    POP RSI
L_180003965:
    POP R14
L_180003967:
    JMP RAX
L_18000396a:
    XOR EAX, EAX
L_18000396c:
    ADD RSP, 028h
L_180003970:
    POP RBX
L_180003971:
    POP RDI
L_180003972:
    POP RSI
L_180003973:
    POP R14
L_180003975:
    RET
reconstructed_150 ENDP
; sndPlaySoundA @ 151; original VA 0x180003c85
; assembly ev_1d85ba9d40a34cb852b6938c1be3719c1523d5202d4f06f948d6f2d10a358d47; pseudocode ev_043d6285ae74266f3ef542452f34e00663e1ca9b0022563d6b18bd27abc97b8c
PUBLIC reconstructed_151
reconstructed_151 PROC FRAME
L_180003c85:
    PUSH R14
    .pushreg R14
L_180003c87:
    PUSH RSI
    .pushreg RSI
L_180003c88:
    PUSH RDI
    .pushreg RDI
L_180003c89:
    PUSH RBX
    .pushreg RBX
L_180003c8a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003c8e:
    MOV R14, RCX
L_180003c91:
    MOV RCX, qword ptr [OuterState+120]
L_180003c98:
    TEST RCX, RCX
L_180003c9b:
    JZ L_180003cd0
L_180003c9d:
    MOV RSI, R9
L_180003ca0:
    MOV RDI, R8
L_180003ca3:
    MOV RBX, RDX
L_180003ca6:
    LEA RDX, [name_151]
L_180003cad:
    CALL qword ptr [__imp_GetProcAddress]
L_180003cb3:
    TEST RAX, RAX
L_180003cb6:
    JZ L_180003cd0
L_180003cb8:
    MOV RCX, R14
L_180003cbb:
    MOV RDX, RBX
L_180003cbe:
    MOV R8, RDI
L_180003cc1:
    MOV R9, RSI
L_180003cc4:
    ADD RSP, 028h
L_180003cc8:
    POP RBX
L_180003cc9:
    POP RDI
L_180003cca:
    POP RSI
L_180003ccb:
    POP R14
L_180003ccd:
    JMP RAX
L_180003cd0:
    XOR EAX, EAX
L_180003cd2:
    ADD RSP, 028h
L_180003cd6:
    POP RBX
L_180003cd7:
    POP RDI
L_180003cd8:
    POP RSI
L_180003cd9:
    POP R14
L_180003cdb:
    RET
reconstructed_151 ENDP
; sndPlaySoundW @ 152; original VA 0x180003cdc
; assembly ev_d26dc03664fa97c644720c71bd05e3f988dcbca6c2a555a01b42ad9b9551276e; pseudocode ev_50cbe1ac782477b6894977c61441b52b934f55f9a1f96c726ea590b428deedea
PUBLIC reconstructed_152
reconstructed_152 PROC FRAME
L_180003cdc:
    PUSH R14
    .pushreg R14
L_180003cde:
    PUSH RSI
    .pushreg RSI
L_180003cdf:
    PUSH RDI
    .pushreg RDI
L_180003ce0:
    PUSH RBX
    .pushreg RBX
L_180003ce1:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003ce5:
    MOV R14, RCX
L_180003ce8:
    MOV RCX, qword ptr [OuterState+120]
L_180003cef:
    TEST RCX, RCX
L_180003cf2:
    JZ L_180003d27
L_180003cf4:
    MOV RSI, R9
L_180003cf7:
    MOV RDI, R8
L_180003cfa:
    MOV RBX, RDX
L_180003cfd:
    LEA RDX, [name_152]
L_180003d04:
    CALL qword ptr [__imp_GetProcAddress]
L_180003d0a:
    TEST RAX, RAX
L_180003d0d:
    JZ L_180003d27
L_180003d0f:
    MOV RCX, R14
L_180003d12:
    MOV RDX, RBX
L_180003d15:
    MOV R8, RDI
L_180003d18:
    MOV R9, RSI
L_180003d1b:
    ADD RSP, 028h
L_180003d1f:
    POP RBX
L_180003d20:
    POP RDI
L_180003d21:
    POP RSI
L_180003d22:
    POP R14
L_180003d24:
    JMP RAX
L_180003d27:
    XOR EAX, EAX
L_180003d29:
    ADD RSP, 028h
L_180003d2d:
    POP RBX
L_180003d2e:
    POP RDI
L_180003d2f:
    POP RSI
L_180003d30:
    POP R14
L_180003d32:
    RET
reconstructed_152 ENDP
; timeBeginPeriod @ 153; original VA 0x180003d33
; assembly ev_b2a4593ac3a1da6a04935e7d4c68ca8df9f08073517e1c14d82a55449174ce53; pseudocode ev_295656339a7331c412f19ecf0ab801bcf5c10c2cc7a5f02172184cadb0122605
PUBLIC reconstructed_153
reconstructed_153 PROC FRAME
L_180003d33:
    PUSH R14
    .pushreg R14
L_180003d35:
    PUSH RSI
    .pushreg RSI
L_180003d36:
    PUSH RDI
    .pushreg RDI
L_180003d37:
    PUSH RBX
    .pushreg RBX
L_180003d38:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003d3c:
    MOV R14, RCX
L_180003d3f:
    MOV RCX, qword ptr [OuterState+120]
L_180003d46:
    TEST RCX, RCX
L_180003d49:
    JZ L_180003d7e
L_180003d4b:
    MOV RSI, R9
L_180003d4e:
    MOV RDI, R8
L_180003d51:
    MOV RBX, RDX
L_180003d54:
    LEA RDX, [name_153]
L_180003d5b:
    CALL qword ptr [__imp_GetProcAddress]
L_180003d61:
    TEST RAX, RAX
L_180003d64:
    JZ L_180003d7e
L_180003d66:
    MOV RCX, R14
L_180003d69:
    MOV RDX, RBX
L_180003d6c:
    MOV R8, RDI
L_180003d6f:
    MOV R9, RSI
L_180003d72:
    ADD RSP, 028h
L_180003d76:
    POP RBX
L_180003d77:
    POP RDI
L_180003d78:
    POP RSI
L_180003d79:
    POP R14
L_180003d7b:
    JMP RAX
L_180003d7e:
    XOR EAX, EAX
L_180003d80:
    ADD RSP, 028h
L_180003d84:
    POP RBX
L_180003d85:
    POP RDI
L_180003d86:
    POP RSI
L_180003d87:
    POP R14
L_180003d89:
    RET
reconstructed_153 ENDP
; timeEndPeriod @ 154; original VA 0x180003d8a
; assembly ev_7ddff4c8557e676cdf39ba0b3e821dc44e2386c6940ac7ff2101b54262d64278; pseudocode ev_6dbc5c95f0f6090a15977e17ee525077ebeef26983bad2d1e30e3f5ec7263229
PUBLIC reconstructed_154
reconstructed_154 PROC FRAME
L_180003d8a:
    PUSH R14
    .pushreg R14
L_180003d8c:
    PUSH RSI
    .pushreg RSI
L_180003d8d:
    PUSH RDI
    .pushreg RDI
L_180003d8e:
    PUSH RBX
    .pushreg RBX
L_180003d8f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003d93:
    MOV R14, RCX
L_180003d96:
    MOV RCX, qword ptr [OuterState+120]
L_180003d9d:
    TEST RCX, RCX
L_180003da0:
    JZ L_180003dd5
L_180003da2:
    MOV RSI, R9
L_180003da5:
    MOV RDI, R8
L_180003da8:
    MOV RBX, RDX
L_180003dab:
    LEA RDX, [name_154]
L_180003db2:
    CALL qword ptr [__imp_GetProcAddress]
L_180003db8:
    TEST RAX, RAX
L_180003dbb:
    JZ L_180003dd5
L_180003dbd:
    MOV RCX, R14
L_180003dc0:
    MOV RDX, RBX
L_180003dc3:
    MOV R8, RDI
L_180003dc6:
    MOV R9, RSI
L_180003dc9:
    ADD RSP, 028h
L_180003dcd:
    POP RBX
L_180003dce:
    POP RDI
L_180003dcf:
    POP RSI
L_180003dd0:
    POP R14
L_180003dd2:
    JMP RAX
L_180003dd5:
    XOR EAX, EAX
L_180003dd7:
    ADD RSP, 028h
L_180003ddb:
    POP RBX
L_180003ddc:
    POP RDI
L_180003ddd:
    POP RSI
L_180003dde:
    POP R14
L_180003de0:
    RET
reconstructed_154 ENDP
; timeGetDevCaps @ 155; original VA 0x180003de1
; assembly ev_0c95528373ea8604336cd746fd72d865f636093584ac1064b95d1bc3384b2800; pseudocode ev_6ca6612c65ec9a0274c1af73623f7217c415d24abd3a8a2b2dba0d372bbae202
PUBLIC reconstructed_155
reconstructed_155 PROC FRAME
L_180003de1:
    PUSH R14
    .pushreg R14
L_180003de3:
    PUSH RSI
    .pushreg RSI
L_180003de4:
    PUSH RDI
    .pushreg RDI
L_180003de5:
    PUSH RBX
    .pushreg RBX
L_180003de6:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003dea:
    MOV R14, RCX
L_180003ded:
    MOV RCX, qword ptr [OuterState+120]
L_180003df4:
    TEST RCX, RCX
L_180003df7:
    JZ L_180003e2c
L_180003df9:
    MOV RSI, R9
L_180003dfc:
    MOV RDI, R8
L_180003dff:
    MOV RBX, RDX
L_180003e02:
    LEA RDX, [name_155]
L_180003e09:
    CALL qword ptr [__imp_GetProcAddress]
L_180003e0f:
    TEST RAX, RAX
L_180003e12:
    JZ L_180003e2c
L_180003e14:
    MOV RCX, R14
L_180003e17:
    MOV RDX, RBX
L_180003e1a:
    MOV R8, RDI
L_180003e1d:
    MOV R9, RSI
L_180003e20:
    ADD RSP, 028h
L_180003e24:
    POP RBX
L_180003e25:
    POP RDI
L_180003e26:
    POP RSI
L_180003e27:
    POP R14
L_180003e29:
    JMP RAX
L_180003e2c:
    XOR EAX, EAX
L_180003e2e:
    ADD RSP, 028h
L_180003e32:
    POP RBX
L_180003e33:
    POP RDI
L_180003e34:
    POP RSI
L_180003e35:
    POP R14
L_180003e37:
    RET
reconstructed_155 ENDP
; timeGetSystemTime @ 156; original VA 0x180003e38
; assembly ev_e0d3aca7f7fb6b740efbc714278c1dc6a4e0ca411f0b6521763428ec54db802c; pseudocode ev_53d33349af8341ffa105757eafd118e9a696774132514303b94ffe2a6879d787
PUBLIC reconstructed_156
reconstructed_156 PROC FRAME
L_180003e38:
    PUSH R14
    .pushreg R14
L_180003e3a:
    PUSH RSI
    .pushreg RSI
L_180003e3b:
    PUSH RDI
    .pushreg RDI
L_180003e3c:
    PUSH RBX
    .pushreg RBX
L_180003e3d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003e41:
    MOV R14, RCX
L_180003e44:
    MOV RCX, qword ptr [OuterState+120]
L_180003e4b:
    TEST RCX, RCX
L_180003e4e:
    JZ L_180003e83
L_180003e50:
    MOV RSI, R9
L_180003e53:
    MOV RDI, R8
L_180003e56:
    MOV RBX, RDX
L_180003e59:
    LEA RDX, [name_156]
L_180003e60:
    CALL qword ptr [__imp_GetProcAddress]
L_180003e66:
    TEST RAX, RAX
L_180003e69:
    JZ L_180003e83
L_180003e6b:
    MOV RCX, R14
L_180003e6e:
    MOV RDX, RBX
L_180003e71:
    MOV R8, RDI
L_180003e74:
    MOV R9, RSI
L_180003e77:
    ADD RSP, 028h
L_180003e7b:
    POP RBX
L_180003e7c:
    POP RDI
L_180003e7d:
    POP RSI
L_180003e7e:
    POP R14
L_180003e80:
    JMP RAX
L_180003e83:
    XOR EAX, EAX
L_180003e85:
    ADD RSP, 028h
L_180003e89:
    POP RBX
L_180003e8a:
    POP RDI
L_180003e8b:
    POP RSI
L_180003e8c:
    POP R14
L_180003e8e:
    RET
reconstructed_156 ENDP
; timeGetTime @ 157; original VA 0x180003e8f
; assembly ev_d62ea3c7f609ad4d7ec019cd5c246808ccf1fbf2834e452ba5c925f62c991d3a; pseudocode ev_cdbad73c74e2ee5be24b98978cf2c4273fc646daadf911f1ba7db86600a32412
PUBLIC reconstructed_157
reconstructed_157 PROC FRAME
L_180003e8f:
    PUSH R14
    .pushreg R14
L_180003e91:
    PUSH RSI
    .pushreg RSI
L_180003e92:
    PUSH RDI
    .pushreg RDI
L_180003e93:
    PUSH RBX
    .pushreg RBX
L_180003e94:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003e98:
    MOV R14, RCX
L_180003e9b:
    MOV RCX, qword ptr [OuterState+120]
L_180003ea2:
    TEST RCX, RCX
L_180003ea5:
    JZ L_180003eda
L_180003ea7:
    MOV RSI, R9
L_180003eaa:
    MOV RDI, R8
L_180003ead:
    MOV RBX, RDX
L_180003eb0:
    LEA RDX, [name_157]
L_180003eb7:
    CALL qword ptr [__imp_GetProcAddress]
L_180003ebd:
    TEST RAX, RAX
L_180003ec0:
    JZ L_180003eda
L_180003ec2:
    MOV RCX, R14
L_180003ec5:
    MOV RDX, RBX
L_180003ec8:
    MOV R8, RDI
L_180003ecb:
    MOV R9, RSI
L_180003ece:
    ADD RSP, 028h
L_180003ed2:
    POP RBX
L_180003ed3:
    POP RDI
L_180003ed4:
    POP RSI
L_180003ed5:
    POP R14
L_180003ed7:
    JMP RAX
L_180003eda:
    XOR EAX, EAX
L_180003edc:
    ADD RSP, 028h
L_180003ee0:
    POP RBX
L_180003ee1:
    POP RDI
L_180003ee2:
    POP RSI
L_180003ee3:
    POP R14
L_180003ee5:
    RET
reconstructed_157 ENDP
; timeKillEvent @ 158; original VA 0x180003ee6
; assembly ev_d6bbc52f38f38faa8d4a9d302631c5e2b16a4d6ab3c574cbcbcad39ec1660115; pseudocode ev_22b4ad3d693647cb4f6c749e7b46b7c34c26d377e6214c457f502a9c418623ae
PUBLIC reconstructed_158
reconstructed_158 PROC FRAME
L_180003ee6:
    PUSH R14
    .pushreg R14
L_180003ee8:
    PUSH RSI
    .pushreg RSI
L_180003ee9:
    PUSH RDI
    .pushreg RDI
L_180003eea:
    PUSH RBX
    .pushreg RBX
L_180003eeb:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003eef:
    MOV R14, RCX
L_180003ef2:
    MOV RCX, qword ptr [OuterState+120]
L_180003ef9:
    TEST RCX, RCX
L_180003efc:
    JZ L_180003f31
L_180003efe:
    MOV RSI, R9
L_180003f01:
    MOV RDI, R8
L_180003f04:
    MOV RBX, RDX
L_180003f07:
    LEA RDX, [name_158]
L_180003f0e:
    CALL qword ptr [__imp_GetProcAddress]
L_180003f14:
    TEST RAX, RAX
L_180003f17:
    JZ L_180003f31
L_180003f19:
    MOV RCX, R14
L_180003f1c:
    MOV RDX, RBX
L_180003f1f:
    MOV R8, RDI
L_180003f22:
    MOV R9, RSI
L_180003f25:
    ADD RSP, 028h
L_180003f29:
    POP RBX
L_180003f2a:
    POP RDI
L_180003f2b:
    POP RSI
L_180003f2c:
    POP R14
L_180003f2e:
    JMP RAX
L_180003f31:
    XOR EAX, EAX
L_180003f33:
    ADD RSP, 028h
L_180003f37:
    POP RBX
L_180003f38:
    POP RDI
L_180003f39:
    POP RSI
L_180003f3a:
    POP R14
L_180003f3c:
    RET
reconstructed_158 ENDP
; timeSetEvent @ 159; original VA 0x180003f3d
; assembly ev_b89633c85964ded7360efaf24c6c6d08d7bb2ef2300a99803bbf8366654b4cd2; pseudocode ev_019c1b5d6a989ec67d6d4c164e9221fa7c44390f2c4ae110b0f3a4f8f664c6f2
PUBLIC reconstructed_159
reconstructed_159 PROC FRAME
L_180003f3d:
    PUSH R14
    .pushreg R14
L_180003f3f:
    PUSH RSI
    .pushreg RSI
L_180003f40:
    PUSH RDI
    .pushreg RDI
L_180003f41:
    PUSH RBX
    .pushreg RBX
L_180003f42:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003f46:
    MOV R14, RCX
L_180003f49:
    MOV RCX, qword ptr [OuterState+120]
L_180003f50:
    TEST RCX, RCX
L_180003f53:
    JZ L_180003f88
L_180003f55:
    MOV RSI, R9
L_180003f58:
    MOV RDI, R8
L_180003f5b:
    MOV RBX, RDX
L_180003f5e:
    LEA RDX, [name_159]
L_180003f65:
    CALL qword ptr [__imp_GetProcAddress]
L_180003f6b:
    TEST RAX, RAX
L_180003f6e:
    JZ L_180003f88
L_180003f70:
    MOV RCX, R14
L_180003f73:
    MOV RDX, RBX
L_180003f76:
    MOV R8, RDI
L_180003f79:
    MOV R9, RSI
L_180003f7c:
    ADD RSP, 028h
L_180003f80:
    POP RBX
L_180003f81:
    POP RDI
L_180003f82:
    POP RSI
L_180003f83:
    POP R14
L_180003f85:
    JMP RAX
L_180003f88:
    XOR EAX, EAX
L_180003f8a:
    ADD RSP, 028h
L_180003f8e:
    POP RBX
L_180003f8f:
    POP RDI
L_180003f90:
    POP RSI
L_180003f91:
    POP R14
L_180003f93:
    RET
reconstructed_159 ENDP
; waveInAddBuffer @ 160; original VA 0x180003f94
; assembly ev_1b5285f7644bc1cdec19e24ce2a697166bb8c297dc8c30df0d05075167d92e89; pseudocode ev_1b089da122d61a553688b0ebf6881ac818ea62691bd116aa9ebae87ae5e91d2f
PUBLIC reconstructed_160
reconstructed_160 PROC FRAME
L_180003f94:
    PUSH R14
    .pushreg R14
L_180003f96:
    PUSH RSI
    .pushreg RSI
L_180003f97:
    PUSH RDI
    .pushreg RDI
L_180003f98:
    PUSH RBX
    .pushreg RBX
L_180003f99:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003f9d:
    MOV R14, RCX
L_180003fa0:
    MOV RCX, qword ptr [OuterState+120]
L_180003fa7:
    TEST RCX, RCX
L_180003faa:
    JZ L_180003fdf
L_180003fac:
    MOV RSI, R9
L_180003faf:
    MOV RDI, R8
L_180003fb2:
    MOV RBX, RDX
L_180003fb5:
    LEA RDX, [name_160]
L_180003fbc:
    CALL qword ptr [__imp_GetProcAddress]
L_180003fc2:
    TEST RAX, RAX
L_180003fc5:
    JZ L_180003fdf
L_180003fc7:
    MOV RCX, R14
L_180003fca:
    MOV RDX, RBX
L_180003fcd:
    MOV R8, RDI
L_180003fd0:
    MOV R9, RSI
L_180003fd3:
    ADD RSP, 028h
L_180003fd7:
    POP RBX
L_180003fd8:
    POP RDI
L_180003fd9:
    POP RSI
L_180003fda:
    POP R14
L_180003fdc:
    JMP RAX
L_180003fdf:
    XOR EAX, EAX
L_180003fe1:
    ADD RSP, 028h
L_180003fe5:
    POP RBX
L_180003fe6:
    POP RDI
L_180003fe7:
    POP RSI
L_180003fe8:
    POP R14
L_180003fea:
    RET
reconstructed_160 ENDP
; waveInClose @ 161; original VA 0x180003feb
; assembly ev_a5f714021f3575624bbb309364a56b1ddb627c2c960e2000fb9fe60a2a5500c6; pseudocode ev_4de96ec30fa0a053a1685065b9bd991b992e365dcf2ba4fd2c1dcee2b0e29689
PUBLIC reconstructed_161
reconstructed_161 PROC FRAME
L_180003feb:
    PUSH R14
    .pushreg R14
L_180003fed:
    PUSH RSI
    .pushreg RSI
L_180003fee:
    PUSH RDI
    .pushreg RDI
L_180003fef:
    PUSH RBX
    .pushreg RBX
L_180003ff0:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180003ff4:
    MOV R14, RCX
L_180003ff7:
    MOV RCX, qword ptr [OuterState+120]
L_180003ffe:
    TEST RCX, RCX
L_180004001:
    JZ L_180004036
L_180004003:
    MOV RSI, R9
L_180004006:
    MOV RDI, R8
L_180004009:
    MOV RBX, RDX
L_18000400c:
    LEA RDX, [name_161]
L_180004013:
    CALL qword ptr [__imp_GetProcAddress]
L_180004019:
    TEST RAX, RAX
L_18000401c:
    JZ L_180004036
L_18000401e:
    MOV RCX, R14
L_180004021:
    MOV RDX, RBX
L_180004024:
    MOV R8, RDI
L_180004027:
    MOV R9, RSI
L_18000402a:
    ADD RSP, 028h
L_18000402e:
    POP RBX
L_18000402f:
    POP RDI
L_180004030:
    POP RSI
L_180004031:
    POP R14
L_180004033:
    JMP RAX
L_180004036:
    XOR EAX, EAX
L_180004038:
    ADD RSP, 028h
L_18000403c:
    POP RBX
L_18000403d:
    POP RDI
L_18000403e:
    POP RSI
L_18000403f:
    POP R14
L_180004041:
    RET
reconstructed_161 ENDP
; waveInGetDevCapsA @ 162; original VA 0x180004042
; assembly ev_3ea3e7a8a2d9ac8db6d17cb636858bc3a283aedb11e500ce1cdfaa6fdf4dcd56; pseudocode ev_f3f58ff87d54a52bc3f5f81b4d98b1d8c8b34239a943ca0129d023f8883edc9e
PUBLIC reconstructed_162
reconstructed_162 PROC FRAME
L_180004042:
    PUSH R14
    .pushreg R14
L_180004044:
    PUSH RSI
    .pushreg RSI
L_180004045:
    PUSH RDI
    .pushreg RDI
L_180004046:
    PUSH RBX
    .pushreg RBX
L_180004047:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000404b:
    MOV R14, RCX
L_18000404e:
    MOV RCX, qword ptr [OuterState+120]
L_180004055:
    TEST RCX, RCX
L_180004058:
    JZ L_18000408d
L_18000405a:
    MOV RSI, R9
L_18000405d:
    MOV RDI, R8
L_180004060:
    MOV RBX, RDX
L_180004063:
    LEA RDX, [name_162]
L_18000406a:
    CALL qword ptr [__imp_GetProcAddress]
L_180004070:
    TEST RAX, RAX
L_180004073:
    JZ L_18000408d
L_180004075:
    MOV RCX, R14
L_180004078:
    MOV RDX, RBX
L_18000407b:
    MOV R8, RDI
L_18000407e:
    MOV R9, RSI
L_180004081:
    ADD RSP, 028h
L_180004085:
    POP RBX
L_180004086:
    POP RDI
L_180004087:
    POP RSI
L_180004088:
    POP R14
L_18000408a:
    JMP RAX
L_18000408d:
    XOR EAX, EAX
L_18000408f:
    ADD RSP, 028h
L_180004093:
    POP RBX
L_180004094:
    POP RDI
L_180004095:
    POP RSI
L_180004096:
    POP R14
L_180004098:
    RET
reconstructed_162 ENDP
; waveInGetDevCapsW @ 163; original VA 0x180004099
; assembly ev_26a1159cb017ab194dde07b630144725521c1713abb98b1d233aef314b1ef5fe; pseudocode ev_2dc3d30949ff4677ada300321969af6e197944e956129f99d66d1e748cce5961
PUBLIC reconstructed_163
reconstructed_163 PROC FRAME
L_180004099:
    PUSH R14
    .pushreg R14
L_18000409b:
    PUSH RSI
    .pushreg RSI
L_18000409c:
    PUSH RDI
    .pushreg RDI
L_18000409d:
    PUSH RBX
    .pushreg RBX
L_18000409e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800040a2:
    MOV R14, RCX
L_1800040a5:
    MOV RCX, qword ptr [OuterState+120]
L_1800040ac:
    TEST RCX, RCX
L_1800040af:
    JZ L_1800040e4
L_1800040b1:
    MOV RSI, R9
L_1800040b4:
    MOV RDI, R8
L_1800040b7:
    MOV RBX, RDX
L_1800040ba:
    LEA RDX, [name_163]
L_1800040c1:
    CALL qword ptr [__imp_GetProcAddress]
L_1800040c7:
    TEST RAX, RAX
L_1800040ca:
    JZ L_1800040e4
L_1800040cc:
    MOV RCX, R14
L_1800040cf:
    MOV RDX, RBX
L_1800040d2:
    MOV R8, RDI
L_1800040d5:
    MOV R9, RSI
L_1800040d8:
    ADD RSP, 028h
L_1800040dc:
    POP RBX
L_1800040dd:
    POP RDI
L_1800040de:
    POP RSI
L_1800040df:
    POP R14
L_1800040e1:
    JMP RAX
L_1800040e4:
    XOR EAX, EAX
L_1800040e6:
    ADD RSP, 028h
L_1800040ea:
    POP RBX
L_1800040eb:
    POP RDI
L_1800040ec:
    POP RSI
L_1800040ed:
    POP R14
L_1800040ef:
    RET
reconstructed_163 ENDP
; waveInGetErrorTextA @ 164; original VA 0x1800040f0
; assembly ev_ecd598807e082f0ee56041693d8d6ceee05082cdd1d81aaf81daf6545a533686; pseudocode ev_601b60e21938398c857ed101e3b611636e1bbf883d870c5f498a27b7c4f39743
PUBLIC reconstructed_164
reconstructed_164 PROC FRAME
L_1800040f0:
    PUSH R14
    .pushreg R14
L_1800040f2:
    PUSH RSI
    .pushreg RSI
L_1800040f3:
    PUSH RDI
    .pushreg RDI
L_1800040f4:
    PUSH RBX
    .pushreg RBX
L_1800040f5:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800040f9:
    MOV R14, RCX
L_1800040fc:
    MOV RCX, qword ptr [OuterState+120]
L_180004103:
    TEST RCX, RCX
L_180004106:
    JZ L_18000413b
L_180004108:
    MOV RSI, R9
L_18000410b:
    MOV RDI, R8
L_18000410e:
    MOV RBX, RDX
L_180004111:
    LEA RDX, [name_164]
L_180004118:
    CALL qword ptr [__imp_GetProcAddress]
L_18000411e:
    TEST RAX, RAX
L_180004121:
    JZ L_18000413b
L_180004123:
    MOV RCX, R14
L_180004126:
    MOV RDX, RBX
L_180004129:
    MOV R8, RDI
L_18000412c:
    MOV R9, RSI
L_18000412f:
    ADD RSP, 028h
L_180004133:
    POP RBX
L_180004134:
    POP RDI
L_180004135:
    POP RSI
L_180004136:
    POP R14
L_180004138:
    JMP RAX
L_18000413b:
    XOR EAX, EAX
L_18000413d:
    ADD RSP, 028h
L_180004141:
    POP RBX
L_180004142:
    POP RDI
L_180004143:
    POP RSI
L_180004144:
    POP R14
L_180004146:
    RET
reconstructed_164 ENDP
; waveInGetErrorTextW @ 165; original VA 0x180004147
; assembly ev_9b9d21b72a7cb311eb855d6f83a040aae5c4c2224d7295601d930677c23a9eed; pseudocode ev_3c666e70e02cba3d97d0eca755fcc50efef687d904c46b9c56a5465b9f3e94d2
PUBLIC reconstructed_165
reconstructed_165 PROC FRAME
L_180004147:
    PUSH R14
    .pushreg R14
L_180004149:
    PUSH RSI
    .pushreg RSI
L_18000414a:
    PUSH RDI
    .pushreg RDI
L_18000414b:
    PUSH RBX
    .pushreg RBX
L_18000414c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004150:
    MOV R14, RCX
L_180004153:
    MOV RCX, qword ptr [OuterState+120]
L_18000415a:
    TEST RCX, RCX
L_18000415d:
    JZ L_180004192
L_18000415f:
    MOV RSI, R9
L_180004162:
    MOV RDI, R8
L_180004165:
    MOV RBX, RDX
L_180004168:
    LEA RDX, [name_165]
L_18000416f:
    CALL qword ptr [__imp_GetProcAddress]
L_180004175:
    TEST RAX, RAX
L_180004178:
    JZ L_180004192
L_18000417a:
    MOV RCX, R14
L_18000417d:
    MOV RDX, RBX
L_180004180:
    MOV R8, RDI
L_180004183:
    MOV R9, RSI
L_180004186:
    ADD RSP, 028h
L_18000418a:
    POP RBX
L_18000418b:
    POP RDI
L_18000418c:
    POP RSI
L_18000418d:
    POP R14
L_18000418f:
    JMP RAX
L_180004192:
    XOR EAX, EAX
L_180004194:
    ADD RSP, 028h
L_180004198:
    POP RBX
L_180004199:
    POP RDI
L_18000419a:
    POP RSI
L_18000419b:
    POP R14
L_18000419d:
    RET
reconstructed_165 ENDP
; waveInGetID @ 166; original VA 0x18000419e
; assembly ev_ba62b6724296a88ef32c5fadaa69204c0158a2c9bfc6f06deb493281a138071e; pseudocode ev_cc8b4e266d3f8596929327c92f8e2b4861152cdb41547653d0c3a1fd14f864eb
PUBLIC reconstructed_166
reconstructed_166 PROC FRAME
L_18000419e:
    PUSH R14
    .pushreg R14
L_1800041a0:
    PUSH RSI
    .pushreg RSI
L_1800041a1:
    PUSH RDI
    .pushreg RDI
L_1800041a2:
    PUSH RBX
    .pushreg RBX
L_1800041a3:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800041a7:
    MOV R14, RCX
L_1800041aa:
    MOV RCX, qword ptr [OuterState+120]
L_1800041b1:
    TEST RCX, RCX
L_1800041b4:
    JZ L_1800041e9
L_1800041b6:
    MOV RSI, R9
L_1800041b9:
    MOV RDI, R8
L_1800041bc:
    MOV RBX, RDX
L_1800041bf:
    LEA RDX, [name_166]
L_1800041c6:
    CALL qword ptr [__imp_GetProcAddress]
L_1800041cc:
    TEST RAX, RAX
L_1800041cf:
    JZ L_1800041e9
L_1800041d1:
    MOV RCX, R14
L_1800041d4:
    MOV RDX, RBX
L_1800041d7:
    MOV R8, RDI
L_1800041da:
    MOV R9, RSI
L_1800041dd:
    ADD RSP, 028h
L_1800041e1:
    POP RBX
L_1800041e2:
    POP RDI
L_1800041e3:
    POP RSI
L_1800041e4:
    POP R14
L_1800041e6:
    JMP RAX
L_1800041e9:
    XOR EAX, EAX
L_1800041eb:
    ADD RSP, 028h
L_1800041ef:
    POP RBX
L_1800041f0:
    POP RDI
L_1800041f1:
    POP RSI
L_1800041f2:
    POP R14
L_1800041f4:
    RET
reconstructed_166 ENDP
; waveInGetNumDevs @ 167; original VA 0x1800041f5
; assembly ev_1d5ad4ee05b7ea6ab9666f0ce1bcebc628563b8bdce712c3861908613a01b83e; pseudocode ev_6aae882089c291eddac4f58fc9a7c6c2c6241693431492d06cfb0d45f4978cd4
PUBLIC reconstructed_167
reconstructed_167 PROC FRAME
L_1800041f5:
    PUSH R14
    .pushreg R14
L_1800041f7:
    PUSH RSI
    .pushreg RSI
L_1800041f8:
    PUSH RDI
    .pushreg RDI
L_1800041f9:
    PUSH RBX
    .pushreg RBX
L_1800041fa:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800041fe:
    MOV R14, RCX
L_180004201:
    MOV RCX, qword ptr [OuterState+120]
L_180004208:
    TEST RCX, RCX
L_18000420b:
    JZ L_180004240
L_18000420d:
    MOV RSI, R9
L_180004210:
    MOV RDI, R8
L_180004213:
    MOV RBX, RDX
L_180004216:
    LEA RDX, [name_167]
L_18000421d:
    CALL qword ptr [__imp_GetProcAddress]
L_180004223:
    TEST RAX, RAX
L_180004226:
    JZ L_180004240
L_180004228:
    MOV RCX, R14
L_18000422b:
    MOV RDX, RBX
L_18000422e:
    MOV R8, RDI
L_180004231:
    MOV R9, RSI
L_180004234:
    ADD RSP, 028h
L_180004238:
    POP RBX
L_180004239:
    POP RDI
L_18000423a:
    POP RSI
L_18000423b:
    POP R14
L_18000423d:
    JMP RAX
L_180004240:
    XOR EAX, EAX
L_180004242:
    ADD RSP, 028h
L_180004246:
    POP RBX
L_180004247:
    POP RDI
L_180004248:
    POP RSI
L_180004249:
    POP R14
L_18000424b:
    RET
reconstructed_167 ENDP
; waveInGetPosition @ 168; original VA 0x18000424c
; assembly ev_7e8199c5486ddce8d3676fdf74a6b8662ca28787c773927665d3d6b2b11d30a7; pseudocode ev_dccb5aafbd6fd38c36d52b1e1abaeda55bec6795e516c634f461c1a859b297a4
PUBLIC reconstructed_168
reconstructed_168 PROC FRAME
L_18000424c:
    PUSH R14
    .pushreg R14
L_18000424e:
    PUSH RSI
    .pushreg RSI
L_18000424f:
    PUSH RDI
    .pushreg RDI
L_180004250:
    PUSH RBX
    .pushreg RBX
L_180004251:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004255:
    MOV R14, RCX
L_180004258:
    MOV RCX, qword ptr [OuterState+120]
L_18000425f:
    TEST RCX, RCX
L_180004262:
    JZ L_180004297
L_180004264:
    MOV RSI, R9
L_180004267:
    MOV RDI, R8
L_18000426a:
    MOV RBX, RDX
L_18000426d:
    LEA RDX, [name_168]
L_180004274:
    CALL qword ptr [__imp_GetProcAddress]
L_18000427a:
    TEST RAX, RAX
L_18000427d:
    JZ L_180004297
L_18000427f:
    MOV RCX, R14
L_180004282:
    MOV RDX, RBX
L_180004285:
    MOV R8, RDI
L_180004288:
    MOV R9, RSI
L_18000428b:
    ADD RSP, 028h
L_18000428f:
    POP RBX
L_180004290:
    POP RDI
L_180004291:
    POP RSI
L_180004292:
    POP R14
L_180004294:
    JMP RAX
L_180004297:
    XOR EAX, EAX
L_180004299:
    ADD RSP, 028h
L_18000429d:
    POP RBX
L_18000429e:
    POP RDI
L_18000429f:
    POP RSI
L_1800042a0:
    POP R14
L_1800042a2:
    RET
reconstructed_168 ENDP
; waveInMessage @ 169; original VA 0x1800042a3
; assembly ev_89967477cd1e966531dfe77f12014d5f0c58531ed7e96e347f554df0c1989e85; pseudocode ev_d9988fd790e6f753bdade548dfbddb510b870e72ea1e8d2fa6ba773b1557fef1
PUBLIC reconstructed_169
reconstructed_169 PROC FRAME
L_1800042a3:
    PUSH R14
    .pushreg R14
L_1800042a5:
    PUSH RSI
    .pushreg RSI
L_1800042a6:
    PUSH RDI
    .pushreg RDI
L_1800042a7:
    PUSH RBX
    .pushreg RBX
L_1800042a8:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800042ac:
    MOV R14, RCX
L_1800042af:
    MOV RCX, qword ptr [OuterState+120]
L_1800042b6:
    TEST RCX, RCX
L_1800042b9:
    JZ L_1800042ee
L_1800042bb:
    MOV RSI, R9
L_1800042be:
    MOV RDI, R8
L_1800042c1:
    MOV RBX, RDX
L_1800042c4:
    LEA RDX, [name_169]
L_1800042cb:
    CALL qword ptr [__imp_GetProcAddress]
L_1800042d1:
    TEST RAX, RAX
L_1800042d4:
    JZ L_1800042ee
L_1800042d6:
    MOV RCX, R14
L_1800042d9:
    MOV RDX, RBX
L_1800042dc:
    MOV R8, RDI
L_1800042df:
    MOV R9, RSI
L_1800042e2:
    ADD RSP, 028h
L_1800042e6:
    POP RBX
L_1800042e7:
    POP RDI
L_1800042e8:
    POP RSI
L_1800042e9:
    POP R14
L_1800042eb:
    JMP RAX
L_1800042ee:
    XOR EAX, EAX
L_1800042f0:
    ADD RSP, 028h
L_1800042f4:
    POP RBX
L_1800042f5:
    POP RDI
L_1800042f6:
    POP RSI
L_1800042f7:
    POP R14
L_1800042f9:
    RET
reconstructed_169 ENDP
; waveInOpen @ 170; original VA 0x1800042fa
; assembly ev_ca57ad18545c2a298011d98d555a120b221e68e600b58ec3512832a148603653; pseudocode ev_93a9677293fc021ae1b3cbd1b347c5a70be794c9200d3657f7779be54e574c72
PUBLIC reconstructed_170
reconstructed_170 PROC FRAME
L_1800042fa:
    PUSH R14
    .pushreg R14
L_1800042fc:
    PUSH RSI
    .pushreg RSI
L_1800042fd:
    PUSH RDI
    .pushreg RDI
L_1800042fe:
    PUSH RBX
    .pushreg RBX
L_1800042ff:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004303:
    MOV R14, RCX
L_180004306:
    MOV RCX, qword ptr [OuterState+120]
L_18000430d:
    TEST RCX, RCX
L_180004310:
    JZ L_180004345
L_180004312:
    MOV RSI, R9
L_180004315:
    MOV RDI, R8
L_180004318:
    MOV RBX, RDX
L_18000431b:
    LEA RDX, [name_170]
L_180004322:
    CALL qword ptr [__imp_GetProcAddress]
L_180004328:
    TEST RAX, RAX
L_18000432b:
    JZ L_180004345
L_18000432d:
    MOV RCX, R14
L_180004330:
    MOV RDX, RBX
L_180004333:
    MOV R8, RDI
L_180004336:
    MOV R9, RSI
L_180004339:
    ADD RSP, 028h
L_18000433d:
    POP RBX
L_18000433e:
    POP RDI
L_18000433f:
    POP RSI
L_180004340:
    POP R14
L_180004342:
    JMP RAX
L_180004345:
    XOR EAX, EAX
L_180004347:
    ADD RSP, 028h
L_18000434b:
    POP RBX
L_18000434c:
    POP RDI
L_18000434d:
    POP RSI
L_18000434e:
    POP R14
L_180004350:
    RET
reconstructed_170 ENDP
; waveInPrepareHeader @ 171; original VA 0x180004351
; assembly ev_0750266cefa9d37be4f99f14acb7f01b7cee6b164d29ab074fcab804203bc4d2; pseudocode ev_731e41d1818b7fb450cc967434632b17272b9cfe8c939ad7943531751ac67dc5
PUBLIC reconstructed_171
reconstructed_171 PROC FRAME
L_180004351:
    PUSH R14
    .pushreg R14
L_180004353:
    PUSH RSI
    .pushreg RSI
L_180004354:
    PUSH RDI
    .pushreg RDI
L_180004355:
    PUSH RBX
    .pushreg RBX
L_180004356:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000435a:
    MOV R14, RCX
L_18000435d:
    MOV RCX, qword ptr [OuterState+120]
L_180004364:
    TEST RCX, RCX
L_180004367:
    JZ L_18000439c
L_180004369:
    MOV RSI, R9
L_18000436c:
    MOV RDI, R8
L_18000436f:
    MOV RBX, RDX
L_180004372:
    LEA RDX, [name_171]
L_180004379:
    CALL qword ptr [__imp_GetProcAddress]
L_18000437f:
    TEST RAX, RAX
L_180004382:
    JZ L_18000439c
L_180004384:
    MOV RCX, R14
L_180004387:
    MOV RDX, RBX
L_18000438a:
    MOV R8, RDI
L_18000438d:
    MOV R9, RSI
L_180004390:
    ADD RSP, 028h
L_180004394:
    POP RBX
L_180004395:
    POP RDI
L_180004396:
    POP RSI
L_180004397:
    POP R14
L_180004399:
    JMP RAX
L_18000439c:
    XOR EAX, EAX
L_18000439e:
    ADD RSP, 028h
L_1800043a2:
    POP RBX
L_1800043a3:
    POP RDI
L_1800043a4:
    POP RSI
L_1800043a5:
    POP R14
L_1800043a7:
    RET
reconstructed_171 ENDP
; waveInReset @ 172; original VA 0x1800043a8
; assembly ev_f67edd53f8d956c5e5f4efcc570aee7a162671106b757e52405a1a23a1eab27d; pseudocode ev_534f302ffac8acfcd518adb6536f5c04711f3919c084fecb36dd02e24974dcfd
PUBLIC reconstructed_172
reconstructed_172 PROC FRAME
L_1800043a8:
    PUSH R14
    .pushreg R14
L_1800043aa:
    PUSH RSI
    .pushreg RSI
L_1800043ab:
    PUSH RDI
    .pushreg RDI
L_1800043ac:
    PUSH RBX
    .pushreg RBX
L_1800043ad:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800043b1:
    MOV R14, RCX
L_1800043b4:
    MOV RCX, qword ptr [OuterState+120]
L_1800043bb:
    TEST RCX, RCX
L_1800043be:
    JZ L_1800043f3
L_1800043c0:
    MOV RSI, R9
L_1800043c3:
    MOV RDI, R8
L_1800043c6:
    MOV RBX, RDX
L_1800043c9:
    LEA RDX, [name_172]
L_1800043d0:
    CALL qword ptr [__imp_GetProcAddress]
L_1800043d6:
    TEST RAX, RAX
L_1800043d9:
    JZ L_1800043f3
L_1800043db:
    MOV RCX, R14
L_1800043de:
    MOV RDX, RBX
L_1800043e1:
    MOV R8, RDI
L_1800043e4:
    MOV R9, RSI
L_1800043e7:
    ADD RSP, 028h
L_1800043eb:
    POP RBX
L_1800043ec:
    POP RDI
L_1800043ed:
    POP RSI
L_1800043ee:
    POP R14
L_1800043f0:
    JMP RAX
L_1800043f3:
    XOR EAX, EAX
L_1800043f5:
    ADD RSP, 028h
L_1800043f9:
    POP RBX
L_1800043fa:
    POP RDI
L_1800043fb:
    POP RSI
L_1800043fc:
    POP R14
L_1800043fe:
    RET
reconstructed_172 ENDP
; waveInStart @ 173; original VA 0x1800043ff
; assembly ev_697e6c4c6adeacee0a7f017259ed517ed0b540588f2de3ef6fcdf39f93eecd8b; pseudocode ev_48baf07726c98780d09989858a89755d5059df3e99db9ddef77748d9a8e2dee3
PUBLIC reconstructed_173
reconstructed_173 PROC FRAME
L_1800043ff:
    PUSH R14
    .pushreg R14
L_180004401:
    PUSH RSI
    .pushreg RSI
L_180004402:
    PUSH RDI
    .pushreg RDI
L_180004403:
    PUSH RBX
    .pushreg RBX
L_180004404:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004408:
    MOV R14, RCX
L_18000440b:
    MOV RCX, qword ptr [OuterState+120]
L_180004412:
    TEST RCX, RCX
L_180004415:
    JZ L_18000444a
L_180004417:
    MOV RSI, R9
L_18000441a:
    MOV RDI, R8
L_18000441d:
    MOV RBX, RDX
L_180004420:
    LEA RDX, [name_173]
L_180004427:
    CALL qword ptr [__imp_GetProcAddress]
L_18000442d:
    TEST RAX, RAX
L_180004430:
    JZ L_18000444a
L_180004432:
    MOV RCX, R14
L_180004435:
    MOV RDX, RBX
L_180004438:
    MOV R8, RDI
L_18000443b:
    MOV R9, RSI
L_18000443e:
    ADD RSP, 028h
L_180004442:
    POP RBX
L_180004443:
    POP RDI
L_180004444:
    POP RSI
L_180004445:
    POP R14
L_180004447:
    JMP RAX
L_18000444a:
    XOR EAX, EAX
L_18000444c:
    ADD RSP, 028h
L_180004450:
    POP RBX
L_180004451:
    POP RDI
L_180004452:
    POP RSI
L_180004453:
    POP R14
L_180004455:
    RET
reconstructed_173 ENDP
; waveInStop @ 174; original VA 0x180004456
; assembly ev_889206db8eb7b0e79f87063be678b107f89ce037b0159b38b82bec3954020dee; pseudocode ev_38e94eab8ad8f6a33fe89f91945d36b6746839c14a79d883274529f1daf4976e
PUBLIC reconstructed_174
reconstructed_174 PROC FRAME
L_180004456:
    PUSH R14
    .pushreg R14
L_180004458:
    PUSH RSI
    .pushreg RSI
L_180004459:
    PUSH RDI
    .pushreg RDI
L_18000445a:
    PUSH RBX
    .pushreg RBX
L_18000445b:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000445f:
    MOV R14, RCX
L_180004462:
    MOV RCX, qword ptr [OuterState+120]
L_180004469:
    TEST RCX, RCX
L_18000446c:
    JZ L_1800044a1
L_18000446e:
    MOV RSI, R9
L_180004471:
    MOV RDI, R8
L_180004474:
    MOV RBX, RDX
L_180004477:
    LEA RDX, [name_174]
L_18000447e:
    CALL qword ptr [__imp_GetProcAddress]
L_180004484:
    TEST RAX, RAX
L_180004487:
    JZ L_1800044a1
L_180004489:
    MOV RCX, R14
L_18000448c:
    MOV RDX, RBX
L_18000448f:
    MOV R8, RDI
L_180004492:
    MOV R9, RSI
L_180004495:
    ADD RSP, 028h
L_180004499:
    POP RBX
L_18000449a:
    POP RDI
L_18000449b:
    POP RSI
L_18000449c:
    POP R14
L_18000449e:
    JMP RAX
L_1800044a1:
    XOR EAX, EAX
L_1800044a3:
    ADD RSP, 028h
L_1800044a7:
    POP RBX
L_1800044a8:
    POP RDI
L_1800044a9:
    POP RSI
L_1800044aa:
    POP R14
L_1800044ac:
    RET
reconstructed_174 ENDP
; waveInUnprepareHeader @ 175; original VA 0x1800044ad
; assembly ev_69bb288ad819642316e004fc883fafc39eef1a5e43dfe354128da263ed56cfea; pseudocode ev_e3d8bc0028e9d723c71fb72b2f0eb797f2fa9e3356d15d43ac3f36f45850b0a9
PUBLIC reconstructed_175
reconstructed_175 PROC FRAME
L_1800044ad:
    PUSH R14
    .pushreg R14
L_1800044af:
    PUSH RSI
    .pushreg RSI
L_1800044b0:
    PUSH RDI
    .pushreg RDI
L_1800044b1:
    PUSH RBX
    .pushreg RBX
L_1800044b2:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800044b6:
    MOV R14, RCX
L_1800044b9:
    MOV RCX, qword ptr [OuterState+120]
L_1800044c0:
    TEST RCX, RCX
L_1800044c3:
    JZ L_1800044f8
L_1800044c5:
    MOV RSI, R9
L_1800044c8:
    MOV RDI, R8
L_1800044cb:
    MOV RBX, RDX
L_1800044ce:
    LEA RDX, [name_175]
L_1800044d5:
    CALL qword ptr [__imp_GetProcAddress]
L_1800044db:
    TEST RAX, RAX
L_1800044de:
    JZ L_1800044f8
L_1800044e0:
    MOV RCX, R14
L_1800044e3:
    MOV RDX, RBX
L_1800044e6:
    MOV R8, RDI
L_1800044e9:
    MOV R9, RSI
L_1800044ec:
    ADD RSP, 028h
L_1800044f0:
    POP RBX
L_1800044f1:
    POP RDI
L_1800044f2:
    POP RSI
L_1800044f3:
    POP R14
L_1800044f5:
    JMP RAX
L_1800044f8:
    XOR EAX, EAX
L_1800044fa:
    ADD RSP, 028h
L_1800044fe:
    POP RBX
L_1800044ff:
    POP RDI
L_180004500:
    POP RSI
L_180004501:
    POP R14
L_180004503:
    RET
reconstructed_175 ENDP
; waveOutBreakLoop @ 176; original VA 0x180004504
; assembly ev_48d73f7e8586f8738ad9c3d5892b45903fbe29af27d3e9c3ebdc01f17792f49d; pseudocode ev_fef5b5acd8ccebddc3f0ab2362a5ebd3ab17766c4af0d9bd0802c30b39f7515a
PUBLIC reconstructed_176
reconstructed_176 PROC FRAME
L_180004504:
    PUSH R14
    .pushreg R14
L_180004506:
    PUSH RSI
    .pushreg RSI
L_180004507:
    PUSH RDI
    .pushreg RDI
L_180004508:
    PUSH RBX
    .pushreg RBX
L_180004509:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000450d:
    MOV R14, RCX
L_180004510:
    MOV RCX, qword ptr [OuterState+120]
L_180004517:
    TEST RCX, RCX
L_18000451a:
    JZ L_18000454f
L_18000451c:
    MOV RSI, R9
L_18000451f:
    MOV RDI, R8
L_180004522:
    MOV RBX, RDX
L_180004525:
    LEA RDX, [name_176]
L_18000452c:
    CALL qword ptr [__imp_GetProcAddress]
L_180004532:
    TEST RAX, RAX
L_180004535:
    JZ L_18000454f
L_180004537:
    MOV RCX, R14
L_18000453a:
    MOV RDX, RBX
L_18000453d:
    MOV R8, RDI
L_180004540:
    MOV R9, RSI
L_180004543:
    ADD RSP, 028h
L_180004547:
    POP RBX
L_180004548:
    POP RDI
L_180004549:
    POP RSI
L_18000454a:
    POP R14
L_18000454c:
    JMP RAX
L_18000454f:
    XOR EAX, EAX
L_180004551:
    ADD RSP, 028h
L_180004555:
    POP RBX
L_180004556:
    POP RDI
L_180004557:
    POP RSI
L_180004558:
    POP R14
L_18000455a:
    RET
reconstructed_176 ENDP
; waveOutClose @ 177; original VA 0x18000455b
; assembly ev_c33984fd954fdcee2e42e131d992e63273db7f5ca7e8c6716c688999c3dd022d; pseudocode ev_4a2cedd04d950b85597b01dc448f33ccb7f0f251f380f91bb8a6b3c2341ba437
PUBLIC reconstructed_177
reconstructed_177 PROC FRAME
L_18000455b:
    PUSH R14
    .pushreg R14
L_18000455d:
    PUSH RSI
    .pushreg RSI
L_18000455e:
    PUSH RDI
    .pushreg RDI
L_18000455f:
    PUSH RBX
    .pushreg RBX
L_180004560:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004564:
    MOV R14, RCX
L_180004567:
    MOV RCX, qword ptr [OuterState+120]
L_18000456e:
    TEST RCX, RCX
L_180004571:
    JZ L_1800045a6
L_180004573:
    MOV RSI, R9
L_180004576:
    MOV RDI, R8
L_180004579:
    MOV RBX, RDX
L_18000457c:
    LEA RDX, [name_177]
L_180004583:
    CALL qword ptr [__imp_GetProcAddress]
L_180004589:
    TEST RAX, RAX
L_18000458c:
    JZ L_1800045a6
L_18000458e:
    MOV RCX, R14
L_180004591:
    MOV RDX, RBX
L_180004594:
    MOV R8, RDI
L_180004597:
    MOV R9, RSI
L_18000459a:
    ADD RSP, 028h
L_18000459e:
    POP RBX
L_18000459f:
    POP RDI
L_1800045a0:
    POP RSI
L_1800045a1:
    POP R14
L_1800045a3:
    JMP RAX
L_1800045a6:
    XOR EAX, EAX
L_1800045a8:
    ADD RSP, 028h
L_1800045ac:
    POP RBX
L_1800045ad:
    POP RDI
L_1800045ae:
    POP RSI
L_1800045af:
    POP R14
L_1800045b1:
    RET
reconstructed_177 ENDP
; waveOutGetDevCapsA @ 178; original VA 0x1800045b2
; assembly ev_69f8922554bec4544f6717738812ef6daff3697e556e9d0836419f6905f435fa; pseudocode ev_75c28f7263c74549d52aa27df906e23adc1fe59251b797fdf4b31701177fb417
PUBLIC reconstructed_178
reconstructed_178 PROC FRAME
L_1800045b2:
    PUSH R14
    .pushreg R14
L_1800045b4:
    PUSH RSI
    .pushreg RSI
L_1800045b5:
    PUSH RDI
    .pushreg RDI
L_1800045b6:
    PUSH RBX
    .pushreg RBX
L_1800045b7:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800045bb:
    MOV R14, RCX
L_1800045be:
    MOV RCX, qword ptr [OuterState+120]
L_1800045c5:
    TEST RCX, RCX
L_1800045c8:
    JZ L_1800045fd
L_1800045ca:
    MOV RSI, R9
L_1800045cd:
    MOV RDI, R8
L_1800045d0:
    MOV RBX, RDX
L_1800045d3:
    LEA RDX, [name_178]
L_1800045da:
    CALL qword ptr [__imp_GetProcAddress]
L_1800045e0:
    TEST RAX, RAX
L_1800045e3:
    JZ L_1800045fd
L_1800045e5:
    MOV RCX, R14
L_1800045e8:
    MOV RDX, RBX
L_1800045eb:
    MOV R8, RDI
L_1800045ee:
    MOV R9, RSI
L_1800045f1:
    ADD RSP, 028h
L_1800045f5:
    POP RBX
L_1800045f6:
    POP RDI
L_1800045f7:
    POP RSI
L_1800045f8:
    POP R14
L_1800045fa:
    JMP RAX
L_1800045fd:
    XOR EAX, EAX
L_1800045ff:
    ADD RSP, 028h
L_180004603:
    POP RBX
L_180004604:
    POP RDI
L_180004605:
    POP RSI
L_180004606:
    POP R14
L_180004608:
    RET
reconstructed_178 ENDP
; waveOutGetDevCapsW @ 179; original VA 0x180004609
; assembly ev_52025a84ea0a55428f6309d30dd0594041d0f3e597e70fcb4f7d9e6fadde38ab; pseudocode ev_13210fa90c23f7c96ec91f01f9d06ee9dd704177c09710135a55352dc5b82570
PUBLIC reconstructed_179
reconstructed_179 PROC FRAME
L_180004609:
    PUSH R14
    .pushreg R14
L_18000460b:
    PUSH RSI
    .pushreg RSI
L_18000460c:
    PUSH RDI
    .pushreg RDI
L_18000460d:
    PUSH RBX
    .pushreg RBX
L_18000460e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004612:
    MOV R14, RCX
L_180004615:
    MOV RCX, qword ptr [OuterState+120]
L_18000461c:
    TEST RCX, RCX
L_18000461f:
    JZ L_180004654
L_180004621:
    MOV RSI, R9
L_180004624:
    MOV RDI, R8
L_180004627:
    MOV RBX, RDX
L_18000462a:
    LEA RDX, [name_179]
L_180004631:
    CALL qword ptr [__imp_GetProcAddress]
L_180004637:
    TEST RAX, RAX
L_18000463a:
    JZ L_180004654
L_18000463c:
    MOV RCX, R14
L_18000463f:
    MOV RDX, RBX
L_180004642:
    MOV R8, RDI
L_180004645:
    MOV R9, RSI
L_180004648:
    ADD RSP, 028h
L_18000464c:
    POP RBX
L_18000464d:
    POP RDI
L_18000464e:
    POP RSI
L_18000464f:
    POP R14
L_180004651:
    JMP RAX
L_180004654:
    XOR EAX, EAX
L_180004656:
    ADD RSP, 028h
L_18000465a:
    POP RBX
L_18000465b:
    POP RDI
L_18000465c:
    POP RSI
L_18000465d:
    POP R14
L_18000465f:
    RET
reconstructed_179 ENDP
; waveOutGetErrorTextA @ 180; original VA 0x180004660
; assembly ev_0f7103fc0eb094601041bf5883ceb96f526d5d36f48a9bf65de645083650c6ee; pseudocode ev_4e3c73f44740d25ce1f363f56210daecab9b0469da08930b85a741eaf0236264
PUBLIC reconstructed_180
reconstructed_180 PROC FRAME
L_180004660:
    PUSH R14
    .pushreg R14
L_180004662:
    PUSH RSI
    .pushreg RSI
L_180004663:
    PUSH RDI
    .pushreg RDI
L_180004664:
    PUSH RBX
    .pushreg RBX
L_180004665:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004669:
    MOV R14, RCX
L_18000466c:
    MOV RCX, qword ptr [OuterState+120]
L_180004673:
    TEST RCX, RCX
L_180004676:
    JZ L_1800046ab
L_180004678:
    MOV RSI, R9
L_18000467b:
    MOV RDI, R8
L_18000467e:
    MOV RBX, RDX
L_180004681:
    LEA RDX, [name_180]
L_180004688:
    CALL qword ptr [__imp_GetProcAddress]
L_18000468e:
    TEST RAX, RAX
L_180004691:
    JZ L_1800046ab
L_180004693:
    MOV RCX, R14
L_180004696:
    MOV RDX, RBX
L_180004699:
    MOV R8, RDI
L_18000469c:
    MOV R9, RSI
L_18000469f:
    ADD RSP, 028h
L_1800046a3:
    POP RBX
L_1800046a4:
    POP RDI
L_1800046a5:
    POP RSI
L_1800046a6:
    POP R14
L_1800046a8:
    JMP RAX
L_1800046ab:
    XOR EAX, EAX
L_1800046ad:
    ADD RSP, 028h
L_1800046b1:
    POP RBX
L_1800046b2:
    POP RDI
L_1800046b3:
    POP RSI
L_1800046b4:
    POP R14
L_1800046b6:
    RET
reconstructed_180 ENDP
; waveOutGetErrorTextW @ 181; original VA 0x1800046b7
; assembly ev_07b0de57e97e2e71eccf05bac677658ca764f7e4f11ce42bf0648cc4c857884f; pseudocode ev_9b6a02ad9ade8b9bc36c3d18213455eab96244acacde8d068441ac40bca9365f
PUBLIC reconstructed_181
reconstructed_181 PROC FRAME
L_1800046b7:
    PUSH R14
    .pushreg R14
L_1800046b9:
    PUSH RSI
    .pushreg RSI
L_1800046ba:
    PUSH RDI
    .pushreg RDI
L_1800046bb:
    PUSH RBX
    .pushreg RBX
L_1800046bc:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800046c0:
    MOV R14, RCX
L_1800046c3:
    MOV RCX, qword ptr [OuterState+120]
L_1800046ca:
    TEST RCX, RCX
L_1800046cd:
    JZ L_180004702
L_1800046cf:
    MOV RSI, R9
L_1800046d2:
    MOV RDI, R8
L_1800046d5:
    MOV RBX, RDX
L_1800046d8:
    LEA RDX, [name_181]
L_1800046df:
    CALL qword ptr [__imp_GetProcAddress]
L_1800046e5:
    TEST RAX, RAX
L_1800046e8:
    JZ L_180004702
L_1800046ea:
    MOV RCX, R14
L_1800046ed:
    MOV RDX, RBX
L_1800046f0:
    MOV R8, RDI
L_1800046f3:
    MOV R9, RSI
L_1800046f6:
    ADD RSP, 028h
L_1800046fa:
    POP RBX
L_1800046fb:
    POP RDI
L_1800046fc:
    POP RSI
L_1800046fd:
    POP R14
L_1800046ff:
    JMP RAX
L_180004702:
    XOR EAX, EAX
L_180004704:
    ADD RSP, 028h
L_180004708:
    POP RBX
L_180004709:
    POP RDI
L_18000470a:
    POP RSI
L_18000470b:
    POP R14
L_18000470d:
    RET
reconstructed_181 ENDP
; waveOutGetID @ 182; original VA 0x18000470e
; assembly ev_8b4447b8901ee204de7f11f68ebdbdbc378a036692e9df7d77008d090c6e0678; pseudocode ev_58435b56b454a8d124389ceb5d733e33311438200d2f73c78fb7bbad966bbe62
PUBLIC reconstructed_182
reconstructed_182 PROC FRAME
L_18000470e:
    PUSH R14
    .pushreg R14
L_180004710:
    PUSH RSI
    .pushreg RSI
L_180004711:
    PUSH RDI
    .pushreg RDI
L_180004712:
    PUSH RBX
    .pushreg RBX
L_180004713:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004717:
    MOV R14, RCX
L_18000471a:
    MOV RCX, qword ptr [OuterState+120]
L_180004721:
    TEST RCX, RCX
L_180004724:
    JZ L_180004759
L_180004726:
    MOV RSI, R9
L_180004729:
    MOV RDI, R8
L_18000472c:
    MOV RBX, RDX
L_18000472f:
    LEA RDX, [name_182]
L_180004736:
    CALL qword ptr [__imp_GetProcAddress]
L_18000473c:
    TEST RAX, RAX
L_18000473f:
    JZ L_180004759
L_180004741:
    MOV RCX, R14
L_180004744:
    MOV RDX, RBX
L_180004747:
    MOV R8, RDI
L_18000474a:
    MOV R9, RSI
L_18000474d:
    ADD RSP, 028h
L_180004751:
    POP RBX
L_180004752:
    POP RDI
L_180004753:
    POP RSI
L_180004754:
    POP R14
L_180004756:
    JMP RAX
L_180004759:
    XOR EAX, EAX
L_18000475b:
    ADD RSP, 028h
L_18000475f:
    POP RBX
L_180004760:
    POP RDI
L_180004761:
    POP RSI
L_180004762:
    POP R14
L_180004764:
    RET
reconstructed_182 ENDP
; waveOutGetNumDevs @ 183; original VA 0x180004765
; assembly ev_73c0daad47006c0b1fa84a7827daf6a8726a2ed649d21348d1aafa3587b9ae7e; pseudocode ev_9b988f76a38c190cbd41a183bdcc4175cd5d299b68136518436c9e074f97fd8c
PUBLIC reconstructed_183
reconstructed_183 PROC FRAME
L_180004765:
    PUSH R14
    .pushreg R14
L_180004767:
    PUSH RSI
    .pushreg RSI
L_180004768:
    PUSH RDI
    .pushreg RDI
L_180004769:
    PUSH RBX
    .pushreg RBX
L_18000476a:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000476e:
    MOV R14, RCX
L_180004771:
    MOV RCX, qword ptr [OuterState+120]
L_180004778:
    TEST RCX, RCX
L_18000477b:
    JZ L_1800047b0
L_18000477d:
    MOV RSI, R9
L_180004780:
    MOV RDI, R8
L_180004783:
    MOV RBX, RDX
L_180004786:
    LEA RDX, [name_183]
L_18000478d:
    CALL qword ptr [__imp_GetProcAddress]
L_180004793:
    TEST RAX, RAX
L_180004796:
    JZ L_1800047b0
L_180004798:
    MOV RCX, R14
L_18000479b:
    MOV RDX, RBX
L_18000479e:
    MOV R8, RDI
L_1800047a1:
    MOV R9, RSI
L_1800047a4:
    ADD RSP, 028h
L_1800047a8:
    POP RBX
L_1800047a9:
    POP RDI
L_1800047aa:
    POP RSI
L_1800047ab:
    POP R14
L_1800047ad:
    JMP RAX
L_1800047b0:
    XOR EAX, EAX
L_1800047b2:
    ADD RSP, 028h
L_1800047b6:
    POP RBX
L_1800047b7:
    POP RDI
L_1800047b8:
    POP RSI
L_1800047b9:
    POP R14
L_1800047bb:
    RET
reconstructed_183 ENDP
; waveOutGetPitch @ 184; original VA 0x1800047bc
; assembly ev_cc694e8fce2d70b089a5a710e277b1b817cc40511957f7d73fcb5cc5ed50962d; pseudocode ev_aa369a24a23863d57f13e370471fb1d234f12feb19ef16e7d8ab116a164bc183
PUBLIC reconstructed_184
reconstructed_184 PROC FRAME
L_1800047bc:
    PUSH R14
    .pushreg R14
L_1800047be:
    PUSH RSI
    .pushreg RSI
L_1800047bf:
    PUSH RDI
    .pushreg RDI
L_1800047c0:
    PUSH RBX
    .pushreg RBX
L_1800047c1:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800047c5:
    MOV R14, RCX
L_1800047c8:
    MOV RCX, qword ptr [OuterState+120]
L_1800047cf:
    TEST RCX, RCX
L_1800047d2:
    JZ L_180004807
L_1800047d4:
    MOV RSI, R9
L_1800047d7:
    MOV RDI, R8
L_1800047da:
    MOV RBX, RDX
L_1800047dd:
    LEA RDX, [name_184]
L_1800047e4:
    CALL qword ptr [__imp_GetProcAddress]
L_1800047ea:
    TEST RAX, RAX
L_1800047ed:
    JZ L_180004807
L_1800047ef:
    MOV RCX, R14
L_1800047f2:
    MOV RDX, RBX
L_1800047f5:
    MOV R8, RDI
L_1800047f8:
    MOV R9, RSI
L_1800047fb:
    ADD RSP, 028h
L_1800047ff:
    POP RBX
L_180004800:
    POP RDI
L_180004801:
    POP RSI
L_180004802:
    POP R14
L_180004804:
    JMP RAX
L_180004807:
    XOR EAX, EAX
L_180004809:
    ADD RSP, 028h
L_18000480d:
    POP RBX
L_18000480e:
    POP RDI
L_18000480f:
    POP RSI
L_180004810:
    POP R14
L_180004812:
    RET
reconstructed_184 ENDP
; waveOutGetPlaybackRate @ 185; original VA 0x180004813
; assembly ev_6882f2188e0039b9162d3a1cc14f45451be0cf989cd1a70d6c0196ad63fca4f3; pseudocode ev_40cf07b28934b9ef57234e126387fa724c9e61a99d98c47601ffe2700247c3bf
PUBLIC reconstructed_185
reconstructed_185 PROC FRAME
L_180004813:
    PUSH R14
    .pushreg R14
L_180004815:
    PUSH RSI
    .pushreg RSI
L_180004816:
    PUSH RDI
    .pushreg RDI
L_180004817:
    PUSH RBX
    .pushreg RBX
L_180004818:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_18000481c:
    MOV R14, RCX
L_18000481f:
    MOV RCX, qword ptr [OuterState+120]
L_180004826:
    TEST RCX, RCX
L_180004829:
    JZ L_18000485e
L_18000482b:
    MOV RSI, R9
L_18000482e:
    MOV RDI, R8
L_180004831:
    MOV RBX, RDX
L_180004834:
    LEA RDX, [name_185]
L_18000483b:
    CALL qword ptr [__imp_GetProcAddress]
L_180004841:
    TEST RAX, RAX
L_180004844:
    JZ L_18000485e
L_180004846:
    MOV RCX, R14
L_180004849:
    MOV RDX, RBX
L_18000484c:
    MOV R8, RDI
L_18000484f:
    MOV R9, RSI
L_180004852:
    ADD RSP, 028h
L_180004856:
    POP RBX
L_180004857:
    POP RDI
L_180004858:
    POP RSI
L_180004859:
    POP R14
L_18000485b:
    JMP RAX
L_18000485e:
    XOR EAX, EAX
L_180004860:
    ADD RSP, 028h
L_180004864:
    POP RBX
L_180004865:
    POP RDI
L_180004866:
    POP RSI
L_180004867:
    POP R14
L_180004869:
    RET
reconstructed_185 ENDP
; waveOutGetPosition @ 186; original VA 0x18000486a
; assembly ev_de6467103b5946a5acb10537d7a35b8436a08beaec523914cce157859c605f3b; pseudocode ev_1f04647a40093701baade97fb4f9c64a3526a258d599cd29dd89595410103aa4
PUBLIC reconstructed_186
reconstructed_186 PROC FRAME
L_18000486a:
    PUSH R14
    .pushreg R14
L_18000486c:
    PUSH RSI
    .pushreg RSI
L_18000486d:
    PUSH RDI
    .pushreg RDI
L_18000486e:
    PUSH RBX
    .pushreg RBX
L_18000486f:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004873:
    MOV R14, RCX
L_180004876:
    MOV RCX, qword ptr [OuterState+120]
L_18000487d:
    TEST RCX, RCX
L_180004880:
    JZ L_1800048b5
L_180004882:
    MOV RSI, R9
L_180004885:
    MOV RDI, R8
L_180004888:
    MOV RBX, RDX
L_18000488b:
    LEA RDX, [name_186]
L_180004892:
    CALL qword ptr [__imp_GetProcAddress]
L_180004898:
    TEST RAX, RAX
L_18000489b:
    JZ L_1800048b5
L_18000489d:
    MOV RCX, R14
L_1800048a0:
    MOV RDX, RBX
L_1800048a3:
    MOV R8, RDI
L_1800048a6:
    MOV R9, RSI
L_1800048a9:
    ADD RSP, 028h
L_1800048ad:
    POP RBX
L_1800048ae:
    POP RDI
L_1800048af:
    POP RSI
L_1800048b0:
    POP R14
L_1800048b2:
    JMP RAX
L_1800048b5:
    XOR EAX, EAX
L_1800048b7:
    ADD RSP, 028h
L_1800048bb:
    POP RBX
L_1800048bc:
    POP RDI
L_1800048bd:
    POP RSI
L_1800048be:
    POP R14
L_1800048c0:
    RET
reconstructed_186 ENDP
; waveOutGetVolume @ 187; original VA 0x1800048c1
; assembly ev_146c9c4cf475b4a5530090e8da5653c3280bc021862147bc1ecfac4c29e469c1; pseudocode ev_bbc8b66abebf0b18008aa32367cfee4c85874d57a3e29b5007c5e382848eac1d
PUBLIC reconstructed_187
reconstructed_187 PROC FRAME
L_1800048c1:
    PUSH R14
    .pushreg R14
L_1800048c3:
    PUSH RSI
    .pushreg RSI
L_1800048c4:
    PUSH RDI
    .pushreg RDI
L_1800048c5:
    PUSH RBX
    .pushreg RBX
L_1800048c6:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800048ca:
    MOV R14, RCX
L_1800048cd:
    MOV RCX, qword ptr [OuterState+120]
L_1800048d4:
    TEST RCX, RCX
L_1800048d7:
    JZ L_18000490c
L_1800048d9:
    MOV RSI, R9
L_1800048dc:
    MOV RDI, R8
L_1800048df:
    MOV RBX, RDX
L_1800048e2:
    LEA RDX, [name_187]
L_1800048e9:
    CALL qword ptr [__imp_GetProcAddress]
L_1800048ef:
    TEST RAX, RAX
L_1800048f2:
    JZ L_18000490c
L_1800048f4:
    MOV RCX, R14
L_1800048f7:
    MOV RDX, RBX
L_1800048fa:
    MOV R8, RDI
L_1800048fd:
    MOV R9, RSI
L_180004900:
    ADD RSP, 028h
L_180004904:
    POP RBX
L_180004905:
    POP RDI
L_180004906:
    POP RSI
L_180004907:
    POP R14
L_180004909:
    JMP RAX
L_18000490c:
    XOR EAX, EAX
L_18000490e:
    ADD RSP, 028h
L_180004912:
    POP RBX
L_180004913:
    POP RDI
L_180004914:
    POP RSI
L_180004915:
    POP R14
L_180004917:
    RET
reconstructed_187 ENDP
; waveOutMessage @ 188; original VA 0x180004918
; assembly ev_a39603b539ca686f896e004761009a7199f9f60394a9dcf19a672ea7366a1c15; pseudocode ev_37b80e7c9a555cb0fae4d1f6b3a99033619808da9caed00c1996796ecc44c7d8
PUBLIC reconstructed_188
reconstructed_188 PROC FRAME
L_180004918:
    PUSH R14
    .pushreg R14
L_18000491a:
    PUSH RSI
    .pushreg RSI
L_18000491b:
    PUSH RDI
    .pushreg RDI
L_18000491c:
    PUSH RBX
    .pushreg RBX
L_18000491d:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004921:
    MOV R14, RCX
L_180004924:
    MOV RCX, qword ptr [OuterState+120]
L_18000492b:
    TEST RCX, RCX
L_18000492e:
    JZ L_180004963
L_180004930:
    MOV RSI, R9
L_180004933:
    MOV RDI, R8
L_180004936:
    MOV RBX, RDX
L_180004939:
    LEA RDX, [name_188]
L_180004940:
    CALL qword ptr [__imp_GetProcAddress]
L_180004946:
    TEST RAX, RAX
L_180004949:
    JZ L_180004963
L_18000494b:
    MOV RCX, R14
L_18000494e:
    MOV RDX, RBX
L_180004951:
    MOV R8, RDI
L_180004954:
    MOV R9, RSI
L_180004957:
    ADD RSP, 028h
L_18000495b:
    POP RBX
L_18000495c:
    POP RDI
L_18000495d:
    POP RSI
L_18000495e:
    POP R14
L_180004960:
    JMP RAX
L_180004963:
    XOR EAX, EAX
L_180004965:
    ADD RSP, 028h
L_180004969:
    POP RBX
L_18000496a:
    POP RDI
L_18000496b:
    POP RSI
L_18000496c:
    POP R14
L_18000496e:
    RET
reconstructed_188 ENDP
; waveOutOpen @ 189; original VA 0x18000496f
; assembly ev_6d20637b33e526f62429eb9c75b244fd91574552b8dd425aae88a64c49adb1bd; pseudocode ev_ffdd9a85049219cd655a9960f4b0b1e4a3787af2db763c628d02625f6657ca98
PUBLIC reconstructed_189
reconstructed_189 PROC FRAME
L_18000496f:
    PUSH R14
    .pushreg R14
L_180004971:
    PUSH RSI
    .pushreg RSI
L_180004972:
    PUSH RDI
    .pushreg RDI
L_180004973:
    PUSH RBX
    .pushreg RBX
L_180004974:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004978:
    MOV R14, RCX
L_18000497b:
    MOV RCX, qword ptr [OuterState+120]
L_180004982:
    TEST RCX, RCX
L_180004985:
    JZ L_1800049ba
L_180004987:
    MOV RSI, R9
L_18000498a:
    MOV RDI, R8
L_18000498d:
    MOV RBX, RDX
L_180004990:
    LEA RDX, [name_189]
L_180004997:
    CALL qword ptr [__imp_GetProcAddress]
L_18000499d:
    TEST RAX, RAX
L_1800049a0:
    JZ L_1800049ba
L_1800049a2:
    MOV RCX, R14
L_1800049a5:
    MOV RDX, RBX
L_1800049a8:
    MOV R8, RDI
L_1800049ab:
    MOV R9, RSI
L_1800049ae:
    ADD RSP, 028h
L_1800049b2:
    POP RBX
L_1800049b3:
    POP RDI
L_1800049b4:
    POP RSI
L_1800049b5:
    POP R14
L_1800049b7:
    JMP RAX
L_1800049ba:
    XOR EAX, EAX
L_1800049bc:
    ADD RSP, 028h
L_1800049c0:
    POP RBX
L_1800049c1:
    POP RDI
L_1800049c2:
    POP RSI
L_1800049c3:
    POP R14
L_1800049c5:
    RET
reconstructed_189 ENDP
; waveOutPause @ 190; original VA 0x1800049c6
; assembly ev_39b6b2cc50bedd4af0d87aabec6a2b273beca930f77ffb4c9bec7dee71815c6c; pseudocode ev_3cc9052842296af6ea4348d2868c3d243a2a5c4ad8d25aec8ee7d7a62fedc455
PUBLIC reconstructed_190
reconstructed_190 PROC FRAME
L_1800049c6:
    PUSH R14
    .pushreg R14
L_1800049c8:
    PUSH RSI
    .pushreg RSI
L_1800049c9:
    PUSH RDI
    .pushreg RDI
L_1800049ca:
    PUSH RBX
    .pushreg RBX
L_1800049cb:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_1800049cf:
    MOV R14, RCX
L_1800049d2:
    MOV RCX, qword ptr [OuterState+120]
L_1800049d9:
    TEST RCX, RCX
L_1800049dc:
    JZ L_180004a11
L_1800049de:
    MOV RSI, R9
L_1800049e1:
    MOV RDI, R8
L_1800049e4:
    MOV RBX, RDX
L_1800049e7:
    LEA RDX, [name_190]
L_1800049ee:
    CALL qword ptr [__imp_GetProcAddress]
L_1800049f4:
    TEST RAX, RAX
L_1800049f7:
    JZ L_180004a11
L_1800049f9:
    MOV RCX, R14
L_1800049fc:
    MOV RDX, RBX
L_1800049ff:
    MOV R8, RDI
L_180004a02:
    MOV R9, RSI
L_180004a05:
    ADD RSP, 028h
L_180004a09:
    POP RBX
L_180004a0a:
    POP RDI
L_180004a0b:
    POP RSI
L_180004a0c:
    POP R14
L_180004a0e:
    JMP RAX
L_180004a11:
    XOR EAX, EAX
L_180004a13:
    ADD RSP, 028h
L_180004a17:
    POP RBX
L_180004a18:
    POP RDI
L_180004a19:
    POP RSI
L_180004a1a:
    POP R14
L_180004a1c:
    RET
reconstructed_190 ENDP
; waveOutPrepareHeader @ 191; original VA 0x180004a1d
; assembly ev_5628e3abaf99063607854c2b820da8f775dc4d9d77920af596eb5a0fc5ceba77; pseudocode ev_d0c78d85b2acaae01152e60f23bcd5fb2ef35aecb4f6de0ad870d48d10f2ed76
PUBLIC reconstructed_191
reconstructed_191 PROC FRAME
L_180004a1d:
    PUSH R14
    .pushreg R14
L_180004a1f:
    PUSH RSI
    .pushreg RSI
L_180004a20:
    PUSH RDI
    .pushreg RDI
L_180004a21:
    PUSH RBX
    .pushreg RBX
L_180004a22:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004a26:
    MOV R14, RCX
L_180004a29:
    MOV RCX, qword ptr [OuterState+120]
L_180004a30:
    TEST RCX, RCX
L_180004a33:
    JZ L_180004a68
L_180004a35:
    MOV RSI, R9
L_180004a38:
    MOV RDI, R8
L_180004a3b:
    MOV RBX, RDX
L_180004a3e:
    LEA RDX, [name_191]
L_180004a45:
    CALL qword ptr [__imp_GetProcAddress]
L_180004a4b:
    TEST RAX, RAX
L_180004a4e:
    JZ L_180004a68
L_180004a50:
    MOV RCX, R14
L_180004a53:
    MOV RDX, RBX
L_180004a56:
    MOV R8, RDI
L_180004a59:
    MOV R9, RSI
L_180004a5c:
    ADD RSP, 028h
L_180004a60:
    POP RBX
L_180004a61:
    POP RDI
L_180004a62:
    POP RSI
L_180004a63:
    POP R14
L_180004a65:
    JMP RAX
L_180004a68:
    XOR EAX, EAX
L_180004a6a:
    ADD RSP, 028h
L_180004a6e:
    POP RBX
L_180004a6f:
    POP RDI
L_180004a70:
    POP RSI
L_180004a71:
    POP R14
L_180004a73:
    RET
reconstructed_191 ENDP
; waveOutReset @ 192; original VA 0x180004a74
; assembly ev_c83f3f988dc1bcbdcd87901d6aa437bc92826db7ede297670785bed058e13a70; pseudocode ev_337fa11cb982b7d118a4c1e4e7095ee7ebdde9bc885d64d5916a1c5e341c50b2
PUBLIC reconstructed_192
reconstructed_192 PROC FRAME
L_180004a74:
    PUSH R14
    .pushreg R14
L_180004a76:
    PUSH RSI
    .pushreg RSI
L_180004a77:
    PUSH RDI
    .pushreg RDI
L_180004a78:
    PUSH RBX
    .pushreg RBX
L_180004a79:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004a7d:
    MOV R14, RCX
L_180004a80:
    MOV RCX, qword ptr [OuterState+120]
L_180004a87:
    TEST RCX, RCX
L_180004a8a:
    JZ L_180004abf
L_180004a8c:
    MOV RSI, R9
L_180004a8f:
    MOV RDI, R8
L_180004a92:
    MOV RBX, RDX
L_180004a95:
    LEA RDX, [name_192]
L_180004a9c:
    CALL qword ptr [__imp_GetProcAddress]
L_180004aa2:
    TEST RAX, RAX
L_180004aa5:
    JZ L_180004abf
L_180004aa7:
    MOV RCX, R14
L_180004aaa:
    MOV RDX, RBX
L_180004aad:
    MOV R8, RDI
L_180004ab0:
    MOV R9, RSI
L_180004ab3:
    ADD RSP, 028h
L_180004ab7:
    POP RBX
L_180004ab8:
    POP RDI
L_180004ab9:
    POP RSI
L_180004aba:
    POP R14
L_180004abc:
    JMP RAX
L_180004abf:
    XOR EAX, EAX
L_180004ac1:
    ADD RSP, 028h
L_180004ac5:
    POP RBX
L_180004ac6:
    POP RDI
L_180004ac7:
    POP RSI
L_180004ac8:
    POP R14
L_180004aca:
    RET
reconstructed_192 ENDP
; waveOutRestart @ 193; original VA 0x180004acb
; assembly ev_c651150f8e46a45647f2db55e54427142396a4bc8a0d60361f86247056fd830e; pseudocode ev_60bf82602369ca6a77874e17b79efa5f44f04ef59eb8348c36334cd325994575
PUBLIC reconstructed_193
reconstructed_193 PROC FRAME
L_180004acb:
    PUSH R14
    .pushreg R14
L_180004acd:
    PUSH RSI
    .pushreg RSI
L_180004ace:
    PUSH RDI
    .pushreg RDI
L_180004acf:
    PUSH RBX
    .pushreg RBX
L_180004ad0:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004ad4:
    MOV R14, RCX
L_180004ad7:
    MOV RCX, qword ptr [OuterState+120]
L_180004ade:
    TEST RCX, RCX
L_180004ae1:
    JZ L_180004b16
L_180004ae3:
    MOV RSI, R9
L_180004ae6:
    MOV RDI, R8
L_180004ae9:
    MOV RBX, RDX
L_180004aec:
    LEA RDX, [name_193]
L_180004af3:
    CALL qword ptr [__imp_GetProcAddress]
L_180004af9:
    TEST RAX, RAX
L_180004afc:
    JZ L_180004b16
L_180004afe:
    MOV RCX, R14
L_180004b01:
    MOV RDX, RBX
L_180004b04:
    MOV R8, RDI
L_180004b07:
    MOV R9, RSI
L_180004b0a:
    ADD RSP, 028h
L_180004b0e:
    POP RBX
L_180004b0f:
    POP RDI
L_180004b10:
    POP RSI
L_180004b11:
    POP R14
L_180004b13:
    JMP RAX
L_180004b16:
    XOR EAX, EAX
L_180004b18:
    ADD RSP, 028h
L_180004b1c:
    POP RBX
L_180004b1d:
    POP RDI
L_180004b1e:
    POP RSI
L_180004b1f:
    POP R14
L_180004b21:
    RET
reconstructed_193 ENDP
; waveOutSetPitch @ 194; original VA 0x180004b22
; assembly ev_96d028f1b8e92d7a5449c1ef0014673a110a557fab5cddc17e282dd5c3b6d969; pseudocode ev_a7a3ef1b61e98d22fc4c7830b0a415365a5517625dbf7ad9fa085642a6674c10
PUBLIC reconstructed_194
reconstructed_194 PROC FRAME
L_180004b22:
    PUSH R14
    .pushreg R14
L_180004b24:
    PUSH RSI
    .pushreg RSI
L_180004b25:
    PUSH RDI
    .pushreg RDI
L_180004b26:
    PUSH RBX
    .pushreg RBX
L_180004b27:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004b2b:
    MOV R14, RCX
L_180004b2e:
    MOV RCX, qword ptr [OuterState+120]
L_180004b35:
    TEST RCX, RCX
L_180004b38:
    JZ L_180004b6d
L_180004b3a:
    MOV RSI, R9
L_180004b3d:
    MOV RDI, R8
L_180004b40:
    MOV RBX, RDX
L_180004b43:
    LEA RDX, [name_194]
L_180004b4a:
    CALL qword ptr [__imp_GetProcAddress]
L_180004b50:
    TEST RAX, RAX
L_180004b53:
    JZ L_180004b6d
L_180004b55:
    MOV RCX, R14
L_180004b58:
    MOV RDX, RBX
L_180004b5b:
    MOV R8, RDI
L_180004b5e:
    MOV R9, RSI
L_180004b61:
    ADD RSP, 028h
L_180004b65:
    POP RBX
L_180004b66:
    POP RDI
L_180004b67:
    POP RSI
L_180004b68:
    POP R14
L_180004b6a:
    JMP RAX
L_180004b6d:
    XOR EAX, EAX
L_180004b6f:
    ADD RSP, 028h
L_180004b73:
    POP RBX
L_180004b74:
    POP RDI
L_180004b75:
    POP RSI
L_180004b76:
    POP R14
L_180004b78:
    RET
reconstructed_194 ENDP
; waveOutSetPlaybackRate @ 195; original VA 0x180004b79
; assembly ev_e6cc7a9734007824392ed50dbc3aa80327414b0c1d7c50a95a3dd03690237ca9; pseudocode ev_7466814cb94371096b0016fe1859b7a040dafa55040fcd841c2cba7cd27cc53d
PUBLIC reconstructed_195
reconstructed_195 PROC FRAME
L_180004b79:
    PUSH R14
    .pushreg R14
L_180004b7b:
    PUSH RSI
    .pushreg RSI
L_180004b7c:
    PUSH RDI
    .pushreg RDI
L_180004b7d:
    PUSH RBX
    .pushreg RBX
L_180004b7e:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004b82:
    MOV R14, RCX
L_180004b85:
    MOV RCX, qword ptr [OuterState+120]
L_180004b8c:
    TEST RCX, RCX
L_180004b8f:
    JZ L_180004bc4
L_180004b91:
    MOV RSI, R9
L_180004b94:
    MOV RDI, R8
L_180004b97:
    MOV RBX, RDX
L_180004b9a:
    LEA RDX, [name_195]
L_180004ba1:
    CALL qword ptr [__imp_GetProcAddress]
L_180004ba7:
    TEST RAX, RAX
L_180004baa:
    JZ L_180004bc4
L_180004bac:
    MOV RCX, R14
L_180004baf:
    MOV RDX, RBX
L_180004bb2:
    MOV R8, RDI
L_180004bb5:
    MOV R9, RSI
L_180004bb8:
    ADD RSP, 028h
L_180004bbc:
    POP RBX
L_180004bbd:
    POP RDI
L_180004bbe:
    POP RSI
L_180004bbf:
    POP R14
L_180004bc1:
    JMP RAX
L_180004bc4:
    XOR EAX, EAX
L_180004bc6:
    ADD RSP, 028h
L_180004bca:
    POP RBX
L_180004bcb:
    POP RDI
L_180004bcc:
    POP RSI
L_180004bcd:
    POP R14
L_180004bcf:
    RET
reconstructed_195 ENDP
; waveOutSetVolume @ 196; original VA 0x180004bd0
; assembly ev_a0e5201db8bfa228564e2794f56646c0629e8ca0208cfe4fd3d15425261e3908; pseudocode ev_bca3cb9feabe0fb38912bbf45dcbe244b275f3a8b49617cfbc35420b376ca50c
PUBLIC reconstructed_196
reconstructed_196 PROC FRAME
L_180004bd0:
    PUSH R14
    .pushreg R14
L_180004bd2:
    PUSH RSI
    .pushreg RSI
L_180004bd3:
    PUSH RDI
    .pushreg RDI
L_180004bd4:
    PUSH RBX
    .pushreg RBX
L_180004bd5:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004bd9:
    MOV R14, RCX
L_180004bdc:
    MOV RCX, qword ptr [OuterState+120]
L_180004be3:
    TEST RCX, RCX
L_180004be6:
    JZ L_180004c1b
L_180004be8:
    MOV RSI, R9
L_180004beb:
    MOV RDI, R8
L_180004bee:
    MOV RBX, RDX
L_180004bf1:
    LEA RDX, [name_196]
L_180004bf8:
    CALL qword ptr [__imp_GetProcAddress]
L_180004bfe:
    TEST RAX, RAX
L_180004c01:
    JZ L_180004c1b
L_180004c03:
    MOV RCX, R14
L_180004c06:
    MOV RDX, RBX
L_180004c09:
    MOV R8, RDI
L_180004c0c:
    MOV R9, RSI
L_180004c0f:
    ADD RSP, 028h
L_180004c13:
    POP RBX
L_180004c14:
    POP RDI
L_180004c15:
    POP RSI
L_180004c16:
    POP R14
L_180004c18:
    JMP RAX
L_180004c1b:
    XOR EAX, EAX
L_180004c1d:
    ADD RSP, 028h
L_180004c21:
    POP RBX
L_180004c22:
    POP RDI
L_180004c23:
    POP RSI
L_180004c24:
    POP R14
L_180004c26:
    RET
reconstructed_196 ENDP
; waveOutUnprepareHeader @ 197; original VA 0x180004c27
; assembly ev_03895755030a12e2fa04787cd87d40a22863653cd9977279f6394e4124e69ec3; pseudocode ev_247348b15fcba2a19aa0bd6b6145bebb7da6d603908e84e6741c892475b572f6
PUBLIC reconstructed_197
reconstructed_197 PROC FRAME
L_180004c27:
    PUSH R14
    .pushreg R14
L_180004c29:
    PUSH RSI
    .pushreg RSI
L_180004c2a:
    PUSH RDI
    .pushreg RDI
L_180004c2b:
    PUSH RBX
    .pushreg RBX
L_180004c2c:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004c30:
    MOV R14, RCX
L_180004c33:
    MOV RCX, qword ptr [OuterState+120]
L_180004c3a:
    TEST RCX, RCX
L_180004c3d:
    JZ L_180004c72
L_180004c3f:
    MOV RSI, R9
L_180004c42:
    MOV RDI, R8
L_180004c45:
    MOV RBX, RDX
L_180004c48:
    LEA RDX, [name_197]
L_180004c4f:
    CALL qword ptr [__imp_GetProcAddress]
L_180004c55:
    TEST RAX, RAX
L_180004c58:
    JZ L_180004c72
L_180004c5a:
    MOV RCX, R14
L_180004c5d:
    MOV RDX, RBX
L_180004c60:
    MOV R8, RDI
L_180004c63:
    MOV R9, RSI
L_180004c66:
    ADD RSP, 028h
L_180004c6a:
    POP RBX
L_180004c6b:
    POP RDI
L_180004c6c:
    POP RSI
L_180004c6d:
    POP R14
L_180004c6f:
    JMP RAX
L_180004c72:
    XOR EAX, EAX
L_180004c74:
    ADD RSP, 028h
L_180004c78:
    POP RBX
L_180004c79:
    POP RDI
L_180004c7a:
    POP RSI
L_180004c7b:
    POP R14
L_180004c7d:
    RET
reconstructed_197 ENDP
; waveOutWrite @ 198; original VA 0x180004c7e
; assembly ev_c3f4ca118b8f10bdcefbdfe203fa0002410843ab86ba39854824beded8a2fdbf; pseudocode ev_d66f0acf89d9922a3be8d160cfb095d99e4c4376c8f753845b63260d1c1e8b01
PUBLIC reconstructed_198
reconstructed_198 PROC FRAME
L_180004c7e:
    PUSH R14
    .pushreg R14
L_180004c80:
    PUSH RSI
    .pushreg RSI
L_180004c81:
    PUSH RDI
    .pushreg RDI
L_180004c82:
    PUSH RBX
    .pushreg RBX
L_180004c83:
    SUB RSP, 028h
    .allocstack 028h
    .endprolog
L_180004c87:
    MOV R14, RCX
L_180004c8a:
    MOV RCX, qword ptr [OuterState+120]
L_180004c91:
    TEST RCX, RCX
L_180004c94:
    JZ L_180004cc9
L_180004c96:
    MOV RSI, R9
L_180004c99:
    MOV RDI, R8
L_180004c9c:
    MOV RBX, RDX
L_180004c9f:
    LEA RDX, [name_198]
L_180004ca6:
    CALL qword ptr [__imp_GetProcAddress]
L_180004cac:
    TEST RAX, RAX
L_180004caf:
    JZ L_180004cc9
L_180004cb1:
    MOV RCX, R14
L_180004cb4:
    MOV RDX, RBX
L_180004cb7:
    MOV R8, RDI
L_180004cba:
    MOV R9, RSI
L_180004cbd:
    ADD RSP, 028h
L_180004cc1:
    POP RBX
L_180004cc2:
    POP RDI
L_180004cc3:
    POP RSI
L_180004cc4:
    POP R14
L_180004cc6:
    JMP RAX
L_180004cc9:
    XOR EAX, EAX
L_180004ccb:
    ADD RSP, 028h
L_180004ccf:
    POP RBX
L_180004cd0:
    POP RDI
L_180004cd1:
    POP RSI
L_180004cd2:
    POP R14
L_180004cd4:
    RET
reconstructed_198 ENDP
END
