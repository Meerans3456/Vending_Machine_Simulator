; ============================================================
; t_output.asm - standalone test: output.inc (print_change)
; Build  : build t_output
; Loads hardcoded counts into 'change' and denom_count, then
; calls print_change. No other module logic is involved.
; ============================================================
.model small
.stack 100h

include data.inc

; --- test-only data ---
.data
t_hdr           db '--- print_change with hardcoded counts ---$'
t_hdr2          db '--- print_receipt ---$'
t_hdr3          db '--- print_summary ---$'
; each case = 6 words: change, count50, count25, count10, count5, count1
t_cases         dw 35,  0, 1, 1, 0, 0
                dw 0,   0, 0, 0, 0, 0
                dw 99,  1, 1, 2, 0, 4
                dw 964, 19, 0, 1, 0, 4
                dw 1,   0, 0, 0, 0, 1
t_casesize      equ 6 * 2               ; bytes per case (6 words)
t_ncases        equ ($ - t_cases) / t_casesize

include utils.inc
include lookup.inc
include output.inc

.code

main proc
    mov ax, @data
    mov ds, ax

    mov dx, offset t_hdr
    call print_str
    call newline

    mov si, offset t_cases
    mov cx, t_ncases
tp_loop:
    mov ax, word ptr [si]
    mov word ptr change, ax         ; set the total
    push cx                         ; save outer loop counter
    push si                         ; save case pointer
    add si, 2                       ; SI -> the five counts
    mov di, offset denom_count
    mov cx, NUM_DENOMS
tp_copy:
    mov ax, word ptr [si]
    mov word ptr [di], ax           ; copy count into denom_count
    add si, 2
    add di, 2
    loop tp_copy
    pop si
    pop cx
    call print_change
    add si, t_casesize              ; next case
    loop tp_loop

    ; --- print_receipt tests (variables set by hand) ---
    call newline
    mov dx, offset t_hdr2
    call print_str
    call newline

    ; Case 1: Chips, price 15, paid 50, change 35 = 1x25 + 1x10
    mov sel_code, 1
    mov price, 15
    mov payment, 50
    mov change, 35
    mov word ptr [denom_count + 0], 0
    mov word ptr [denom_count + 2], 1
    mov word ptr [denom_count + 4], 1
    mov word ptr [denom_count + 6], 0
    mov word ptr [denom_count + 8], 0
    call print_receipt

    ; Case 2: Soda, price 25, paid 25, change 0 (exact payment)
    mov sel_code, 2
    mov price, 25
    mov payment, 25
    mov change, 0
    mov word ptr [denom_count + 0], 0
    mov word ptr [denom_count + 2], 0
    mov word ptr [denom_count + 4], 0
    mov word ptr [denom_count + 6], 0
    mov word ptr [denom_count + 8], 0
    call print_receipt

    ; Case 3: Juice, price 35, paid 100, change 65 = 1x50 + 1x10 + 1x5
    mov sel_code, 6
    mov price, 35
    mov payment, 100
    mov change, 65
    mov word ptr [denom_count + 0], 1
    mov word ptr [denom_count + 2], 0
    mov word ptr [denom_count + 4], 1
    mov word ptr [denom_count + 6], 1
    mov word ptr [denom_count + 8], 0
    call print_receipt

    ; --- print_summary tests (variables set by hand) ---
    call newline
    mov dx, offset t_hdr3
    call print_str
    call newline

    mov sales_count, 3
    mov sales_total, 70
    call print_summary      ; expect 3 and 70

    mov sales_count, 0
    mov sales_total, 0
    call print_summary      ; expect 0 and 0

    mov ah, 4Ch
    int 21h
main endp

end main
