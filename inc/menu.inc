; ==========================================================
; menu.inc - display_menu
; Requires: data.inc, utils.inc
; ==========================================================

.code

; display_menu: prints the product menu. Clobbers: AX, DX
display_menu proc
    mov dx, offset msg_menu
    call print_str
    ret
display_menu endp
