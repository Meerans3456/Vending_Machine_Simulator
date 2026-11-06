; ==========================================================
; menu.inc - display_menu
; Requires: data.inc, utils.inc
; ==========================================================

.code

; display_menu: prints the menu. Each product line ("1. Chips - 15") is
; built from product_names and product_prices, so the tables are the only
; place where product data is stored.
; The names and prices are padded so every column lines up.
; Saves and restores every register it uses.
display_menu proc
    push ax
    push bx
    push cx
    push dx
    push si

    mov dx, offset msg_menu_top
    call print_str

    xor si, si              ; SI = byte offset into the word tables
    mov bx, 1               ; BX = product code shown to the user
    mov cx, NUM_PRODUCTS
dm_loop:
    mov ax, bx
    call print_int          ; code
    mov dx, offset msg_dot
    call print_str          ; ". "
    mov dx, word ptr [product_names + si]
    call print_name_col     ; name, padded to the name column
    mov dx, offset msg_dash
    call print_str          ; " - "
    mov ax, word ptr [product_prices + si]
    call print_int3         ; price, right-aligned
    cmp word ptr [product_stock + si], 0
    jne dm_endline
    mov dx, offset msg_sold_tag
    call print_str          ; " (sold out)"
dm_endline:
    call newline
    add si, 2
    inc bx
    loop dm_loop

    mov dx, offset msg_menu_exit
    call print_str

    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
display_menu endp
