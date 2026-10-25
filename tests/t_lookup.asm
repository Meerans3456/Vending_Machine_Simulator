; ============================================================
; t_lookup.asm - standalone test: lookup.inc
; Build  : build t_lookup
; Runs every routine over the same list of codes (t_codes).
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_codes         db 0, 1, 3, 6, 7, 9
t_ncodes        equ $ - t_codes          ; number of codes
t_hdr_price     db '--- lookup_price ---$'
t_hdr_name      db '--- lookup_name ---$'
t_hdr_offset    db '--- code_to_offset ---$'
t_nf            db 'not found$'
t_routine       dw 0            ; routine under test (called by t_run)
t_printer       dw 0            ; routine that prints its result

include utils.inc
include lookup.inc

.code

; t_show_bx: prints BX as a number (code_to_offset returns its result in BX)
t_show_bx proc
    mov ax, bx
    call print_int
    ret
t_show_bx endp

; t_run: DX = header string. For each code in t_codes it prints "code: ",
; calls t_routine, then prints the result with t_printer or "not found".
t_run proc
    call newline
    call print_str          ; header
    call newline
    mov si, offset t_codes
    mov cx, t_ncodes
tr_loop:
    mov al, byte ptr [si]
    xor ah, ah
    call print_int          ; the code
    mov dl, ':'
    call print_char
    mov dl, ' '
    call print_char
    mov al, byte ptr [si]   ; reload (print_char clobbers AX)
    call word ptr t_routine
    jc tr_fail
    call word ptr t_printer ; result is in AX, BX or DX, depending on the routine
    jmp tr_next
tr_fail:
    mov dx, offset t_nf
    call print_str
tr_next:
    call newline
    inc si
    loop tr_loop
    ret
t_run endp

main proc
    mov ax, @data
    mov ds, ax

    mov word ptr t_routine, offset lookup_price
    mov word ptr t_printer, offset print_int        ; price is in AX
    mov dx, offset t_hdr_price
    call t_run

    mov word ptr t_routine, offset lookup_name
    mov word ptr t_printer, offset print_str        ; name offset is in DX
    mov dx, offset t_hdr_name
    call t_run

    mov word ptr t_routine, offset code_to_offset
    mov word ptr t_printer, offset t_show_bx        ; offset is in BX
    mov dx, offset t_hdr_offset
    call t_run

    mov ah, 4Ch
    int 21h
main endp

end main
