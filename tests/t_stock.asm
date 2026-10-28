; ============================================================
; t_stock.asm - standalone test: stock.inc
; Build  : build t_stock
; Every product starts with FULL_STOCK units (3).
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdr_look      db '--- lookup_stock ---$'
t_hdr_take      db '--- take_stock ---$'
t_nf            db 'not found$'
t_ok            db 'take: ok$'
t_refused       db 'take: refused$'
t_hdr_coin      db '--- take_coins ---$'
t_hdr_put       db '--- put_coins ---$'

include utils.inc
include lookup.inc
include stock.inc

.code

; t_show: AL = product code. Prints "code: units left" or "code: not found".
; Preserves AX.
t_show proc
    push ax
    xor ah, ah
    call print_int          ; the code
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    pop ax                  ; AL = code again (print_char clobbers AX)
    push ax
    call lookup_stock
    jc ts_fail
    call print_int          ; units left (AX)
    jmp ts_done
ts_fail:
    mov dx, offset t_nf
    call print_str
ts_done:
    call newline
    pop ax
    ret
t_show endp

; t_take: AL = product code. Calls take_stock and prints ok or refused.
; Preserves AX.
t_take proc
    push ax
    call take_stock
    mov dx, offset t_ok
    jnc tt_print            ; mov does not change CF
    mov dx, offset t_refused
tt_print:
    call print_str
    call newline
    pop ax
    ret
t_take endp

; t_show_coins: prints the five values of coin_stock, separated by spaces.
t_show_coins proc
    push ax
    push dx
    push si
    xor si, si
tsc_loop:
    mov ax, word ptr [coin_stock + si]
    call print_int
    mov dl, ' '
    call print_char
    add si, 2
    cmp si, NUM_DENOMS * 2
    jb tsc_loop
    call newline
    pop si
    pop dx
    pop ax
    ret
t_show_coins endp

; t_put: AX = money amount. Prints "amount: " and the coin stock after put_coins.
t_put proc
    push ax
    call print_int          ; the amount
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    pop ax                  ; AX = amount again (print_char clobbered it)
    call put_coins
    call t_show_coins
    ret
t_put endp

main proc
    mov ax, @data
    mov ds, ax

    mov dx, offset t_hdr_look
    call print_str
    call newline
    mov al, 1
    call t_show             ; 1: 3
    mov al, 6
    call t_show             ; 6: 3
    mov al, 0
    call t_show             ; 0: not found
    mov al, 9
    call t_show             ; 9: not found

    call newline
    mov dx, offset t_hdr_take
    call print_str
    call newline
    mov al, 2
    call t_take             ; ok
    call t_show             ; 2: 2
    call t_take             ; ok
    call t_show             ; 2: 1
    call t_take             ; ok
    call t_show             ; 2: 0
    call t_take             ; refused: already at 0
    call t_show             ; 2: 0 (did not go below zero)
    mov al, 9
    call t_take             ; refused: unknown code
    call t_show             ; 9: not found
    mov al, 1
    call t_show             ; 1: 3 (other products untouched)

    ; --- take_coins ---
    call newline
    mov dx, offset t_hdr_coin
    call print_str
    call newline
    call t_show_coins       ; 10 10 10 10 10 (FULL_COINS)
    mov word ptr [denom_count + 0], 1
    mov word ptr [denom_count + 2], 0
    mov word ptr [denom_count + 4], 2
    mov word ptr [denom_count + 6], 0
    mov word ptr [denom_count + 8], 3
    call take_coins
    call t_show_coins       ; 9 10 8 10 7
    call take_coins
    call t_show_coins       ; 8 10 6 10 4

    ; --- put_coins ---
    call newline
    mov dx, offset t_hdr_put
    call print_str
    call newline
    mov ax, 35
    call t_put              ; 35 = 25 + 10         -> 8 11 7 10 4
    mov ax, 64
    call t_put              ; 64 = 50 + 10 + 4 x 1 -> 9 11 8 10 8

    mov ah, 4Ch
    int 21h
main endp

end main
