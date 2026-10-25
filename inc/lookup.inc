; ==========================================================
; lookup.inc - code_to_offset, lookup_price, lookup_name
; Requires: data.inc
; ==========================================================

.code

; code_to_offset: converts a product code into a table position.
; Input : AL = product code
; Output: CF=0 -> BX = byte offset of that product in the word tables
;                 (product_prices, product_names)
;         CF=1 -> code is not 1..NUM_PRODUCTS (BX undefined)
; Clobbers: BX
code_to_offset proc
    cmp al, 1
    jb cto_fail
    cmp al, NUM_PRODUCTS
    ja cto_fail
    mov bl, al
    xor bh, bh
    dec bx                  ; index 0..NUM_PRODUCTS-1
    shl bx, 1               ; *2 (word tables)
    clc                     ; shl changes CF, so clear it last
    ret
cto_fail:
    stc
    ret
code_to_offset endp

; lookup_price: AL = product code
; Output: CF=0 -> AX = price ; CF=1 -> not found (AX unchanged)
lookup_price proc
    push bx
    call code_to_offset
    jc lp_exit              ; pop does not change flags
    mov ax, word ptr [product_prices + bx]
lp_exit:
    pop bx
    ret
lookup_price endp

; lookup_name: AL = product code
; Output: CF=0 -> DX = offset of the '$'-terminated name ;
;         CF=1 -> not found (DX unchanged). AX is preserved.
lookup_name proc
    push bx
    call code_to_offset
    jc ln_exit
    mov dx, word ptr [product_names + bx]
ln_exit:
    pop bx
    ret
lookup_name endp
