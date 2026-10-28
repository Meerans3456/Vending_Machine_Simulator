; ==========================================================
; admin.inc - admin_mode and the routines it offers
; Requires: data.inc, utils.inc, lookup.inc, input.inc, output.inc
; The admin routines may change any register; admin_mode saves and
; restores all of them for its caller.
; ==========================================================

.data
; Admin menu choice 1, 2, ... calls these routines, in this order.
admin_actions   dw admin_view, refill_products, refill_coins, set_product_stock, set_coin_count
ADMIN_CHOICES   equ ($ - admin_actions) / 2     ; number of menu choices

.code

; fill_words: stores AX in CX consecutive words starting at DS:DI.
; Clobbers CX and DI.
fill_words proc
    push es
    push ds
    pop es                  ; ES = DS, because STOSW writes through ES:DI
    cld
    rep stosw
    pop es
    ret
fill_words endp

; refill_products: puts every product back to FULL_STOCK.
refill_products proc
    mov di, offset product_stock
    mov cx, NUM_PRODUCTS
    mov ax, FULL_STOCK
    call fill_words
    mov dx, offset msg_done
    call print_str
    ret
refill_products endp

; refill_coins: puts every denomination back to FULL_COINS.
refill_coins proc
    mov di, offset coin_stock
    mov cx, NUM_DENOMS
    mov ax, FULL_COINS
    call fill_words
    mov dx, offset msg_done
    call print_str
    ret
refill_coins endp

; set_product_stock: asks for a product code and a new stock, and stores it.
; Product code 0 goes back without changing anything.
set_product_stock proc
    mov dx, offset msg_ask_prod
    mov bx, NUM_PRODUCTS
    call ask_int            ; AX = product code (0 = go back)
    call code_to_offset     ; AL = code -> BX = table offset, CF=1 for code 0
    jc sp_exit
    push bx                 ; keep the offset while the stock is asked for
    mov dx, offset msg_ask_stock
    mov bx, MAX_INPUT
    call ask_int            ; AX = new stock
    pop bx
    mov word ptr [product_stock + bx], ax
    mov dx, offset msg_done
    call print_str
sp_exit:
    ret
set_product_stock endp

; set_coin_count: asks for a coin value (one of the denominations) and a new
; number of coins, and stores it. Coin value 0 goes back.
set_coin_count proc
    mov dx, offset msg_ask_coin
    mov bx, MAX_INPUT
    call ask_int            ; AX = coin value (0 = go back)
    or ax, ax
    jz sc_exit
    xor si, si
sc_find:
    cmp ax, word ptr [denoms + si]
    je sc_found
    add si, 2
    cmp si, NUM_DENOMS * 2
    jb sc_find
    mov dx, offset msg_nocoin
    call print_str          ; not one of the denominations
    ret
sc_found:
    mov dx, offset msg_ask_count
    mov bx, MAX_INPUT
    call ask_int            ; AX = new number of coins (ask_int keeps SI)
    mov word ptr [coin_stock + si], ax
    mov dx, offset msg_done
    call print_str
sc_exit:
    ret
set_coin_count endp

; admin_view: shows the stock of every product, the coins, and the sales totals.
admin_view proc
    mov dx, offset msg_hdr_stock
    call print_str
    xor si, si              ; SI = byte offset into the word tables
    mov bx, 1               ; BX = product code
    mov cx, NUM_PRODUCTS
av_prod:
    mov ax, bx
    call print_int          ; code
    mov dx, offset msg_dot
    call print_str          ; ". "
    mov dx, word ptr [product_names + si]
    call print_str          ; name
    mov dx, offset msg_colon
    call print_str          ; ": "
    mov ax, word ptr [product_stock + si]
    call print_int          ; units left
    call newline
    add si, 2
    inc bx
    loop av_prod

    mov dx, offset msg_hdr_coins
    call print_str
    xor si, si
    mov cx, NUM_DENOMS
av_coin:
    mov ax, word ptr [denoms + si]
    call print_int          ; denomination
    mov dx, offset msg_colon
    call print_str
    mov ax, word ptr [coin_stock + si]
    call print_int          ; coins left
    call newline
    add si, 2
    loop av_coin

    call print_summary      ; sales made and total taken
    ret
admin_view endp

; ask_pin: asks for the admin PIN once.
; Anything but the right PIN is wrong, including input that is not a valid number.
; Output: CF=0 if the PIN is right, CF=1 if it is wrong. AX and DX are preserved.
ask_pin proc
    push ax
    push dx
    mov dx, offset msg_pin
    call print_str
    mov dx, PIN_MAX
    call read_int           ; CF=1 (not a valid number) already means a wrong PIN
    jc ap_exit
    sub ax, ADMIN_PIN
    neg ax                  ; CF=1 unless the PIN matched (AX was 0)
ap_exit:
    pop dx                  ; pop does not change the flags
    pop ax
    ret
ask_pin endp

; admin_mode: asks for the PIN, then shows the admin menu until 0 is chosen.
admin_mode proc
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    call ask_pin
    jnc am_menu
    mov dx, offset msg_wrongpin
    call print_str
    call wait_key
    jmp am_exit
am_menu:
    call clear_screen
    mov dx, offset msg_admin_menu
    call print_str
    mov dx, offset msg_admin_ask
    mov bx, ADMIN_CHOICES
    call ask_int            ; AX = choice (0 = leave)
    or ax, ax
    jz am_exit
    shl ax, 1
    mov bx, ax
    call word ptr [admin_actions + bx - 2]
    call wait_key           ; let the admin read the result
    jmp am_menu
am_exit:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
admin_mode endp
