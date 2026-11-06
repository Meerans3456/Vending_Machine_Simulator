; ==========================================================
; input.inc - read_int, ask_int, read_product
; Requires: data.inc, utils.inc, stock.inc (lookup_stock)
; ==========================================================

.code

; read_int: reads a decimal number from the keyboard until Enter.
; Input : DX = largest value accepted (0..65535)
; Output: AX = value, CF=0 ; CF=1 if invalid (AX undefined)
; Every printable key is kept in rbuf and shown; Backspace erases the last
; key typed (a letter too), so a typing mistake can be corrected before Enter.
; The line is checked when Enter is pressed.
; Invalid = empty line, a non-digit, or a value above DX (or above 65535).
read_int proc
    push bx
    push cx
    push dx
    push si
    push di
    mov si, dx              ; SI = largest value accepted
    xor cx, cx              ; CX = number of characters typed
ri_loop:
    mov ah, 08h             ; read a key without echo -> AL
    int 21h
    cmp al, CR              ; Enter?
    je ri_enter
    cmp al, BS              ; Backspace?
    je ri_back
    cmp al, 20h             ; other control keys (Tab, Esc...) are ignored
    jb ri_loop
    cmp cx, RBUF_SIZE       ; line full: ignore the key
    jae ri_loop
    mov bx, offset rbuf
    add bx, cx
    mov byte ptr [bx], al   ; keep the key in the line buffer
    inc cx
    mov dl, byte ptr echo_mask  ; a mask character hides the key (PIN entry)
    or dl, dl
    jnz ri_show
    mov dl, al              ; no mask: show the key itself
ri_show:
    mov ah, 02h
    int 21h
    jmp ri_loop
ri_back:                    ; erase the last key typed
    cmp cx, 0               ; nothing typed yet: ignore the key
    je ri_loop
    dec cx
    mov ah, 02h             ; move back, blank the character, move back again
    mov dl, BS
    int 21h
    mov ah, 02h
    mov dl, ' '
    int 21h
    mov ah, 02h
    mov dl, BS
    int 21h
    jmp ri_loop
ri_enter:
    mov ah, 02h             ; echo Enter as a carriage return
    mov dl, CR
    int 21h
    cmp cx, 0               ; empty line -> invalid
    je ri_fail
    mov di, offset rbuf     ; DI -> first typed character
    xor bx, bx              ; BX = accumulator
ri_parse:
    mov al, byte ptr [di]   ; next character
    inc di
    cmp al, '0'
    jb ri_fail
    cmp al, '9'
    ja ri_fail
    sub al, '0'
    xor ah, ah              ; AX = digit
    push ax                 ; save digit
    mov ax, bx
    mov dx, 10
    mul dx                  ; DX:AX = accumulator * 10
    mov bx, dx              ; BX = high word (not zero: more than 16 bits)
    pop dx                  ; DX = digit
    or bx, bx
    jnz ri_fail             ; too big for 16 bits
    add ax, dx
    jc ri_fail              ; too big for 16 bits
    mov bx, ax              ; new accumulator
    cmp bx, si
    ja ri_fail              ; above the largest accepted value
    loop ri_parse           ; next character (CX counts down to 0)
    mov ax, bx
    clc
    jmp ri_exit
ri_fail:
    stc
ri_exit:
    pop di
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
; Output: AX = admin_code (enter admin mode), 0 (exit), or the code of a
;         product that is in stock. BX and DX are preserved.
read_product proc
    push bx
    push dx
rp_loop:
    mov dx, offset msg_select
    mov bx, 0FFFFh          ; any 16-bit number: admin_code is larger than 999
    call ask_int
    cmp ax, word ptr admin_code
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
