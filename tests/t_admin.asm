; ============================================================
; t_admin.asm - standalone test: admin.inc
; Build  : build t_admin
; Parts 1-4 are automatic (press a key after each screen).
; Parts 5-7, 9 and 10 ask you to type values; follow the header. Part 8 is automatic.
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
t_hdr6          db '--- 6. set_coin_count: type 20, then 4 ---$'
t_hdr6v         db '--- after set_coin_count: the 20-coins should be 4 ---$'
t_hdr7          db '--- 7. ask_pin (keys show as *): run it three times: 0, then 9999, then 1234 ---$'
t_pincan        db CR, LF, 'PIN cancelled (0 typed)$'
t_hdr8          db '--- 8. admin lock logic, with fake clock values (automatic) ---$'
t_l1            db 'left, started 1000 len 300, now 1100 (expect 200): $'
t_l2            db 'left, now 1300 (expect 0, lock cleared): $'
t_l3            db 'left, no lock running, now 1100 (expect 0): $'
t_l4            db 'left, midnight case, 30 s after the start (expect 30): $'
t_l5            db 'left, midnight case, 12 s after the start (expect 48): $'
t_l6            db 'level after the first lock (expect 1; 1 minute): $'
t_l7            db 'level after a lock at the top level (expect 5; 60 minutes): $'
t_hdr9          db '--- 9. change_admin_code: type 12345 twice ---$'
t_hdr9v         db 'admin_code is now (expect 12345): $'
t_hdr10         db '--- 10. change_admin_pin: type 4321 twice (shown as *) ---$'
t_hdr10v        db 'admin_pin is now (expect 4321): $'
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
    mov word ptr [coin_stock + 0], 0        ; no 100-coins
    mov word ptr [coin_stock + 2], 3        ; three 50-coins
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
    or ax, ax               ; AX = 0 means 0 was typed (go back)
    jnz tp_print
    mov dx, offset t_pincan
tp_print:
    call print_str
    call newline
    call wait_key

    call clear_screen
    mov dx, offset t_hdr8
    call print_str
    call newline
    mov word ptr lock_active, 1
    mov word ptr lock_start_lo, 1000
    mov word ptr lock_start_hi, 0
    mov word ptr lock_len, 300
    mov dx, offset t_l1
    call print_str
    mov ax, 1100
    xor dx, dx
    call lock_left_at
    call print_int
    call newline
    mov dx, offset t_l2
    call print_str
    mov ax, 1300
    xor dx, dx
    call lock_left_at
    call print_int
    call newline
    mov dx, offset t_l3
    call print_str
    mov ax, 1100
    xor dx, dx
    call lock_left_at       ; lock_active was cleared by the expiry above
    call print_int
    call newline
    mov word ptr lock_active, 1
    mov word ptr lock_start_lo, 5176h   ; 86390 = 10 s before midnight
    mov word ptr lock_start_hi, 1
    mov word ptr lock_len, 60
    mov dx, offset t_l4
    call print_str
    mov ax, 20              ; 20 s after midnight = 30 s after the start
    xor dx, dx
    call lock_left_at
    call print_int
    call newline
    mov dx, offset t_l5
    call print_str
    mov ax, 2               ; 2 s after midnight = 12 s after the start
    xor dx, dx
    call lock_left_at
    call print_int
    call newline
    mov word ptr lock_level, 0
    call start_lock         ; says: locked for 1 minute(s)
    mov dx, offset t_l6
    call print_str
    mov ax, word ptr lock_level
    call print_int
    call newline
    mov word ptr lock_level, NUM_LOCKS - 1
    call start_lock         ; says: locked for 60 minute(s)
    mov dx, offset t_l7
    call print_str
    mov ax, word ptr lock_level
    call print_int
    call newline
    call wait_key

    call clear_screen
    mov dx, offset t_hdr9
    call print_str
    call change_admin_code
    call newline
    mov dx, offset t_hdr9v
    call print_str
    mov ax, word ptr admin_code
    call print_int
    call newline
    mov dx, offset t_hdr10
    call print_str
    call change_admin_pin
    call newline
    mov dx, offset t_hdr10v
    call print_str
    mov ax, word ptr admin_pin
    call print_int
    call newline

    mov ah, 4Ch
    int 21h
main endp

end main
