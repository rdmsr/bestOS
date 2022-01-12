bits 64

global gdt_update
gdt_update:
  lgdt [rdi]
  mov ax, 0x30
  mov ss, ax
  mov ds, ax
  mov es, ax
  pop rdi
  mov rax, 0x28
  push rax
  push rdi
  retfq

global tss_update
tss_update:
  mov ax, 0x48
  ltr ax
  ret