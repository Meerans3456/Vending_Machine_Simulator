; ============================================================
; VENDING MACHINE SIMULATOR - main program
; Target : 8086 DOS (16-bit), assembled with JWasm, run in DOSBox
; Build  : .\build.bat vending   (JWasm; run from the project folder)
;
; Module files (pasted in at assembly time by 'include'):
;   data.inc    tables, messages, variables      (must come first)
;   utils.inc   print_str, newline, print_char, print_int, clear_screen, wait_key
;   lookup.inc  code_to_offset, lookup_price, lookup_name
;   stock.inc   lookup_stock, take_stock
;   input.inc   read_int, ask_int, read_product
;   menu.inc    display_menu
;   math.inc    calc_change, break_change
;   output.inc  print_change, print_receipt, print_summary
;   admin.inc   admin_mode (PIN-protected restock and view)
;   file.inc    load_state, save_state (stock and coins survive between runs)
;   bridge.inc  bridge mode (vending.exe /g): commands and replies through CMD.TXT and OUT.TXT
; ============================================================
.model small
.stack 100h

include data.inc
include utils.inc
include lookup.inc
include stock.inc
include input.inc
include menu.inc
include math.inc
include output.inc
include admin.inc
include file.inc
include bridge.inc

.code

; ---------- Main ----------
; One pass of main_loop = one purchase attempt.
; 'payment' is the running total of money inserted for this purchase.
main proc
    mov ax, @data          ; mandatory: initialize DS
    mov ds, ax
    call detect_bridge     ; ES is still the program prefix here: looks for /g on the command line
    call load_state        ; carry on with the stock and coins saved by the last run
    cmp byte ptr bridge_flag, 0
    je main_loop
    call bridge_run        ; bridge mode: commands come from CMD.TXT, replies go to OUT.TXT
    mov ax, 4C00h          ; exit to DOS
    int 21h

main_loop:
    call save_state        ; keep the save file up to date (after every sale or admin change)
    call clear_screen      ; fresh screen for every transaction
    call display_menu
    call read_product      ; AX = 0 (exit), admin_code (admin mode) or a product in stock
    cmp ax, word ptr admin_code
    jne main_notadmin
    call admin_mode
    jmp main_loop
main_notadmin:
    cmp ax, 0
    jne main_buy
    jmp main_done          ; trampoline: main_done is out of short-jump range

main_buy:
    mov sel_code, al
    call lookup_price      ; AL = code -> AX = price (the code is already valid)
    mov price, ax

    mov dx, offset msg_price
    call print_str
    mov ax, word ptr price
    call print_int
    call newline

    mov word ptr payment, 0 ; nothing inserted yet

pay_loop:
    mov dx, offset msg_pay
    mov bx, MAX_INPUT
    call ask_int           ; AX = money inserted now; 0 = cancel
    cmp ax, 0
    je main_cancel
    ; The total is below the price before every insertion, and each insertion
    ; is at most MAX_INPUT, so this addition can never overflow 16 bits.
    add word ptr payment, ax

    mov ax, word ptr payment
    mov bx, word ptr price
    call calc_change       ; CF=0: AX = change; CF=1: AX = shortfall
    jc main_short

    mov change, ax         ; print_receipt reads this variable
    call break_change
    jc main_nochange        ; CF=1: the machine lacks the coins for exact change
    call print_receipt
    mov al, sel_code
    call take_stock         ; one unit leaves the machine
    call take_coins         ; the change coins leave the machine
    mov ax, word ptr payment
    call put_coins          ; the inserted money joins the machine's coins
    inc word ptr sales_count        ; record the completed sale
    mov ax, word ptr price
    add word ptr sales_total, ax
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

main_nochange:
    mov dx, offset msg_nochange
    call print_str
    jmp main_refund        ; nothing is sold: give the money back

main_cancel:
    mov dx, offset msg_cancel
    call print_str
main_refund:               ; shared by cancel and "no exact change"
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
    call print_summary
    mov dx, offset msg_bye
    call print_str
    call newline
    mov ah, 4Ch            ; exit to DOS
    int 21h
main endp

end main
