; ==========================================================
; admin.inc - admin_mode and the routines it offers
; Requires: data.inc, utils.inc, lookup.inc, input.inc, output.inc
; The admin routines may change any register; admin_mode saves and
; restores all of them for its caller.
; ==========================================================

.data
; Admin menu choice 1, 2, ... calls these routines, in this order.
admin_actions   dw admin_view, refill_products, refill_coins, set_product_stock, set_coin_count
                dw change_admin_code, change_admin_pin
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
    call show_product_stock ; list the products so the admin sees the codes
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

; show_product_stock: prints "Product stock:" and one line per product
; (code, name, units left), so the admin can see which code is which.
; Clobbers AX, BX, CX, DX, SI.
show_product_stock proc
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
    call print_name_col     ; name, padded to the name column
    mov dx, offset msg_dash
    call print_str          ; " - "
    mov ax, word ptr [product_stock + si]
    call print_int3         ; units left, right-aligned
    call newline
    add si, 2
    inc bx
    loop av_prod
    ret
show_product_stock endp

; admin_view: shows the stock of every product, the coins, and the sales totals.
admin_view proc
    call show_product_stock ; product codes, names and stock

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

; get_seconds: reads the DOS clock.
; Output: DX:AX = seconds since midnight (0..86399). Other registers kept.
get_seconds proc
    push bx
    push cx
    mov ah, 2Ch             ; DOS: CH = hours, CL = minutes, DH = seconds
    int 21h
    mov bl, dh
    xor bh, bh              ; BX = seconds
    mov al, ch
    xor ah, ah
    mov dx, 3600
    mul dx                  ; DX:AX = hours * 3600
    push ax
    push dx                 ; keep the hours part
    mov al, cl
    xor ah, ah
    mov dx, 60
    mul dx                  ; AX = minutes * 60
    add ax, bx              ; + seconds
    pop dx
    pop cx                  ; DX:CX = hours part
    add ax, cx
    adc dx, 0               ; DX:AX = the total
    pop cx
    pop bx
    ret
get_seconds endp

; lock_left_at: DX:AX = the time now (seconds since midnight).
; Output: AX = seconds of the lock still left (0 = not locked).
; A lock that has run out is cleared. The wrap past midnight is handled.
lock_left_at proc
    push bx
    push dx
    cmp word ptr lock_active, 0
    je ll_none              ; no lock running
    sub ax, word ptr lock_start_lo
    sbb dx, word ptr lock_start_hi  ; DX:AX = time since the lock started
    jnc ll_pos              ; no borrow: the clock did not pass midnight
    add ax, 5180h
    adc dx, 1               ; add 86400 seconds (one day)
ll_pos:
    or dx, dx
    jnz ll_expired          ; more than 65535 seconds: long over
    mov bx, word ptr lock_len
    cmp ax, bx
    jae ll_expired          ; the lock time has passed
    sub bx, ax
    mov ax, bx              ; AX = seconds left
    jmp ll_done
ll_expired:
    mov word ptr lock_active, 0
ll_none:
    xor ax, ax
ll_done:
    pop dx
    pop bx
    ret
lock_left_at endp

; lock_remaining: AX = seconds of the admin lock still left (0 = not locked).
lock_remaining proc
    push dx
    call get_seconds
    call lock_left_at
    pop dx
    ret
lock_remaining endp

; start_lock: starts a lock whose length depends on lock_level (1, 5, 10,
; 20, 30, then 60 minutes), makes the next lock longer, clears pin_fails
; and says how long the admin is locked.
start_lock proc
    push ax
    push bx
    push dx
    mov bx, word ptr lock_level
    shl bx, 1
    mov ax, word ptr [lock_times + bx]
    mov word ptr lock_len, ax
    call get_seconds
    mov word ptr lock_start_lo, ax
    mov word ptr lock_start_hi, dx
    mov word ptr lock_active, 1
    mov word ptr pin_fails, 0
    cmp word ptr lock_level, NUM_LOCKS - 1
    jae sl_say              ; already at the longest lock time
    inc word ptr lock_level
sl_say:
    mov dx, offset msg_locknow
    call print_str
    mov ax, word ptr lock_len
    xor dx, dx
    mov bx, 60
    div bx                  ; AX = minutes
    call print_int
    mov dx, offset msg_minutes
    call print_str
    call newline
    pop dx
    pop bx
    pop ax
    ret
start_lock endp

