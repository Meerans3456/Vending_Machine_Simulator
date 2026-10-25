; ============================================================
; t_input.asm - standalone test: input.inc (read_int, read_product, read_payment)
; Build  : build t_input
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdra          db '--- read_int test: type numbers, 0 quits ---$'
t_hdrb          db '--- read_product test ---$'
t_hdrc          db '--- read_payment test ---$'
t_ri            db 'read_int> $'
t_ok            db 'OK, AX = $'
t_cf            db 'CF=1 (invalid)$'
t_rp            db 'read_product returned AL = $'
t_rpay          db 'read_payment returned AX = $'

include utils.inc
include input.inc

.code

main proc
    mov ax, @data
    mov ds, ax

    ; --- A: read_int, interactive; entering 0 ends this phase ---
    mov dx, offset t_hdra
    call print_str
    call newline
ta_loop:
    mov dx, offset t_ri
    call print_str
    call read_int
    jc ta_bad
    push ax                 ; print_str clobbers AX
    mov dx, offset t_ok
    call print_str
    pop ax
    call print_int
    call newline
    cmp ax, 0
    jne ta_loop
    jmp ta_done
ta_bad:
    mov dx, offset t_cf
    call print_str
    call newline
    jmp ta_loop
ta_done:

    ; --- B: read_product ---
    call newline
    mov dx, offset t_hdrb
    call print_str
    call newline
    call read_product
    xor ah, ah
    push ax
    mov dx, offset t_rp
    call print_str
    pop ax
    call print_int
    call newline

    ; --- C: read_payment ---
    call newline
    mov dx, offset t_hdrc
    call print_str
    call newline
    call read_payment
    push ax
    mov dx, offset t_rpay
    call print_str
    pop ax
    call print_int
    call newline

    mov ah, 4Ch
    int 21h
main endp

end main
