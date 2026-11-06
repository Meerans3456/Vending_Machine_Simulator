; ============================================================
; t_file.asm - standalone test: file.inc (save_state, load_state)
; Build  : build t_file
; Uses its own file, T_STATE.DAT, so your real VENDING.DAT is never touched.
; Fully automatic: compare the printed lines with the comments in main.
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_name          db 'T_STATE.DAT', 0     ; same length as the real save file name (12 bytes with the 0)
t_hdr1          db '--- 1. no save file: load_state changes nothing ---$'
t_hdr2          db '--- 2. save, scramble, load: the saved values come back ---$'
t_hdr3          db '--- 3. a file of the wrong size is ignored ---$'
t_lbl_prod      db 'products: $'
t_lbl_coins     db 'coins:    $'
t_lbl_sales     db 'sales:    $'     ; sales_count and sales_total
t_lbl_admin     db 'admin:    $'     ; admin_code and admin_pin

include utils.inc
include file.inc

.code

; t_show_words: SI = table, CX = number of words. Prints them separated by spaces.
t_show_words proc
    push ax
    push dx
    push si
tsw_loop:
    mov ax, word ptr [si]
    call print_int
    mov dl, ' '
    call print_char
    add si, 2
    loop tsw_loop
    call newline
    pop si
    pop dx
    pop ax
    ret
t_show_words endp

; t_show_state: prints product_stock, coin_stock, sales_count, sales_total, admin_code and admin_pin.
t_show_state proc
    push cx
    push dx
    push si
    mov dx, offset t_lbl_prod
    call print_str
    mov si, offset product_stock
    mov cx, NUM_PRODUCTS
    call t_show_words
    mov dx, offset t_lbl_coins
    call print_str
    mov si, offset coin_stock
    mov cx, NUM_DENOMS
    call t_show_words
    mov dx, offset t_lbl_sales
    call print_str
    mov si, offset sales_count
    mov cx, 2
    call t_show_words
    mov dx, offset t_lbl_admin
    call print_str
    mov si, offset admin_code
    mov cx, 2
    call t_show_words
    pop si
    pop dx
    pop cx
    ret
t_show_state endp

; t_scramble: sets every product, coin, sales and admin value to 9 (ES must equal DS).
t_scramble proc
    mov ax, 9
    cld
    mov di, offset product_stock
    mov cx, NUM_PRODUCTS
    rep stosw
    mov di, offset coin_stock
    mov cx, NUM_DENOMS
    rep stosw
    mov di, offset sales_count
    mov cx, 2
    rep stosw
    mov di, offset admin_code
    mov cx, 2
    rep stosw
    ret
t_scramble endp

; t_delete: deletes the file named in state_file (an error, such as no such file, is fine).
t_delete proc
    mov ah, 41h
    mov dx, offset state_file
    int 21h
    ret
t_delete endp

; t_wrongsize: creates the file with only 3 bytes, far fewer than a real save.
t_wrongsize proc
    mov ah, 3Ch
    xor cx, cx
    mov dx, offset state_file
    int 21h
    mov bx, ax
    mov ah, 40h
    mov cx, 3
    mov dx, offset state_file
    int 21h
    mov ah, 3Eh
    int 21h
    ret
t_wrongsize endp

main proc
    mov ax, @data
    mov ds, ax
    mov es, ax              ; STOSW and MOVSB write through ES:DI

    ; use the test's own file name instead of VENDING.DAT
    mov si, offset t_name
    mov di, offset state_file
    mov cx, 12
    cld
    rep movsb
    call t_delete           ; make sure T_STATE.DAT does not exist

    mov dx, offset t_hdr1
    call print_str
    call newline
    call load_state
    call t_show_state       ; products: 3 3 3 3 3 3 / coins: 10 10 10 10 10 / sales: 0 0 / admin: 31415 1234

    call newline
    mov dx, offset t_hdr2
    call print_str
    call newline
    mov word ptr [product_stock + 0], 0
    mov word ptr [product_stock + 2], 2
    mov word ptr [coin_stock + 0], 7
    mov word ptr sales_count, 4
    mov word ptr sales_total, 65
    mov word ptr admin_code, 12345
    mov word ptr admin_pin, 4321
    call t_show_state       ; to be saved: 0 2 3 3 3 3 / 7 10 10 10 10 / sales: 4 65 / admin: 12345 4321
    call save_state
    call t_scramble
    call t_show_state       ; scrambled: all 9, sales 9 9, admin 9 9
    call load_state
    call t_show_state       ; back again: 0 2 3 3 3 3 / 7 10 10 10 10 / sales: 4 65 / admin: 12345 4321

    call newline
    mov dx, offset t_hdr3
    call print_str
    call newline
    call t_wrongsize
    call t_scramble
    call load_state
    call t_show_state       ; still all 9 (sales 9 9, admin 9 9): the 3-byte file was ignored
    call t_delete           ; leave nothing behind

    mov ah, 4Ch
    int 21h
main endp

end main
