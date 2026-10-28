; ==========================================================
; math.inc - calc_change, break_change
; Requires: data.inc
; ==========================================================

.code

; calc_change: AX = payment, BX = price
; Output: CF=0 -> AX = change (payment - price)
;         CF=1 -> AX = shortfall (price - payment)
; Compares BEFORE subtracting so the result can never wrap around.
calc_change proc
    cmp ax, bx
    jb cc_short             ; payment < price (unsigned)
    sub ax, bx
    clc
    ret
cc_short:
    push dx
    mov dx, bx
    sub dx, ax              ; shortfall = price - payment
    mov ax, dx
    pop dx
    stc                     ; (pop does not change flags; set CF last)
    ret
calc_change endp

; break_change: works out which coins to give as change.
; Input : AX = change amount
; Output: denom_count[] = coins to give of each denomination (50, 25, 10, 5, 1),
;         largest first, never more than coin_stock[] holds.
;         CF=0 -> the change is made exactly
;         CF=1 -> the machine lacks the coins (denom_count[] must not be used)
; It only calculates; take_coins removes the coins from the stock. AX is preserved.
break_change proc
    push ax
    push bx
    push cx
    push dx
    push si
    xor si, si              ; SI = byte offset into the three word tables
bc_loop:
    mov bx, word ptr [denoms + si]      ; BX = denomination
    push ax                 ; the amount still to cover
    xor dx, dx              ; clear DX before DIV (avoid overflow crash)
    div bx                  ; AX = coins that would fit
    mov cx, word ptr [coin_stock + si]  ; CX = coins in stock
    cmp ax, cx
    jbe bc_use              ; enough coins in stock
    mov ax, cx              ; otherwise use all we have
bc_use:
    mov word ptr [denom_count + si], ax
    mul bx                  ; AX = value of those coins (DX = 0: it never exceeds the amount)
    mov cx, ax
    pop ax                  ; amount still to cover, before this denomination
    sub ax, cx              ; amount still to cover, after it
    add si, 2
    cmp si, NUM_DENOMS * 2
    jb bc_loop
    neg ax                  ; CF=1 if an amount is left over, CF=0 if nothing is
    pop si
    pop dx
    pop cx
    pop bx
    pop ax                  ; give the caller's AX back (pop keeps CF)
    ret
break_change endp
