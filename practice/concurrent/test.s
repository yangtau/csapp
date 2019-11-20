	.file	"test.c"
	.text
	.comm	cnt,8,8
	.globl	fn
	.type	fn, @function
fn:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	cnt(%rip), %rax
	addq	$1, %rax
	movq	%rax, cnt(%rip)
	leaq	fn(%rip), %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	fn, .-fn
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
