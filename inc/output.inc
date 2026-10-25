; ==========================================================
; output.inc - print_change, print_receipt
; Requires: data.inc, utils.inc, input.inc (lookup_name)
; ==========================================================

.code

; print_change: prints the change total and its breakdown.
; Input : 'change' variable = total change
;         denom_count[0..4] = counts of 50, 25, 10, 5, 1 (from break_change)
; Output: "Change: <total>", then one "<count> x <denom>" line per
;         denomination whose count is non-zero.
; Saves and restores every register it uses.
print_change proc
    push ax
    push bx
    push cx
    push dx
    push si
    push di

    mov dx, offset msg_change
    call print_str
    mov ax, word ptr change
    call print_int
    call newline

    mov si, offset denoms
    mov di, offset denom_count
    mov cx, NUM_DENOMS
pc_loop:
    mov ax, word ptr [di]   ; count for this denomination
    cmp ax, 0
    je pc_next              ; skip zero counts
    call print_int          ; count
    mov dx, offset msg_x
    call print_str          ; " x "
    mov ax, word ptr [si]
    call print_int          ; denomination
    call newline
pc_next:
    add si, 2
    add di, 2
    loop pc_loop

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
print_change endp

; print_receipt: prints a receipt for a completed sale.
; Input : variables sel_code, price, payment, change, and denom_count[]
;         (set by main / break_change before the call)
; Output: banner, item name, price, paid, then print_change's
;         "Change:" line and breakdown, then a footer line.
; Saves and restores AX and DX (print_change saves its own registers).
print_receipt proc
    push ax
    push dx

    mov dx, offset msg_rcpt_top
    call print_str

    mov dx, offset msg_r_item
    call print_str
    mov al, sel_code
    call lookup_name        ; DX = name string
    call print_str
    call newline

    mov dx, offset msg_r_price
    call print_str
    mov ax, word ptr price
    call print_int
    call newline

    mov dx, offset msg_r_paid
    call print_str
    mov ax, word ptr payment
    call print_int          ; no newline: print_change starts with CRLF
    call print_change

    mov dx, offset msg_rcpt_end
    call print_str

    pop dx
    pop ax
    ret
print_receipt endp
