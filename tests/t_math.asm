; ============================================================
; t_math.asm - standalone test: menu.inc and math.inc (display_menu, calc_change, break_change)
; Build  : build t_math
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdrm          db '--- display_menu ---$'
t_hdrcc         db '--- calc_change (payment - price) ---$'
t_hdrbc         db '--- break_change (counts for 50 25 10 5 1) ---$'
t_cf0           db 'CF=0 change = $'
t_cf1           db 'CF=1 shortfall = $'
t_calc          dw 50, 25,  10, 25,  25, 25     ; payment, price pairs
t_ncalc         equ ($ - t_calc) / 4     ; number of pairs
t_changes       dw 35, 0, 100, 99, 964
t_nchanges      equ ($ - t_changes) / 2  ; number of values
t_axafter       db ' AX after: $'

include utils.inc
include menu.inc
include math.inc

.code

; t_show_counts: prints the 5 values of denom_count separated by spaces.
t_show_counts proc
    push ax
    push cx
    push dx
    push si
    mov si, offset denom_count
    mov cx, NUM_DENOMS
tsc_loop:
    mov ax, word ptr [si]
    call print_int
    mov dl, ' '
    call print_char
    add si, 2
    loop tsc_loop
    pop si
    pop dx
    pop cx
    pop ax
    ret
t_show_counts endp

main proc
    mov ax, @data
    mov ds, ax

    ; --- display_menu ---
    mov dx, offset t_hdrm
    call print_str
    call newline
    call display_menu

    ; --- calc_change on (payment, price) pairs ---
    call newline
    mov dx, offset t_hdrcc
    call print_str
    call newline
    mov si, offset t_calc
    mov cx, t_ncalc
tc_loop:
    mov ax, word ptr [si]
    call print_int          ; payment
    mov dl, ' '
    call print_char
    mov dl, '-'
    call print_char
    mov dl, ' '
    call print_char
    mov ax, word ptr [si+2]
    call print_int          ; price
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    mov ax, word ptr [si]
    mov bx, word ptr [si+2]
    call calc_change
    jc tc_short
    push ax
    mov dx, offset t_cf0
    call print_str
    pop ax
    call print_int
    call newline
    jmp tc_next
tc_short:
    push ax
    mov dx, offset t_cf1
    call print_str
    pop ax
    call print_int
    call newline
tc_next:
    add si, 4
    loop tc_loop

    ; --- break_change on each value in t_changes ---
    call newline
    mov dx, offset t_hdrbc
    call print_str
    call newline
    mov si, offset t_changes
    mov cx, t_nchanges
tk_loop:
    mov ax, word ptr [si]
    call print_int          ; the change value
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    mov ax, word ptr [si]
    call break_change
    call t_show_counts
    push ax                 ; print_str clobbers AX
    mov dx, offset t_axafter
    call print_str
    pop ax
    call print_int          ; must equal the value tested (AX is preserved)
    call newline
    add si, 2
    loop tk_loop

    mov ah, 4Ch
    int 21h
main endp

end main
