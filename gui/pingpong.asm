; ==========================================================
; pingpong.asm - handshake test for the GUI bridge (not part of the vending program)
; Creates CMD.TXT and OUT.TXT in the current folder, then keeps checking CMD.TXT.
; When Python writes a new record "SEQ|TEXT#" into CMD.TXT, this program writes
; "SEQ|OK TEXT#" into OUT.TXT. A record of "QUIT" ends the program.
; Every record is exactly REC_SIZE bytes, padded with spaces, so a file is
; always overwritten in place and never needs to be shortened.
; Build (from the project folder):
;   & "D:\8086VendingMachine\tools\JWasm\JWasm.exe" -0 -mz -Fo pingpong.exe gui\pingpong.asm
; ==========================================================
.model small
.stack 100h

REC_SIZE        equ 64          ; size of every record, in bytes
CR              equ 0Dh
LF              equ 0Ah

.data
f_cmd           db 'CMD.TXT', 0     ; commands from Python
f_out           db 'OUT.TXT', 0     ; replies to Python
inbuf           db REC_SIZE dup(' ')    ; the record read from CMD.TXT
outbuf          db REC_SIZE dup(' ')    ; the record written to OUT.TXT
init_rec        db '0|READY#'           ; first content of both files (sequence 0)
INIT_LEN        equ $ - init_rec
ok_str          db 'OK '
quit_str        db 'QUIT#'
last_seq        dw 0            ; sequence number of the last command answered
cur_seq         dw 0            ; sequence number being answered now
text_ptr        dw 0            ; address of the text after '|' in inbuf
msg_start       db 'pingpong: CMD.TXT and OUT.TXT are ready. Run gui\pingpong.py in another window.', CR, LF, '$'
msg_got         db 'got: $'
msg_crlf        db CR, LF, '$'
msg_bye         db 'QUIT received. Bye.', CR, LF, '$'
msg_err         db 'File error: could not create CMD.TXT or OUT.TXT.', CR, LF, '$'

.code

; clear_buf: DI = buffer; fills REC_SIZE bytes with spaces.
clear_buf proc
    push ax
    push cx
    mov al, ' '
    mov cx, REC_SIZE
    cld
    rep stosb
    pop cx
    pop ax
    ret
clear_buf endp

; create_init: DX = file name; creates the file holding outbuf (the first record).
; CF=1 on error.
create_init proc
    mov ah, 3Ch             ; DOS: create (or empty) the file
    xor cx, cx              ; normal attributes
    int 21h
    jc ci_end
    mov bx, ax              ; BX = file handle
    mov dx, offset outbuf
    mov cx, REC_SIZE
    mov ah, 40h             ; DOS: write the record
    int 21h
    pushf                   ; keep the result of the write
    mov ah, 3Eh             ; DOS: close the file
    int 21h
    popf
ci_end:
    ret
create_init endp

; make_files: creates CMD.TXT and OUT.TXT, both holding "0|READY#". CF=1 on error.
make_files proc
    mov di, offset outbuf
    call clear_buf
    mov si, offset init_rec
    mov di, offset outbuf
    mov cx, INIT_LEN
    cld
    rep movsb               ; outbuf = "0|READY#" and spaces
    mov dx, offset f_cmd
    call create_init
    jc mf_end
    mov dx, offset f_out
    call create_init
mf_end:
    ret
make_files endp

; read_cmd: reads REC_SIZE bytes of CMD.TXT into inbuf. CF=1 if it could not be read.
read_cmd proc
    mov dx, offset f_cmd
    mov ax, 3D00h           ; DOS: open for reading
    int 21h
    jc rc_fail
    mov bx, ax
    mov dx, offset inbuf
    mov cx, REC_SIZE
    mov ah, 3Fh             ; DOS: read the record
    int 21h
    pushf
    push ax                 ; keep the result of the read
    mov ah, 3Eh             ; DOS: close the file
    int 21h
    pop ax
    popf
    jc rc_fail
    cmp ax, REC_SIZE        ; a short read: the record is not complete
    jne rc_fail
    clc
    ret
rc_fail:
    stc
    ret
read_cmd endp

; parse_seq: reads the digits at the start of inbuf up to '|'.
; Output: AX = sequence number, CF=0 and text_ptr = address after '|'; CF=1 if bad.
parse_seq proc
    push bx
    push cx
    push dx
    push si
    mov si, offset inbuf
    xor bx, bx              ; BX = number so far
