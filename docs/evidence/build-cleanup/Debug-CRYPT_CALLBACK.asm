
/workspace/yellowfield/build/runtime-debug/CMakeFiles/preloader_l_objects.dir/src/runtime/application.cpp.obj:	file format coff-x86-64

Disassembly of section .text:

0000000000001190 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE>:
    1190: 48 83 ec 58                  	subq	$0x58, %rsp
    1194: 48 89 4c 24 50               	movq	%rcx, 0x50(%rsp)
    1199: 89 54 24 4c                  	movl	%edx, 0x4c(%rsp)
    119d: 48 8b 44 24 50               	movq	0x50(%rsp), %rax
    11a2: 48 89 44 24 30               	movq	%rax, 0x30(%rsp)
    11a7: 8b 05 00 00 00 00            	movl	(%rip), %eax            # 0x11ad <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x1d>
    11ad: 89 c1                        	movl	%eax, %ecx
    11af: 65 48 8b 04 25 58 00 00 00   	movq	%gs:0x58, %rax
    11b8: 48 8b 04 c8                  	movq	(%rax,%rcx,8), %rax
    11bc: 48 8d 80 00 00 00 00         	leaq	(%rax), %rax
    11c3: f6 00 01                     	testb	$0x1, (%rax)
    11c6: 74 02                        	je	0x11ca <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x3a>
    11c8: eb 6d                        	jmp	0x1237 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0xa7>
    11ca: 48 8b 44 24 30               	movq	0x30(%rsp), %rax
    11cf: 8b 0d 00 00 00 00            	movl	(%rip), %ecx            # 0x11d5 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x45>
    11d5: 89 ca                        	movl	%ecx, %edx
    11d7: 65 48 8b 0c 25 58 00 00 00   	movq	%gs:0x58, %rcx
    11e0: 48 8b 0c d1                  	movq	(%rcx,%rdx,8), %rcx
    11e4: c6 81 00 00 00 00 01         	movb	$0x1, (%rcx)
    11eb: 48 8b 40 60                  	movq	0x60(%rax), %rax
    11ef: 48 89 44 24 28               	movq	%rax, 0x28(%rsp)
    11f4: 48 8d 15 f0 05 00 00         	leaq	0x5f0(%rip), %rdx       # 0x17eb <_ZN11preloader_l11application12_GLOBAL__N_15Hooks22version_condition_maskEyjh+0x1b>
    11fb: 48 8d 4c 24 38               	leaq	0x38(%rsp), %rcx
    1200: e8 00 00 00 00               	callq	0x1205 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x75>
    1205: eb 00                        	jmp	0x1207 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x77>
    1207: 48 8b 4c 24 28               	movq	0x28(%rsp), %rcx
    120c: 48 8d 54 24 38               	leaq	0x38(%rsp), %rdx
    1211: e8 00 00 00 00               	callq	0x1216 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x86>
    1216: eb 00                        	jmp	0x1218 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x88>
    1218: 8b 05 00 00 00 00            	movl	(%rip), %eax            # 0x121e <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x8e>
    121e: 89 c1                        	movl	%eax, %ecx
    1220: 65 48 8b 04 25 58 00 00 00   	movq	%gs:0x58, %rax
    1229: 48 8b 04 c8                  	movq	(%rax,%rcx,8), %rax
    122d: 48 8d 80 00 00 00 00         	leaq	(%rax), %rax
    1234: c6 00 00                     	movb	$0x0, (%rax)
    1237: 48 83 c4 58                  	addq	$0x58, %rsp
    123b: c3                           	retq
    123c: 48 89 c1                     	movq	%rax, %rcx
    123f: e8 00 00 00 00               	callq	0x1244 <_ZN11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0xb4>
    1244: cc                           	int3
    1245: 66 66 2e 0f 1f 84 00 00 00 00 00     	nopw	%cs:(%rax,%rax)

0000000000001f60 <_ZThn8_N11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE>:
    1f60: 48 83 ec 10                  	subq	$0x10, %rsp
    1f64: 48 89 4c 24 08               	movq	%rcx, 0x8(%rsp)
    1f69: 89 54 24 04                  	movl	%edx, 0x4(%rsp)
    1f6d: 48 8b 4c 24 08               	movq	0x8(%rsp), %rcx
    1f72: 48 83 c1 f8                  	addq	$-0x8, %rcx
    1f76: 8b 54 24 04                  	movl	0x4(%rsp), %edx
    1f7a: 48 83 c4 10                  	addq	$0x10, %rsp
    1f7e: e9 00 00 00 00               	jmp	0x1f83 <_ZThn8_N11preloader_l11application12_GLOBAL__N_15Hooks14crypt_observedENS_5hooks9SelectionE+0x23>
    1f83: 66 66 66 66 2e 0f 1f 84 00 00 00 00 00       	nopw	%cs:(%rax,%rax)
