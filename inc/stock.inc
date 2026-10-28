; ==========================================================
; stock.inc - lookup_stock, take_stock, take_coins, put_coins
; Requires: data.inc, lookup.inc (code_to_offset)
; ==========================================================

.code

; lookup_stock: AL = product code
; Output: CF=0 -> AX = units left of that product ; CF=1 -> not found (AX unchanged)
lookup_stock proc
    push bx
    call code_to_offset
    jc ls_exit
    mov ax, word ptr [product_stock + bx]
ls_exit:
    pop bx
    ret
lookup_stock endp

; take_stock: removes one unit of a product. AL = product code.
; Output: CF=0 -> one unit removed
;         CF=1 -> nothing changed (unknown code, or the product is already at 0)
; AX is preserved.
take_stock proc
    push bx
    call code_to_offset
    jc ts_exit              ; unknown code
    cmp word ptr [product_stock + bx], 1
    jc ts_exit              ; stock is 0 (below 1): refuse, so it never goes below zero
    dec word ptr [product_stock + bx]   ; dec leaves CF = 0
ts_exit:
    pop bx
    ret
take_stock endp

; take_coins: removes the coins listed in denom_count[] from coin_stock[].
; Call it only after break_change reported success. Saves and restores AX and SI.
take_coins proc
    push ax
    push si
    xor si, si
tc_loop:
    mov ax, word ptr [denom_count + si]
    sub word ptr [coin_stock + si], ax
    add si, 2
    cmp si, NUM_DENOMS * 2
    jb tc_loop
    pop si
    pop ax
    ret
take_coins endp

; put_coins: adds a money amount to coin_stock[], split into coins largest first
; (inserted money joins the machine's coins). Input: AX = amount.
; Saves and restores AX, BX, DX and SI.
put_coins proc
    push ax
    push bx
    push dx
    push si
    xor si, si
pu_loop:
    mov bx, word ptr [denoms + si]      ; BX = denomination
    xor dx, dx              ; clear DX before DIV (avoid overflow crash)
    div bx                  ; AX = coins of this denomination, DX = remainder
    add word ptr [coin_stock + si], ax
    mov ax, dx              ; the remainder is split with the next denomination
    add si, 2
    cmp si, NUM_DENOMS * 2
    jb pu_loop
    pop si
    pop dx
    pop bx
    pop ax
    ret
put_coins endp
