; ============================================================
; t_utils.asm - standalone test: utils.inc (print_char, print_int, print_spaces, print_name_col, print_int3)
; Build  : build t_utils
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hello         db 'Utilities test running.$'
t_char          db 'print_char A  = $'
t_int0          db 'print_int 0     = $'
t_int7          db 'print_int 7     = $'
t_int10         db 'print_int 10    = $'
t_int12345      db 'print_int 12345 = $'
t_int65535      db 'print_int 65535 = $'
t_sp            db 'print_spaces 3 = [$'
t_nc            db 'print_name_col = [$'
t_nm            db 'Soda$'
t_i3            db 'print_int3 5, 15, 150 = [$'
t_scr           db 'Next: press any key; the screen will be cleared.$'
t_cleared       db 'Screen cleared OK (text now starts at top-left).$'

include utils.inc

.code

main proc
    mov ax, @data
    mov ds, ax

    mov dx, offset t_hello
    call print_str
    call newline

    mov dx, offset t_char
    call print_str
    mov dl, 'A'
    call print_char
    call newline

    mov dx, offset t_int0
    call print_str
    mov ax, 0
    call print_int
    call newline

    mov dx, offset t_int7
    call print_str
    mov ax, 7
    call print_int
    call newline

    mov dx, offset t_int10
    call print_str
    mov ax, 10
    call print_int
    call newline

    mov dx, offset t_int12345
    call print_str
    mov ax, 12345
    call print_int
    call newline

    mov dx, offset t_int65535
    call print_str
    mov ax, 65535
    call print_int
    call newline

    ; --- clear_screen / wait_key ---
    call newline
    mov dx, offset t_sp
    call print_str
    mov ax, 3
    call print_spaces       ; three spaces should appear between the brackets
    mov dl, ']'
    call print_char
    call newline
    mov ax, 0
    call print_spaces       ; zero spaces: nothing is printed
    mov dx, offset t_nc
    call print_str
    mov dx, offset t_nm
    call print_name_col     ; "Soda" followed by spaces up to NAME_WIDTH
    mov dl, ']'
    call print_char
    call newline
    mov dx, offset t_i3
    call print_str
    mov ax, 5
    call print_int3         ; "  5"
    mov dl, '|'
    call print_char
    mov ax, 15
    call print_int3         ; " 15"
    mov dl, '|'
    call print_char
    mov ax, 150
    call print_int3         ; "150"
    mov dl, ']'
    call print_char
    call newline
    mov dx, offset t_scr
    call print_str
    call wait_key
    call clear_screen
    mov dx, offset t_cleared
    call print_str
    call newline

    mov ah, 4Ch
    int 21h
main endp

end main