; show_locked: AX = seconds left; prints "Admin is locked. Try again in M min S sec."
show_locked proc
    push ax
    push bx
    push dx
    xor dx, dx
    mov bx, 60
    div bx                  ; AX = minutes, DX = seconds
    push dx                 ; keep the seconds
    push ax                 ; keep the minutes
    mov dx, offset msg_locked
    call print_str
    pop ax
    call print_int
    mov dx, offset msg_min
    call print_str
    pop ax
    call print_int
    mov dx, offset msg_sec
    call print_str
    call newline
    pop dx
    pop bx
    pop ax
    ret
show_locked endp

; ask_pin: asks for the PIN; the keys are shown as *.
; Output: CF=0 the PIN is right.
;         CF=1 and AX=0: 0 was typed (go back).
;         CF=1 and AX=1: wrong PIN (or not a valid number).
ask_pin proc
    push dx
    mov dx, offset msg_pin
    call print_str
    mov byte ptr echo_mask, '*'   ; hide the keys (mov does not change flags)
    mov dx, PIN_MAX
    call read_int
    mov byte ptr echo_mask, 0
    jc ap_wrong             ; not a valid number: a wrong PIN
    or ax, ax
    jz ap_cancel            ; 0 = go back
    cmp ax, word ptr admin_pin
    jne ap_wrong
    clc                     ; the PIN matched
    jmp ap_exit
ap_cancel:
    xor ax, ax
    stc
    jmp ap_exit
ap_wrong:
    mov ax, 1
    stc
ap_exit:
    pop dx                  ; pop does not change the flags
    ret
ask_pin endp

; change_admin_code: the admin types a new secret number twice (0 cancels).
; The new code must be at least ADMIN_CODE_MIN so it never clashes with a product.
change_admin_code proc
    push ax
    push bx
    push dx
    mov dx, offset msg_new_code
    mov bx, 0FFFFh
    call ask_int
    or ax, ax
    jz cc_exit              ; 0 = cancel
    cmp ax, ADMIN_CODE_MIN
    jb cc_short
    push ax                 ; keep the first entry
    mov dx, offset msg_again
    mov bx, 0FFFFh
    call ask_int
    pop bx                  ; BX = first entry
    cmp ax, bx
    jne cc_nomatch
    mov word ptr admin_code, ax
    mov dx, offset msg_done
    jmp cc_say
cc_short:
    mov dx, offset msg_code_short
    jmp cc_say
cc_nomatch:
    mov dx, offset msg_nomatch
cc_say:
    call print_str
cc_exit:
    pop dx
    pop bx
    pop ax
    ret
change_admin_code endp

; change_admin_pin: the admin types a new PIN twice, shown as * (0 cancels).
; The new PIN must be PIN_MIN..PIN_MAX.
change_admin_pin proc
    push ax
    push bx
    push dx
    mov byte ptr echo_mask, '*'   ; hide the keys
    mov dx, offset msg_new_pin
    mov bx, PIN_MAX
    call ask_int
    or ax, ax
    jz cp_exit              ; 0 = cancel
    cmp ax, PIN_MIN
    jb cp_short
    push ax                 ; keep the first entry
    mov dx, offset msg_again
    mov bx, PIN_MAX
    call ask_int
    pop bx                  ; BX = first entry
    cmp ax, bx
    jne cp_nomatch
    mov word ptr admin_pin, ax
    mov dx, offset msg_done
    jmp cp_say
cp_short:
    mov dx, offset msg_pin_range
    jmp cp_say
cp_nomatch:
    mov dx, offset msg_nomatch
cp_say:
    call print_str
cp_exit:
    mov byte ptr echo_mask, 0     ; keys are shown again
    pop dx
    pop bx
    pop ax
    ret
change_admin_pin endp

; admin_mode: checks the lock, asks for the PIN (0 goes back, a wrong PIN asks
; again and counts), then shows the admin menu until 0 is chosen.
admin_mode proc
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    call lock_remaining     ; AX = seconds of lock left
    or ax, ax
    jz am_pin
    call show_locked        ; still locked: say how long and leave
    call wait_key
    jmp am_exit
am_pin:
    call ask_pin
    jnc am_ok               ; right PIN
    or ax, ax
    jz am_exit              ; 0 typed: back to the menu
    inc word ptr pin_fails
    mov ax, PIN_TRIES
    sub ax, word ptr pin_fails  ; AX = tries left
    jbe am_lock             ; none left: lock the admin
    push ax
    mov dx, offset msg_wrongpin
    call print_str
    mov dx, offset msg_tries
    call print_str
    pop ax
    call print_int
    call newline
    jmp am_pin              ; ask for the PIN again (not the admin code)
am_lock:
    call start_lock
    call wait_key
    jmp am_exit
am_ok:
    mov word ptr pin_fails, 0   ; a right PIN clears the count and the lock level
    mov word ptr lock_level, 0
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
