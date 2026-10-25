; ==========================================================
; utils.inc - print_str, newline, print_char, print_int, clear_screen, wait_key
; Requires: data.inc
; ==========================================================

.code

; print_str: DX = offset of '$'-terminated string
; Clobbers: AX
print_str proc
    mov ah, 09h
    int 21h
    ret
print_str endp

; newline: prints CR LF. Clobbers: nothing (saves AX, DX)
newline proc
    push ax
    push dx
    mov ah, 02h
    mov dl, 0Dh
    int 21h
    mov dl, 0Ah
    int 21h
    pop dx
    pop ax
    ret
newline endp

; print_char: DL = character to print. Clobbers: AX
print_char proc
    mov ah, 02h
    int 21h
    ret
print_char endp

; print_int: AX = unsigned integer (0..65535) to print in decimal.
; Saves and restores all registers it uses.
print_int proc
    push ax
    push bx
    push cx
    push dx

    cmp ax, 0
    jne pi_nonzero
    mov dl, '0'             ; special case: zero
    mov ah, 02h
    int 21h
    jmp pi_done

pi_nonzero:
    mov bx, 10
    xor cx, cx              ; CX = digit count
pi_divloop:
    xor dx, dx              ; clear DX before DIV (avoid overflow crash)
    div bx                  ; AX = quotient, DX = remainder (digit)
    push dx                 ; save digit on stack
    inc cx
    cmp ax, 0
    jne pi_divloop

pi_printloop:
    pop dx                  ; digits come back most-significant first
    add dl, '0'
    mov ah, 02h
    int 21h
    loop pi_printloop

pi_done:
    pop dx
    pop cx
    pop bx
    pop ax
    ret
print_int endp

; clear_screen: blanks the whole 80x25 screen and homes the cursor.
; Uses INT 10h fn 06h (scroll window up with AL=0 = clear), then fn 02h.
; Saves and restores every register it uses.
clear_screen proc
    push ax
    push bx
    push cx
    push dx
    mov ax, 0600h           ; AH=06h scroll up, AL=0 -> clear whole window
    mov bh, 07h             ; attribute for blank lines: light grey on black
    xor cx, cx              ; top-left corner: row 0, col 0
    mov dx, 184Fh           ; bottom-right corner: row 24, col 79
    int 10h
    mov ah, 02h             ; set cursor position
    xor bh, bh              ; page 0
    xor dx, dx              ; row 0, col 0
    int 10h
    pop dx
    pop cx
    pop bx
    pop ax
    ret
clear_screen endp

; wait_key: prints the "Press any key" prompt and waits for a keypress
; (no echo). Saves and restores AX and DX.
wait_key proc
    push ax
    push dx
    mov dx, offset msg_press
    call print_str
    mov ah, 08h             ; read key, no echo
    int 21h
    pop dx
    pop ax
    ret
wait_key endp
