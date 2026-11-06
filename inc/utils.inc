; ==========================================================
; utils.inc - print_str, newline, print_char, print_int, print_spaces, print_name_col, print_int3, clear_screen, wait_key
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
    mov dl, CR
    int 21h
    mov dl, LF
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

; print_spaces: AX = number of spaces to print (0 prints nothing).
; Saves and restores every register it uses.
print_spaces proc
    push ax
    push cx
    push dx
    mov cx, ax              ; CX = number of spaces
    jcxz ps_done            ; nothing to print
ps_loop:
    mov ah, 02h
    mov dl, ' '
    int 21h
    loop ps_loop
ps_done:
    pop dx
    pop cx
    pop ax
    ret
print_spaces endp

; print_name_col: DX = offset of a '$'-terminated text; prints it and pads
; it with spaces up to NAME_WIDTH characters, so the next column lines up.
; Saves and restores every register it uses.
print_name_col proc
    push ax
    push dx
    push di
    call print_str          ; print the text
    mov di, dx              ; DI walks the text to measure it
    xor dx, dx              ; DX = length of the text
pn_len:
    cmp byte ptr [di], '$'  ; end of the text?
    je pn_lendone
    inc di
    inc dx
    jmp pn_len
pn_lendone:
    mov ax, NAME_WIDTH
    sub ax, dx              ; AX = spaces needed to fill the column
    jbe pn_done             ; the text already fills the column
    call print_spaces
pn_done:
    pop di
    pop dx
    pop ax
    ret
print_name_col endp

; print_int3: AX = unsigned integer (0..999) printed right-aligned in 3 columns
; (1 digit -> 2 spaces first, 2 digits -> 1 space, 3 digits -> none).
; Saves and restores every register it uses.
print_int3 proc
    push ax
    push dx
    mov dx, 2               ; 1 digit  -> 2 spaces
    cmp ax, 10
    jb p3_pad
    mov dx, 1               ; 2 digits -> 1 space
    cmp ax, 100
    jb p3_pad
    xor dx, dx              ; 3 digits -> no space
p3_pad:
    push ax                 ; keep the number
    mov ax, dx
    call print_spaces       ; the padding
    pop ax
    call print_int          ; the number
    pop dx
    pop ax
    ret
print_int3 endp

; clear_screen: blanks the whole 80x25 screen and homes the cursor.
; Uses INT 10h fn 06h (scroll window up with AL=0 = clear), then fn 02h.
; Saves and restores every register it uses.
clear_screen proc
    push ax
    push bx
    push cx
    push dx
    mov ax, 0600h           ; AH=06h scroll up, AL=0 -> clear whole window
    mov bh, TEXT_ATTR             ; attribute for blank lines: light grey on black
    xor cx, cx              ; top-left corner: row 0, col 0
    mov dx, SCREEN_END           ; bottom-right corner: row 24, col 79
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