ps_loop:
    lodsb
    cmp al, '|'
    je ps_done
    sub al, '0'
    cmp al, 9
    ja ps_bad               ; not a digit
    xor ah, ah
    mov cx, ax              ; CX = digit
    mov ax, bx
    mov dx, 10
    mul dx
    add ax, cx
    mov bx, ax
    jmp ps_loop
ps_done:
    mov word ptr text_ptr, si
    mov ax, bx
    clc
    jmp ps_exit
ps_bad:
    stc
ps_exit:
    pop si
    pop dx
    pop cx
    pop bx
    ret
parse_seq endp

; show_got: prints the record in inbuf up to the '#' on the screen.
show_got proc
    push ax
    push dx
    push si
    mov dx, offset msg_got
    mov ah, 09h
    int 21h
    mov si, offset inbuf
sg_loop:
    lodsb
    cmp al, '#'
    je sg_end
    mov dl, al
    mov ah, 02h
    int 21h
    jmp sg_loop
sg_end:
    mov dx, offset msg_crlf
    mov ah, 09h
    int 21h
    pop si
    pop dx
    pop ax
    ret
show_got endp

; build_reply: outbuf = "SEQ|OK TEXT#" followed by spaces, from the record in inbuf.
build_reply proc
    push ax
    push cx
    push si
    push di
    mov di, offset outbuf
    call clear_buf
    mov si, offset inbuf
    mov di, offset outbuf
    mov cx, word ptr text_ptr
    sub cx, offset inbuf    ; CX = length of "SEQ|"
    cld
    rep movsb
    mov si, offset ok_str
    mov cx, 3
    rep movsb               ; "OK "
    mov si, word ptr text_ptr
br_loop:
    lodsb                   ; copy the text up to and including '#'
    stosb
    cmp al, '#'
    jne br_loop
    pop di
    pop si
    pop cx
    pop ax
    ret
build_reply endp

; write_reply: writes outbuf over OUT.TXT, in place. CF=1 on error.
write_reply proc
    mov dx, offset f_out
    mov ax, 3D01h           ; DOS: open for writing (the file is not emptied)
    int 21h
    jc wr_end
    mov bx, ax
    mov dx, offset outbuf
    mov cx, REC_SIZE
    mov ah, 40h             ; DOS: write the record
    int 21h
    pushf
    mov ah, 3Eh             ; DOS: close the file
    int 21h
    popf
wr_end:
    ret
write_reply endp

; handle_cmd: answers a new complete command, if there is one.
; Output: CF=1 if the command was QUIT.
handle_cmd proc
    call read_cmd
    jc hc_none              ; could not read it: try again later
    mov di, offset inbuf
    mov cx, REC_SIZE
    mov al, '#'
    cld
    repne scasb
    jne hc_none             ; no end marker yet: the record is not complete
    call parse_seq
    jc hc_none
    cmp ax, word ptr last_seq
    je hc_none              ; this command was already answered
    mov word ptr cur_seq, ax
    call show_got
    call build_reply
    call write_reply
    jc hc_none              ; not written: answer it again next time
    mov ax, word ptr cur_seq
    mov word ptr last_seq, ax
    mov si, word ptr text_ptr
    mov di, offset quit_str
    mov cx, 5
    cld
    repe cmpsb
    je hc_quit              ; the text was QUIT#
hc_none:
    clc
    ret
hc_quit:
    stc
    ret
handle_cmd endp

; wait_tick: waits for the next BIOS clock tick (about 55 ms).
wait_tick proc
    push ax
    push bx
    push cx
    push dx
    mov ah, 0
    int 1Ah                 ; CX:DX = clock ticks
    mov bx, dx
wt_loop:
    mov ah, 0
    int 1Ah
    cmp dx, bx
    je wt_loop              ; same tick: keep waiting
    pop dx
    pop cx
    pop bx
    pop ax
    ret
wait_tick endp

main proc
    mov ax, @data
    mov ds, ax
    push ds
    pop es                  ; ES = DS for the string instructions
    call make_files
    jc m_error
    mov dx, offset msg_start
    mov ah, 09h
    int 21h
m_loop:
    call handle_cmd
    jc m_done               ; QUIT received
    call wait_tick
    jmp m_loop
m_done:
    mov dx, offset msg_bye
    mov ah, 09h
    int 21h
    mov ax, 4C00h           ; DOS: end the program
    int 21h
m_error:
    mov dx, offset msg_err
    mov ah, 09h
    int 21h
    mov ax, 4C01h
    int 21h
main endp

end main
