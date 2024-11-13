section .data
    str0 db "sneha gawar h", 0
    newline db 0xA
section .text
global _start
_start:
    mov rdi,5
    push rdi
    mov rdi,10
    push rdi
    mov rax, 1
    mov rdi, 1
    mov rsi, str0
    mov rdx, 13
    syscall
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
label1:
    push  QWORD [rsp +0]
    push  QWORD [rsp +16]
    pop rax
    pop rbx
    cmp rax, rbx
    setle al
    movzx rax, al
    push rax
    pop rax
    test rax, rax
    jz label0
    mov rdi,2
    push rdi
    push  QWORD [rsp +16]
    pop rax
    pop rbx
    mul rbx
    push rax
    pop rax
    mov [rsp + 8],rax
    add rsp, 0
    jmp label1
label0:
    push  QWORD [rsp +8]
    mov rax, 60
    pop rdi
    syscall
    mov rax, 60
    mov rdi, 0
    syscall