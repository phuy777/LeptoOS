.section .multiboot_header,"a"
.align 8
header_start:
    .long 0xe85250d6
    .long 0
    .long header_end - header_start
    .long -(0xe85250d6 + (header_end - header_start))
    .short 0
    .short 0
    .long 8
header_end:

.section .text.boot,"ax"
.code32
.global start
.extern kmain
start:
    cli
    cld
    movl $stack_top, %esp

    movl $pml4_table, %edi
    xorl %eax, %eax
    movl $3072, %ecx
    rep stosl

    movl $page_directory_pointer_table + 3, pml4_table
    movl $page_directory_table + 3, page_directory_pointer_table

    movl $page_directory_table, %edi
    movl $0x83, %eax
    movl $512, %ecx
1:
    movl %eax, (%edi)
    movl $0, 4(%edi)
    addl $0x200000, %eax
    addl $8, %edi
    loop 1b

    movl $0x80000000, %eax
    cpuid
    cmpl $0x80000001, %eax
    jb .halt
    movl $0x80000001, %eax
    cpuid
    testl $(1 << 29), %edx
    jz .halt

    lgdt gdt64_pointer

    movl %cr4, %eax    #CR4 HERE
    orl $0x620, %eax
    movl %eax, %cr4

    movl $pml4_table, %eax
    movl %eax, %cr3

    movl $0xc0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr

    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    ljmp $0x08, $long_mode_start

.halt:
    cli
2:
    hlt
    jmp 2b

.code64
long_mode_start:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    xorq %rbp, %rbp
    movq $stack_top, %rsp
    andq $-16, %rsp
    call kmain

    cli
3:
    hlt
    jmp 3b

.section .rodata
.align 8
gdt64:
    .quad 0
    .quad 0x00af9a000000ffff
    .quad 0x00cf92000000ffff
gdt64_pointer:
    .word gdt64_pointer - gdt64 - 1
    .long gdt64

.section .bss
.align 4096
pml4_table:
    .skip 4096
page_directory_pointer_table:
    .skip 4096
page_directory_table:
    .skip 4096

.align 16
stack_bottom:
    .skip 16384
stack_top:
