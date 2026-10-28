; ==========================================================
; input.inc - read_int, ask_int, read_product
; Requires: data.inc, utils.inc, stock.inc (lookup_stock)
; ==========================================================

.code

; read_int: reads a decimal number from the keyboard until Enter.
; Input : DX = largest value accepted (0..65535)
; Output: AX = value, CF=0 ; CF=1 if invalid (AX undefined)
; Invalid = empty input, a non-digit, or a value above DX (or above 65535).
; On invalid input the rest of the line is discarded.
read_int proc
    push bx
    push cx
    push dx
    push si
    mov si, dx              ; SI = largest value accepted
    xor bx, bx              ; BX = accumulator
    xor cx, cx              ; CX = number of digits typed
ri_loop:
    mov ah, 01h             ; read char with echo -> AL
    int 21h
    cmp al, CR              ; Enter?
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
    mul dx                  ; DX:AX = accumulator * 10
    mov bx, dx              ; BX = high word (not zero: more than 16 bits)
    pop dx                  ; DX = digit
    or bx, bx
    jnz ri_bad              ; too big for 16 bits
    add ax, dx
    jc ri_bad               ; too big for 16 bits
    mov bx, ax              ; new accumulator
    inc cx
    cmp bx, si
    ja ri_bad               ; above the largest accepted value
    jmp ri_loop
ri_enter:
    cmp cx, 0               ; no digits typed -> invalid
    je ri_fail
    mov ax, bx
    clc
    jmp ri_exit
ri_bad:                     ; discard the rest of the line
    mov ah, 01h
    int 21h
    cmp al, CR
    jne ri_bad
ri_fail:
    stc
ri_exit:
    pop si
    pop dx
    pop cx
    pop bx
    ret
read_int endp

; ask_int: prints a prompt and reads a number, asking again until it is valid.
; Input : DX = prompt string, BX = largest value accepted
; Output: AX = value (BX and DX are preserved)
ask_int proc
    push si
    push dx
    mov si, dx              ; SI = prompt
ai_loop:
    mov dx, si
    call print_str          ; prompt
    mov dx, bx
    call read_int
    jnc ai_done
    mov dx, offset msg_badnum
    call print_str
    jmp ai_loop
ai_done:
    pop dx
    pop si
    ret
ask_int endp

; read_product: prompts for a product code and validates it.
; Asks again on an invalid number, an unknown code or a sold-out product.
; Output: AX = ADMIN_CODE (enter admin mode), 0 (exit), or the code of a
;         product that is in stock. BX and DX are preserved.
read_product proc
    push bx
    push dx
rp_loop:
    mov dx, offset msg_select
    mov bx, 0FFFFh          ; any 16-bit number: ADMIN_CODE is larger than 999
    call ask_int
    cmp ax, ADMIN_CODE
    je rp_exit
    cmp ax, NUM_PRODUCTS
    ja rp_badcode
    cmp ax, 0
    je rp_exit              ; 0 = exit
    push ax                 ; keep the code while the stock is checked
    call lookup_stock       ; AX = units left (the code is in AL)
    cmp ax, 0
    pop ax                  ; pop does not change the flags
    jne rp_exit             ; in stock
    mov dx, offset msg_soldout
    jmp rp_say
rp_badcode:
    mov dx, offset msg_invalid
rp_say:
    call print_str
    jmp rp_loop
rp_exit:
    pop dx
    pop bx
    ret
read_product endp
