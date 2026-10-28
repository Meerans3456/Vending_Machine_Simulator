; ============================================================
; t_input.asm - standalone test: input.inc (read_int, ask_int, read_product)
; Build  : build t_input
; Interactive: follow the header printed before each part.
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdra          db '--- read_int, largest value 999: type numbers, 0 quits ---$'
t_hdrb          db '--- read_int, largest value 65535: type numbers, 0 quits ---$'
t_hdrc          db '--- read_product (product 3 is sold out) ---$'
t_hdrd          db '--- ask_int, largest value 999 ---$'
t_ri            db 'read_int> $'
t_askp          db CR, LF, 'ask_int> $'
t_ok            db CR, LF, 'OK, AX = $'
t_cf            db CR, LF, 'CF=1 (invalid)$'
t_rp            db CR, LF, 'read_product returned AX = $'
t_ra            db CR, LF, 'ask_int returned AX = $'

include utils.inc
include lookup.inc
include stock.inc
include input.inc

.code

; t_read_phase: DX = header string, BX = largest value accepted.
; Calls read_int again and again and shows each result, until 0 is typed.
t_read_phase proc
    call newline
    call print_str          ; header
    call newline
trp_loop:
    mov dx, offset t_ri
    call print_str
    mov dx, bx
    call read_int
    jc trp_bad
    push ax                 ; print_str clobbers AX
    mov dx, offset t_ok
    call print_str
    pop ax
    call print_int
    call newline
    cmp ax, 0
    jne trp_loop
    ret
trp_bad:
    mov dx, offset t_cf
    call print_str
    call newline
    jmp trp_loop
t_read_phase endp

; t_show_ax: DX = label string. Prints the label and then AX.
t_show_ax proc
    push ax
    call print_str
    pop ax
    call print_int
    call newline
    ret
t_show_ax endp

main proc
    mov ax, @data
    mov ds, ax

    mov dx, offset t_hdra
    mov bx, MAX_INPUT
    call t_read_phase

    mov dx, offset t_hdrb
    mov bx, 0FFFFh
    call t_read_phase

    call newline
    mov dx, offset t_hdrc
    call print_str
    call newline
    mov word ptr [product_stock + 4], 0   ; make product 3 sold out
    call read_product
    mov dx, offset t_rp
    call t_show_ax

    call newline
    mov dx, offset t_hdrd
    call print_str
    call newline
    mov dx, offset t_askp
    mov bx, MAX_INPUT
    call ask_int
    mov dx, offset t_ra
    call t_show_ax

    mov ah, 4Ch
    int 21h
main endp

end main
