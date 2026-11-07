; ============================================================
; t_math.asm - standalone test: menu.inc and math.inc (display_menu, calc_change, break_change)
; Build  : build t_math
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdrm          db '--- display_menu (product 3 is sold out) ---$'
t_hdrcc         db '--- calc_change (payment - price) ---$'
t_hdrbc         db '--- break_change (counts for 100 50 20 10 5 1) ---$'
t_cf0           db 'CF=0 change = $'
t_cf1           db 'CF=1 shortfall = $'
t_calc          dw 50, 25,  10, 25,  25, 25     ; payment, price pairs
t_ncalc         equ ($ - t_calc) / 4     ; number of pairs
t_changes       dw 35, 0, 100, 99, 964
t_nchanges      equ ($ - t_changes) / 2  ; number of values
t_axafter       db ' AX after: $'
t_nochange      db 'no exact change $'
t_hdrlim        db '--- break_change with limited coins ---$'

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

; t_break: AX = change amount to try. Prints "amount: counts" or
; "amount: no exact change", then AX after the call (it must be unchanged).
t_break proc
    push ax                 ; keep the amount
    call print_int          ; the amount
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    pop ax                  ; AX = amount again (print_char clobbered it)
    call break_change
    jc tb_fail
    call t_show_counts      ; it saves and restores AX
    jmp tb_after
tb_fail:
    push ax
    mov dx, offset t_nochange
    call print_str
    pop ax
tb_after:
    push ax                 ; print_str clobbers AX
    mov dx, offset t_axafter
    call print_str
    pop ax
    call print_int          ; must equal the amount (AX is preserved, also on failure)
    call newline
    ret
t_break endp

main proc
    mov ax, @data
    mov ds, ax

    ; --- display_menu ---
    mov dx, offset t_hdrm
    call print_str
    call newline
    mov word ptr [product_stock + 4], 0   ; make product 3 sold out
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
    call t_break
    add si, 2
    loop tk_loop

    ; --- break_change with limited coins ---
    call newline
    mov dx, offset t_hdrlim
    call print_str
    call newline
    mov word ptr [coin_stock + 10], 2           ; only two 1-coins left
    mov ax, 2
    call t_break                                ; 0 0 0 0 0 2
    mov ax, 3
    call t_break                                ; no exact change
    mov word ptr [coin_stock + 10], FULL_COINS ; 1-coins restored
    mov word ptr [coin_stock + 4], 0            ; no 20-coins
    mov ax, 30
    call t_break                                ; 0 0 0 3 0 0 (three 10-coins)
    mov ax, 35
    call t_break                                ; 0 0 0 3 1 0

    mov ah, 4Ch
    int 21h
main endp

end main
