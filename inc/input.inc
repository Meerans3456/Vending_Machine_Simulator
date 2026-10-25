; ==========================================================
; input.inc - read_int, read_product, read_payment
; Requires: data.inc, utils.inc
; ==========================================================

.code

; read_int: reads a decimal number from the keyboard until Enter.
; Output: AX = value (0..MAX_INPUT), CF=0 ; CF=1 if invalid (AX undefined)
; Invalid = empty input, non-digit character, or value > MAX_INPUT.
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
    cmp al, CR             ; Enter?
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
    mul dx                  ; DX:AX = BX*10 (DX = 0: MAX_INPUT*10 fits in 16 bits)
    pop dx                  ; DX = digit
    add ax, dx
    mov bx, ax
    inc cx
    cmp bx, MAX_INPUT
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
    cmp al, CR
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

; read_payment: prompts for money to insert (used for the first payment
; and for every extra insertion after "insufficient funds").
; Re-prompts internally on non-numeric or over-MAX_INPUT input.
; Output: AX = amount (0..MAX_INPUT), CF=0.   AX = 0 means the user CANCELLED.
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
