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

; break_change: AX = change amount.
; Output: denom_count[0..4] = number of 50, 25, 10, 5, 1 coins/notes.
; AX is DESTROYED (ends as 0). Save the change value in 'change' first.
break_change proc
    push bx
    push cx
    push dx
    push si
    push di
    mov si, offset denoms
    mov di, offset denom_count
    mov cx, NUM_DENOMS
bc_loop:
    mov bx, word ptr [si]   ; BX = current denomination
    xor dx, dx              ; clear DX before DIV (avoid overflow crash)
    div bx                  ; AX = quotient (count), DX = remainder
    mov word ptr [di], ax   ; store count
    mov ax, dx              ; remainder becomes next dividend
    add si, 2
    add di, 2
    loop bc_loop
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    ret
break_change endp
