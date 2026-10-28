; ============================================================
; t_admin.asm - standalone test: admin.inc
; Build  : build t_admin
; Parts 1-4 are automatic (press a key after each screen).
; Parts 5-7 ask you to type values; follow the header.
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdr1          db '--- 1. admin_view: starting state (everything full) ---$'
t_hdr2          db '--- 2. some stock and coins changed by hand ---$'
t_hdr3          db '--- 3. after refill_products (coins stay as they were) ---$'
t_hdr4          db '--- 4. after refill_coins (everything full again) ---$'
t_hdr5          db '--- 5. set_product_stock: type 2, then 7 ---$'
t_hdr5v         db '--- after set_product_stock: product 2 should be 7 ---$'
t_hdr6          db '--- 6. set_coin_count: type 25, then 4 ---$'
t_hdr6v         db '--- after set_coin_count: the 25-coins should be 4 ---$'
t_hdr7          db '--- 7. ask_pin: type anything; only the right PIN is accepted ---$'
t_pinok         db CR, LF, 'PIN accepted$'
t_pinbad        db CR, LF, 'PIN rejected$'

include utils.inc
include lookup.inc
include stock.inc
include input.inc
include output.inc
include admin.inc

.code

; t_view: DX = header string. Clears the screen, prints the header and the
; admin view, then waits for a key.
t_view proc
    call clear_screen
    call print_str
    call admin_view
    call wait_key
    ret
t_view endp

main proc
    mov ax, @data
    mov ds, ax

    mov dx, offset t_hdr1
    call t_view

    mov word ptr [product_stock + 0], 0     ; Chips sold out
    mov word ptr [product_stock + 2], 1     ; one Soda left
    mov word ptr [coin_stock + 0], 0        ; no 50-coins
    mov word ptr [coin_stock + 2], 3        ; three 25-coins
    mov dx, offset t_hdr2
    call t_view

    call refill_products
    mov dx, offset t_hdr3
    call t_view

    call refill_coins
    mov dx, offset t_hdr4
    call t_view

    call clear_screen
    mov dx, offset t_hdr5
    call print_str
    call set_product_stock
    mov dx, offset t_hdr5v
    call t_view

    call clear_screen
    mov dx, offset t_hdr6
    call print_str
    call set_coin_count
    mov dx, offset t_hdr6v
    call t_view

    call clear_screen
    mov dx, offset t_hdr7
    call print_str
    call ask_pin
    mov dx, offset t_pinok
    jnc tp_print            ; mov does not change CF
    mov dx, offset t_pinbad
tp_print:
    call print_str
    call newline

    mov ah, 4Ch
    int 21h
main endp

end main
