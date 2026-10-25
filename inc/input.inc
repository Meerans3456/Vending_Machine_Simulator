; ==========================================================
; input.inc - read_int, read_product, lookup_price, lookup_name, read_payment
; Requires: data.inc, utils.inc
; ==========================================================

.code

; read_int: reads a decimal number from the keyboard until Enter.
; Output: AX = value (0..999), CF=0 ; CF=1 if invalid (AX undefined)
; Invalid = empty input, non-digit character, or value > 999.
; On invalid input the rest of the line is discarded.
read_int proc
    push bx
    push cx
    push dx
    xor bx, bx              ; BX = accumulator
    xor cx, cx              ; CX = number of digits typed
ri_loop:
    mov ah, 01h             ; read char with echo -> AL
    int 21h
    cmp al, 0Dh             ; Enter?
    je ri_enter
    cmp al, '0'
    jb ri_bad
    cmp al, '9'
    ja ri_bad
    sub al, '0'
    xor ah, ah              ; AX = digit
    push ax                 ; save digit
    mov ax, bx
    mov dx, 10
    mul dx                  ; DX:AX = BX*10 (max 9990, DX = 0)
    pop dx                  ; DX = digit
    add ax, dx
    mov bx, ax
    inc cx
    cmp bx, 999
    ja ri_bad
    jmp ri_loop
ri_enter:
    cmp cx, 0               ; no digits typed -> invalid
    je ri_fail
    mov ax, bx
    clc
    jmp ri_exit
ri_bad:                     ; flush the rest of the line
    mov ah, 01h
    int 21h
    cmp al, 0Dh
    jne ri_bad
ri_fail:
    stc
ri_exit:
    pop dx
    pop cx
    pop bx
    ret
read_int endp

; read_product: prompts for a product code and validates it.
; Re-prompts internally on invalid input.
; Output: AL = code (0 = exit, 1..6 = product), CF=0
read_product proc
rp_loop:
    mov dx, offset msg_select
    call print_str
    call read_int
    jc rp_badnum
    cmp ax, NUM_PRODUCTS
    ja rp_badcode
    clc                     ; valid: AX = 0..6, so AL = code
    ret
rp_badcode:
    mov dx, offset msg_invalid
    call print_str
    jmp rp_loop
rp_badnum:
    mov dx, offset msg_badnum
    call print_str
    jmp rp_loop
read_product endp

; lookup_price: AL = product code (1..6)
; Output: AX = price, CF=0 ; CF=1 if code not found
lookup_price proc
    push bx
    cmp al, 1
    jb lp_fail
    cmp al, NUM_PRODUCTS
    ja lp_fail
    xor ah, ah
    dec ax                  ; index 0..5
    shl ax, 1               ; *2 (word array)
    mov bx, ax
    mov ax, word ptr [product_prices + bx]
    clc
    jmp lp_exit
lp_fail:
    stc
lp_exit:
    pop bx
    ret
lookup_price endp

; lookup_name: AL = product code (1..6)
; Output: DX = offset of the '$'-terminated product name, CF=0
;         CF=1 if the code is not found (DX unchanged)
lookup_name proc
    push ax
    push bx
    cmp al, 1
    jb ln_fail
    cmp al, NUM_PRODUCTS
    ja ln_fail
    xor ah, ah
    dec ax                  ; index 0..5
    shl ax, 1               ; *2 (word array of offsets)
    mov bx, ax
    mov dx, word ptr [product_names + bx]
    clc
    jmp ln_exit
ln_fail:
    stc
ln_exit:
    pop bx
    pop ax
    ret
lookup_name endp

; read_payment: prompts for money to insert (used for the first payment
; and for every extra insertion after "insufficient funds").
; Re-prompts internally on non-numeric or over-999 input.
; Output: AX = amount (0..999), CF=0.   AX = 0 means the user CANCELLED.
read_payment proc
rpay_loop:
    mov dx, offset msg_pay
    call print_str
    call read_int
    jc rpay_bad
    clc
    ret
rpay_bad:
    mov dx, offset msg_badnum
    call print_str
    jmp rpay_loop
read_payment endp
