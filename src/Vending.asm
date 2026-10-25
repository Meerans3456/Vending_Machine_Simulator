; ============================================================
; VENDING MACHINE SIMULATOR - main program
; Target : 8086 DOS (16-bit), assembled with JWasm, run in DOSBox
; Build  : build vending        (or: JWasm.exe -0 -mz -Fo vending.exe vending.asm)
;
; Module files (pasted in at assembly time by 'include'):
;   data.inc    tables, messages, variables      (must come first)
;   utils.inc   print_str, newline, print_char, print_int, clear_screen, wait_key
;   input.inc   read_int, read_product, lookup_price, lookup_name, read_payment
;   menu.inc    display_menu
;   math.inc    calc_change, break_change
;   output.inc  print_change, print_receipt
; ============================================================
.model small
.stack 100h

include data.inc
include utils.inc
include input.inc
include menu.inc
include math.inc
include output.inc

.code

; ---------- Main ----------
; One pass of main_loop = one purchase attempt.
; 'payment' is the running total of money inserted for this purchase.
main proc
    mov ax, @data          ; mandatory: initialize DS
    mov ds, ax

main_loop:
    call clear_screen      ; fresh screen for every transaction
    call display_menu
    call read_product      ; AL = code (0 = exit); re-prompts on bad input
    jc main_loop           ; safety net; read_product never returns CF=1
    cmp al, 0
    jne main_buy
    jmp main_done          ; trampoline: main_done is out of short-jump range

main_buy:
    mov sel_code, al
    call lookup_price      ; AL = code -> AX = price
    jc main_loop           ; safety net; code was already validated
    mov price, ax

    mov dx, offset msg_price
    call print_str
    mov ax, word ptr price
    call print_int
    call newline

    mov word ptr payment, 0 ; nothing inserted yet

pay_loop:
    call read_payment      ; AX = money inserted now; 0 = cancel
    cmp ax, 0
    je main_cancel
    ; The total is below the price (<= 35) before every insertion, and each
    ; insertion is <= 999, so this addition can never overflow 16 bits.
    add word ptr payment, ax

    mov ax, word ptr payment
    mov bx, word ptr price
    call calc_change       ; CF=0: AX = change; CF=1: AX = shortfall
    jc main_short

    mov change, ax         ; save BEFORE break_change (it destroys AX)
    call break_change
    call print_receipt
    call wait_key          ; let the user read the receipt
    jmp main_loop

main_short:
    mov shortfall, ax
    mov dx, offset msg_short
    call print_str
    mov ax, word ptr shortfall
    call print_int
    call newline
    jmp pay_loop           ; ask for more money (not back to the menu)

main_cancel:
    mov dx, offset msg_cancel
    call print_str
    mov ax, word ptr payment
    cmp ax, 0
    je main_cancel_end     ; nothing inserted -> nothing to refund
    mov dx, offset msg_refund
    call print_str
    mov ax, word ptr payment
    call print_int
main_cancel_end:
    call newline
    call wait_key
    jmp main_loop

main_done:
    mov dx, offset msg_bye
    call print_str
    call newline
    mov ah, 4Ch            ; exit to DOS
    int 21h
main endp

end main
