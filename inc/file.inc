; ==========================================================
; file.inc - load_state, save_state
; Requires: data.inc
; The save file holds product_stock and coin_stock (sales totals are not saved).
; ==========================================================

.code

; save_state: writes the save block to the save file (created or emptied first).
; Errors such as a full or read-only disk are ignored: the machine keeps running.
; Saves and restores every register it uses.
save_state proc
    push ax
    push bx
    push cx
    push dx
    mov word ptr save_check, SAVE_BYTES
    mov ah, 3Ch             ; create or empty the file
    xor cx, cx              ; normal file attributes
    mov dx, offset state_file
    int 21h                 ; AX = file handle
    jc sv_exit
    mov bx, ax
    mov ah, 40h             ; write
    mov cx, SAVE_BYTES
    mov dx, offset save_block
    int 21h
    mov ah, 3Eh             ; close (BX = handle)
    int 21h
sv_exit:
    pop dx
    pop cx
    pop bx
    pop ax
    ret
save_state endp

; load_state: reads the save file into the stock tables.
; Nothing changes (the machine stays as it is) if the file is missing, cannot be
; read, has the wrong size, or was written for tables of a different length.
; Saves and restores every register it uses.
load_state proc
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push es
    mov ax, 3D00h           ; open for reading
    mov dx, offset state_file
    int 21h                 ; AX = file handle
    jc ld_exit              ; no save file yet
    mov bx, ax
    mov ah, 3Fh             ; read into the buffer, not into the live tables
    mov cx, SAVE_BYTES
    mov dx, offset load_buffer
    int 21h                 ; AX = bytes read
    mov si, ax              ; (mov keeps CF from the read)
    jnc ld_close
    xor si, si              ; the read failed: count it as nothing read
ld_close:
    mov ah, 3Eh             ; close (BX = handle)
    int 21h
    cmp si, SAVE_BYTES
    jne ld_exit             ; too short
    cmp word ptr load_buffer, SAVE_BYTES
    jne ld_exit             ; written for tables of a different length
    push ds
    pop es                  ; ES = DS, because MOVSB writes through ES:DI
    mov si, offset load_buffer
    mov di, offset save_block
    mov cx, SAVE_BYTES
    cld
    rep movsb               ; the file looks right: use it
ld_exit:
    pop es
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
load_state endp
