; ==========================================================
; bridge.inc - bridge mode: lets a program such as the Python GUI drive the
;              vending machine through two files in the project folder.
; Requires: data.inc, utils.inc, lookup.inc, stock.inc, math.inc, admin.inc, file.inc
; Started with:  vending.exe /g
;
; CMD.TXT (GUI -> program): one record of BR_CMD_SIZE bytes, "SEQ|COMMAND args#",
;                           padded with spaces. SEQ is a number that grows with every command.
; OUT.TXT (program -> GUI): one record of BR_OUT_SIZE bytes, "SEQ|status", a line
;                           per data item, then '#', padded with spaces.
; Both files are created when bridge mode starts and are only ever overwritten
; in place, so DOSBox always sees them. The program text output is redirected
; into OUT.TXT while a command runs, so print_str, print_int, ... write the reply.
; Before a reply is written the old one is wiped with spaces, and the '#' is written
; last, so a reply that is still being written is never mistaken for a finished one.
; The commands and replies are listed in README.md.
; ==========================================================

.data
BR_CMD_SIZE     equ 64          ; size of the command record
BR_OUT_SIZE     equ 1024        ; size of the reply record
bridge_flag     db 0            ; 1 when the program was started with /g
br_cmd_name     db 'CMD.TXT', 0
br_out_name     db 'OUT.TXT', 0
br_inbuf        db BR_CMD_SIZE dup(' ')     ; the command record read from CMD.TXT
br_space        db BR_OUT_SIZE dup(' ')     ; spaces, used to pad files and replies
br_init         db '0|READY#'               ; first content of both files (sequence 0)
BR_INIT_LEN     equ $ - br_init
br_last_seq     dw 0            ; sequence number of the last command answered
br_cur_seq      dw 0            ; sequence number being answered now
br_text         dw 0            ; address of the command text (after the '|')
br_args         dw 0            ; address of the arguments (after the command word)
br_out_handle   dw 0            ; DOS handle of OUT.TXT while a reply is written
br_old_stdout   dw 0            ; copy of the console handle while output is redirected
br_quit         db 0            ; 1 after QUIT
br_tx           db 0            ; 1 while a purchase is open (BUY done, not finished)
br_admin        db 0            ; 1 while an admin session is open (LOGIN done)

; Command words ('$' ends each word) and the routine that handles each.
brv_state       db 'STATE$'
brv_buy         db 'BUY$'
brv_pay         db 'PAY$'
brv_cancel      db 'CANCEL$'
brv_login       db 'LOGIN$'
brv_logout      db 'LOGOUT$'
brv_refill      db 'REFILL$'
brv_setstock    db 'SETSTOCK$'
brv_setcoin     db 'SETCOIN$'
brv_setcode     db 'SETCODE$'
brv_setpin      db 'SETPIN$'
brv_quit        db 'QUIT$'
br_verbs        dw brv_state, h_state
                dw brv_buy, h_buy
                dw brv_pay, h_pay
                dw brv_cancel, h_cancel
                dw brv_login, h_login
                dw brv_logout, h_logout
                dw brv_refill, h_refill
                dw brv_setstock, h_setstock
                dw brv_setcoin, h_setcoin
                dw brv_setcode, h_setcode
                dw brv_setpin, h_setpin
                dw brv_quit, h_quit
                dw 0, 0         ; end of the table

; Reply words ('$' ends each one).
br_s_ok         db 'OK$'
br_s_oktx       db 'OK TX$'
br_s_short      db 'OK SHORT$'
br_s_nochange   db 'OK NOCHANGE$'
br_s_sold       db 'OK SOLD$'
br_s_refund     db 'OK REFUND$'
br_s_wrong      db 'ERR WRONG$'
br_s_locked     db 'ERR LOCKED$'
br_e_invalid    db 'ERR invalid$'
br_e_soldout    db 'ERR soldout$'
br_e_busy       db 'ERR busy$'
br_e_notx       db 'ERR notx$'
br_e_amount     db 'ERR amount$'
br_e_login      db 'ERR login$'
br_e_range      db 'ERR range$'
br_e_denom      db 'ERR denom$'
br_e_unknown    db 'ERR unknown$'
br_msg_start    db 'Bridge mode: waiting for commands in CMD.TXT (the reply goes to OUT.TXT).', CR, LF, '$'
br_msg_error    db 'Bridge mode: could not create CMD.TXT or OUT.TXT.', CR, LF, '$'

.code

; ---------- start-up ----------

; detect_bridge: sets bridge_flag when the command line holds "/g".
; ES must still hold the program prefix segment (the value at program start).
detect_bridge proc
    push ax
    push cx
    push si
    xor ch, ch
    mov cl, byte ptr es:[80h]   ; length of the command line
    mov si, 81h                 ; the command line itself
    jcxz db_done
db_loop:
    cmp byte ptr es:[si], '/'
    jne db_next
    mov al, byte ptr es:[si + 1]
    or al, 20h                  ; to lower case
    cmp al, 'g'
    jne db_next
    mov byte ptr bridge_flag, 1
    jmp db_done
db_next:
    inc si
    loop db_loop
db_done:
    pop si
    pop cx
    pop ax
    ret
detect_bridge endp

; br_create: DX = file name, SI = file size. Creates the file holding "0|READY#"
; padded with spaces to SI bytes. CF=1 on error.
br_create proc
    mov ah, 3Ch                 ; DOS: create (or empty) the file
    xor cx, cx
    int 21h
    jc bc_end
    mov bx, ax                  ; BX = file handle
    mov dx, offset br_init
    mov cx, BR_INIT_LEN
    mov ah, 40h                 ; DOS: write the first record
    int 21h
    jc bc_close
    mov cx, si
    sub cx, BR_INIT_LEN         ; CX = number of padding spaces
    mov dx, offset br_space
    mov ah, 40h
    int 21h
bc_close:
    pushf                       ; keep the result of the writes
    mov ah, 3Eh                 ; DOS: close the file
    int 21h
    popf
bc_end:
    ret
br_create endp

; br_make_files: creates CMD.TXT and OUT.TXT. CF=1 on error.
br_make_files proc
    push si
    mov dx, offset br_cmd_name
    mov si, BR_CMD_SIZE
    call br_create
    jc bm_end
    mov dx, offset br_out_name
    mov si, BR_OUT_SIZE
    call br_create
bm_end:
    pop si
    ret
br_make_files endp

; ---------- reading a command ----------

; br_read_cmd: reads the command record of CMD.TXT into br_inbuf. CF=1 if it
; could not be read completely.
br_read_cmd proc
    mov dx, offset br_cmd_name
    mov ax, 3D00h               ; DOS: open for reading
    int 21h
    jc rc_fail
    mov bx, ax
    mov dx, offset br_inbuf
    mov cx, BR_CMD_SIZE
    mov ah, 3Fh                 ; DOS: read the record
    int 21h
    pushf
    push ax                     ; keep the result of the read
    mov ah, 3Eh                 ; DOS: close the file
    int 21h
    pop ax
    popf
    jc rc_fail
    cmp ax, BR_CMD_SIZE         ; a short read: the record is not complete
    jne rc_fail
    clc
    ret
rc_fail:
    stc
    ret
br_read_cmd endp

; br_parse_seq: reads the digits at the start of br_inbuf up to '|'.
; Output: CF=0, AX = sequence number and br_text = address after '|'; CF=1 if bad.
br_parse_seq proc
    push bx
    push cx
    push dx
    push si
    mov si, offset br_inbuf
    xor bx, bx                  ; BX = number so far
bps_loop:
    lodsb
    cmp al, '|'
    je bps_done
    sub al, '0'
    cmp al, 9
    ja bps_bad                   ; not a digit
    xor ah, ah
    mov cx, ax                  ; CX = digit
    mov ax, bx
    mov dx, 10
    mul dx
    add ax, cx
    mov bx, ax
    jmp bps_loop
bps_done:
    mov word ptr br_text, si
    mov ax, bx
    clc
    jmp bps_exit
bps_bad:
    stc
bps_exit:
    pop si
    pop dx
    pop cx
    pop bx
    ret
br_parse_seq endp

; br_num: reads a whole number at SI (leading spaces are skipped); it must be
; followed by a space or '#'. SI moves past it.
; Output: CF=0, AX = number; CF=1 if there is no valid number (above 65535 too).
br_num proc
    push bx
    push cx
    push dx
bn_skip:
    cmp byte ptr [si], ' '
    jne bn_start
    inc si
    jmp bn_skip
bn_start:
    xor bx, bx                  ; BX = number so far
    xor cx, cx                  ; CX = digits read
bn_loop:
    mov al, byte ptr [si]
    sub al, '0'
    cmp al, 9
    ja bn_end                   ; not a digit
    xor ah, ah
    push ax                     ; keep the digit
    mov ax, bx
    mov dx, 10
    mul dx
    mov bx, dx                  ; BX = high word (must stay 0)
    pop dx                      ; DX = digit
    or bx, bx
    jnz bn_fail                 ; more than 16 bits
    add ax, dx
    jc bn_fail                  ; more than 16 bits
    mov bx, ax
    inc cx
    inc si
    jmp bn_loop
bn_end:
    cmp cx, 0
    je bn_fail                  ; no digits
    mov al, byte ptr [si]
    cmp al, ' '
    je bn_ok
    cmp al, '#'
    jne bn_fail                 ; junk after the number
bn_ok:
    mov ax, bx
    clc
    jmp bn_exit
bn_fail:
    stc
bn_exit:
    pop dx
    pop cx
    pop bx
    ret
br_num endp

; ---------- writing a reply ----------

; br_sp: prints one space.
br_sp proc
    push dx
    mov dl, ' '
    call print_char
    pop dx
    ret
br_sp endp

; br_line: DX = '$'-terminated text; prints it and goes to the next line.
br_line proc
    call print_str
    call newline
    ret
br_line endp

; br_reply: runs the command in br_text and writes the reply into OUT.TXT.
; CF=1 if the reply could not be started (then the command was NOT run).
br_reply proc
    mov dx, offset br_out_name
    mov ax, 3D01h               ; DOS: open OUT.TXT for writing (it is not emptied)
    int 21h
    jnc rp_opened
    stc                         ; could not open it: nothing was run
    ret
rp_opened:
    mov word ptr br_out_handle, ax
    mov bx, ax
    mov dx, offset br_space
    mov cx, BR_OUT_SIZE
    mov ah, 40h                 ; DOS: overwrite the old reply with spaces (this removes
    int 21h                     ; its '#', so a half-written reply is never taken for a whole one)
    mov bx, word ptr br_out_handle
    mov ax, 4200h               ; DOS: back to the start of the file
    xor cx, cx
    xor dx, dx
    int 21h
    mov bx, 1
    mov ah, 45h                 ; DOS: duplicate the console handle
    int 21h
    jnc rp_duped
    mov bx, word ptr br_out_handle
    mov ah, 3Eh                 ; DOS: close OUT.TXT again
    int 21h
    stc                         ; nothing was run
    ret
rp_duped:
    mov word ptr br_old_stdout, ax
    mov bx, word ptr br_out_handle
    mov cx, 1
    mov ah, 46h                 ; DOS: handle 1 (the console) now points at OUT.TXT
    int 21h
    mov ax, word ptr br_cur_seq ; the reply starts with "SEQ|"
    call print_int
    mov dl, '|'
    call print_char
    call br_dispatch            ; the handler prints the rest of the reply
    mov dl, '#'
    call print_char             ; end marker: written last, so the reply is complete when it appears
rp_restore:
    mov bx, word ptr br_old_stdout
    mov cx, 1
    mov ah, 46h                 ; DOS: handle 1 is the console again
    int 21h
    mov bx, word ptr br_old_stdout
    mov ah, 3Eh                 ; DOS: close the copy of the console handle
    int 21h
    mov bx, word ptr br_out_handle
    mov ah, 3Eh                 ; DOS: close OUT.TXT
    int 21h
    clc
    ret
br_reply endp

; br_dispatch: finds the command word at br_text and runs its handler.
; The handlers may change any register; this routine saves them all.
br_dispatch proc
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    mov bx, offset br_verbs
bd_next:
    mov di, word ptr [bx]       ; DI = command word of this table entry
    or di, di
    jz bd_unknown               ; end of the table
    mov si, word ptr br_text
bd_cmp:
    mov al, byte ptr [di]
    cmp al, '$'
    je bd_word_end              ; the whole word matched
    cmp al, byte ptr [si]
    jne bd_skip
    inc di
    inc si
    jmp bd_cmp
bd_word_end:
    mov al, byte ptr [si]       ; the word must be followed by a space or '#'
    cmp al, ' '
    je bd_found
    cmp al, '#'
    je bd_found
bd_skip:
    add bx, 4                   ; next table entry
    jmp bd_next
bd_found:
    mov word ptr br_args, si
    call word ptr [bx + 2]      ; run the handler
    jmp bd_exit
bd_unknown:
    mov dx, offset br_e_unknown
    call br_line
bd_exit:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
br_dispatch endp

; ---------- shared replies (jumped to from the handlers; they end the handler) ----------

br_ok proc
    mov dx, offset br_s_ok
    jmp br_line
br_ok endp

br_err_invalid proc
    mov dx, offset br_e_invalid
    jmp br_line
br_err_invalid endp

br_err_soldout proc
    mov dx, offset br_e_soldout
    jmp br_line
br_err_soldout endp

br_err_busy proc
    mov dx, offset br_e_busy
    jmp br_line
br_err_busy endp

br_err_notx proc
    mov dx, offset br_e_notx
    jmp br_line
br_err_notx endp

br_err_amount proc
    mov dx, offset br_e_amount
    jmp br_line
br_err_amount endp

br_err_login proc
    mov dx, offset br_e_login
    jmp br_line
br_err_login endp

br_err_range proc
    mov dx, offset br_e_range
    jmp br_line
br_err_range endp

br_err_denom proc
    mov dx, offset br_e_denom
    jmp br_line
br_err_denom endp

; br_say_locked: AX = seconds left; prints "ERR LOCKED seconds".
br_say_locked proc
    push ax
    mov dx, offset br_s_locked
    call print_str
    call br_sp
    pop ax
    call print_int
    jmp newline
br_say_locked endp

; br_tx_end: closes the open purchase.
br_tx_end proc
    mov byte ptr br_tx, 0
    mov word ptr payment, 0
    ret
br_tx_end endp

; ---------- the command handlers (SI is set from br_args; the reply goes to the console handle) ----------

; STATE: the whole machine.
;   OK / P code name price stock (per product) / C denom count (per coin)
;   / S sales_count sales_total / L seconds_locked / A admin_session
;   / X purchase_open product inserted
h_state proc
    mov dx, offset br_s_ok
    call br_line
    xor si, si                  ; SI = offset in the tables
    mov bx, 1                   ; BX = product code
    mov cx, NUM_PRODUCTS
hs_prod:
    mov dl, 'P'
    call print_char
    call br_sp
    mov ax, bx
    call print_int
    call br_sp
    mov dx, word ptr [product_names + si]
    call print_str
    call br_sp
    mov ax, word ptr [product_prices + si]
    call print_int
    call br_sp
    mov ax, word ptr [product_stock + si]
    call print_int
    call newline
    add si, 2
    inc bx
    loop hs_prod
    xor si, si
    mov cx, NUM_DENOMS
hs_coin:
    mov dl, 'C'
    call print_char
    call br_sp
    mov ax, word ptr [denoms + si]
    call print_int
    call br_sp
    mov ax, word ptr [coin_stock + si]
    call print_int
    call newline
    add si, 2
    loop hs_coin
    mov dl, 'S'
    call print_char
    call br_sp
    mov ax, word ptr sales_count
    call print_int
    call br_sp
    mov ax, word ptr sales_total
    call print_int
    call newline
    mov dl, 'L'
    call print_char
    call br_sp
    call lock_remaining
    call print_int
    call newline
    mov dl, 'A'
    call print_char
    call br_sp
    mov al, byte ptr br_admin
    xor ah, ah
    call print_int
    call newline
    mov dl, 'X'
    call print_char
    call br_sp
    mov al, byte ptr br_tx
    xor ah, ah
    call print_int
    call br_sp
    mov al, byte ptr sel_code
    xor ah, ah
    call print_int
    call br_sp
    mov ax, word ptr payment
    call print_int
    jmp newline
h_state endp

; BUY n: starts a purchase of product n.  OK TX product price
h_buy proc
    mov si, word ptr br_args
    call br_num
    jnc hb_num
    jmp br_err_invalid
hb_num:
    cmp ax, 1
    jae hb_low
    jmp br_err_invalid
hb_low:
    cmp ax, NUM_PRODUCTS
    jbe hb_range
    jmp br_err_invalid
hb_range:
    cmp byte ptr br_tx, 0
    je hb_free
    jmp br_err_busy             ; finish or CANCEL the open purchase first
hb_free:
    mov bx, ax                  ; BX = product code
    call lookup_stock           ; AL = code -> AX = units left
    or ax, ax
    jnz hb_instock
    jmp br_err_soldout
hb_instock:
    mov ax, bx
    mov byte ptr sel_code, al
    call lookup_price           ; AL = code -> AX = price
    mov word ptr price, ax
    mov word ptr payment, 0
    mov byte ptr br_tx, 1
    mov dx, offset br_s_oktx
    call print_str
    call br_sp
    mov al, byte ptr sel_code
    xor ah, ah
    call print_int
    call br_sp
    mov ax, word ptr price
    call print_int
    jmp newline
h_buy endp

; PAY amount: inserts money (1..MAX_INPUT) into the open purchase.
;   OK SHORT missing
;   OK NOCHANGE refunded               (the machine lacks the coins; purchase ended)
;   OK SOLD / R item price paid change / K count denom (per coin used)
h_pay proc
    cmp byte ptr br_tx, 0
    jne hp_open
    jmp br_err_notx
hp_open:
    mov si, word ptr br_args
    call br_num
    jnc hp_num
    jmp br_err_amount
hp_num:
    cmp ax, 1
    jae hp_low
    jmp br_err_amount
hp_low:
    cmp ax, MAX_INPUT
    jbe hp_add
    jmp br_err_amount
hp_add:
    ; The total is below the price before every insertion and each insertion is
    ; at most MAX_INPUT, so this addition cannot overflow 16 bits.
    add word ptr payment, ax
    mov ax, word ptr payment
    mov bx, word ptr price
    call calc_change            ; CF=0: AX = change; CF=1: AX = shortfall
    jnc hp_enough
    push ax                     ; AX = shortfall
    mov dx, offset br_s_short
    call print_str
    call br_sp
    pop ax
    call print_int
    jmp newline
hp_enough:
    mov word ptr change, ax
    call break_change           ; CF=1: the machine lacks the coins
    jnc hp_sell
    mov dx, offset br_s_nochange
    call print_str
    call br_sp
    mov ax, word ptr payment
    call print_int
    call newline
    jmp br_tx_end               ; nothing is sold: the money is given back
hp_sell:
    mov al, byte ptr sel_code
    call take_stock             ; one unit leaves the machine
    call take_coins             ; the change coins leave the machine
    mov ax, word ptr payment
    call put_coins              ; the inserted money joins the machine's coins
    inc word ptr sales_count    ; record the completed sale
    mov ax, word ptr price
    add word ptr sales_total, ax
    mov dx, offset br_s_sold
    call br_line
    mov dl, 'R'
    call print_char
    call br_sp
    mov al, byte ptr sel_code
    call lookup_name            ; AL = code -> DX = name
    call print_str
    call br_sp
    mov ax, word ptr price
    call print_int
    call br_sp
    mov ax, word ptr payment
    call print_int
    call br_sp
    mov ax, word ptr change
    call print_int
    call newline
    xor si, si
    mov cx, NUM_DENOMS
hp_coin:
    mov ax, word ptr [denom_count + si]
    or ax, ax
    jz hp_next                  ; no coin of this value
    mov dl, 'K'
    call print_char
    call br_sp
    mov ax, word ptr [denom_count + si]
    call print_int
    call br_sp
    mov ax, word ptr [denoms + si]
    call print_int
    call newline
hp_next:
    add si, 2
    loop hp_coin
    call br_tx_end
    jmp save_state              ; keep the save file up to date
h_pay endp

; CANCEL: ends the open purchase.  OK REFUND amount
h_cancel proc
    mov dx, offset br_s_refund
    call print_str
    call br_sp
    mov ax, word ptr payment    ; 0 when nothing was inserted
    call print_int
    call newline
    jmp br_tx_end
h_cancel endp

; LOGIN code pin: opens an admin session when both the admin code and the PIN are right.
; A wrong code or a wrong PIN counts as one wrong try (same tries and lockout as the keyboard).
;   OK / ERR WRONG tries_left / ERR LOCKED seconds
h_login proc
    call lock_remaining         ; AX = seconds of lock left
    or ax, ax
    jz hl_free
    jmp br_say_locked
hl_free:
    mov si, word ptr br_args
    call br_num                 ; AX = the admin code
    jc hl_wrong
    cmp ax, word ptr admin_code
    jne hl_wrong
    call br_num                 ; AX = the PIN (SI is already past the code)
    jc hl_wrong
    cmp ax, word ptr admin_pin
    jne hl_wrong
    mov word ptr pin_fails, 0   ; a right PIN clears the count and the lock level
    mov word ptr lock_level, 0
    mov byte ptr br_admin, 1
    jmp br_ok
hl_wrong:
    inc word ptr pin_fails
    mov ax, PIN_TRIES
    sub ax, word ptr pin_fails  ; AX = tries left
    jbe hl_lock
    push ax
    mov dx, offset br_s_wrong
    call print_str
    call br_sp
    pop ax
    call print_int
    jmp newline
hl_lock:
    call begin_lock             ; starts the lock without printing anything
    mov ax, word ptr lock_len
    jmp br_say_locked
h_login endp

; LOGOUT: closes the admin session.  OK
h_logout proc
    mov byte ptr br_admin, 0
    jmp br_ok
h_logout endp

; REFILL PRODUCTS | REFILL COINS: fills every product or every coin (admin only).  OK
h_refill proc
    cmp byte ptr br_admin, 0
    jne hr_admin
    jmp br_err_login
hr_admin:
    mov si, word ptr br_args
hr_skip:
    cmp byte ptr [si], ' '
    jne hr_word
    inc si
    jmp hr_skip
hr_word:
    mov al, byte ptr [si]
    or al, 20h                  ; to lower case
    cmp al, 'p'
    je hr_products
    cmp al, 'c'
    je hr_coins
    jmp br_err_invalid
hr_products:
    mov di, offset product_stock
    mov cx, NUM_PRODUCTS
    mov ax, FULL_STOCK
    jmp hr_fill
hr_coins:
    mov di, offset coin_stock
    mov cx, NUM_DENOMS
    mov ax, FULL_COINS
hr_fill:
    call fill_words
    call save_state
    jmp br_ok
h_refill endp

; SETSTOCK product quantity: sets one product's stock (admin only).  OK
h_setstock proc
    cmp byte ptr br_admin, 0
    jne hss_admin
    jmp br_err_login
hss_admin:
    mov si, word ptr br_args
    call br_num                 ; AX = product code
    jnc hss_p
    jmp br_err_invalid
hss_p:
    cmp ax, 1
    jae hss_low
    jmp br_err_invalid
hss_low:
    cmp ax, NUM_PRODUCTS
    jbe hss_pok
    jmp br_err_invalid
hss_pok:
    mov bx, ax                  ; BX = product code (br_num keeps BX)
    call br_num                 ; AX = quantity
    jnc hss_q
    jmp br_err_invalid
hss_q:
    cmp ax, MAX_INPUT
    jbe hss_set
    jmp br_err_range
hss_set:
    dec bx
    shl bx, 1                   ; BX = table offset
    mov word ptr [product_stock + bx], ax
    call save_state
    jmp br_ok
h_setstock endp

; SETCOIN denomination quantity: sets one coin count (admin only).  OK
h_setcoin proc
    cmp byte ptr br_admin, 0
    jne hsc_admin
    jmp br_err_login
hsc_admin:
    mov si, word ptr br_args
    call br_num                 ; AX = coin value
    jnc hsc_d
    jmp br_err_invalid
hsc_d:
    mov dx, ax                  ; DX = coin value (br_num keeps DX)
    call br_num                 ; AX = quantity
    jnc hsc_q
    jmp br_err_invalid
hsc_q:
    cmp ax, MAX_INPUT
    jbe hsc_find0
    jmp br_err_range
hsc_find0:
    xor bx, bx
hsc_find:
    cmp dx, word ptr [denoms + bx]
    je hsc_found
    add bx, 2
    cmp bx, NUM_DENOMS * 2
    jb hsc_find
    jmp br_err_denom            ; not one of the denominations
hsc_found:
    mov word ptr [coin_stock + bx], ax
    call save_state
    jmp br_ok
h_setcoin endp

; SETCODE n: new admin code, at least ADMIN_CODE_MIN (admin only).  OK
h_setcode proc
    cmp byte ptr br_admin, 0
    jne hsd_admin
    jmp br_err_login
hsd_admin:
    mov si, word ptr br_args
    call br_num
    jnc hsd_n
    jmp br_err_invalid
hsd_n:
    cmp ax, ADMIN_CODE_MIN
    jae hsd_set
    jmp br_err_range
hsd_set:
    mov word ptr admin_code, ax
    call save_state
    jmp br_ok
h_setcode endp

; SETPIN n: new PIN, PIN_MIN..PIN_MAX (admin only).  OK
h_setpin proc
    cmp byte ptr br_admin, 0
    jne hsn_admin
    jmp br_err_login
hsn_admin:
    mov si, word ptr br_args
    call br_num
    jnc hsn_n
    jmp br_err_invalid
hsn_n:
    cmp ax, PIN_MIN
    jae hsn_low
    jmp br_err_range
hsn_low:
    cmp ax, PIN_MAX
    jbe hsn_set
    jmp br_err_range
hsn_set:
    mov word ptr admin_pin, ax
    call save_state
    jmp br_ok
h_setpin endp

; QUIT: saves and ends bridge mode.  OK
h_quit proc
    mov byte ptr br_quit, 1
    call save_state
    jmp br_ok
h_quit endp

; ---------- the main loop of bridge mode ----------

; br_poll: answers a new complete command in CMD.TXT, if there is one.
br_poll proc
    call br_read_cmd
    jc bpo_none                 ; could not read it: try again later
    mov di, offset br_inbuf
    mov cx, BR_CMD_SIZE
    mov al, '#'
    cld
    repne scasb
    jne bpo_none                ; no end marker yet: the record is not complete
    call br_parse_seq
    jc bpo_none
    cmp ax, word ptr br_last_seq
    je bpo_none                 ; this command was already answered
    mov word ptr br_cur_seq, ax
    call br_reply
    jc bpo_none                 ; the reply could not be started: nothing ran, try again
    mov ax, word ptr br_cur_seq
    mov word ptr br_last_seq, ax
bpo_none:
    ret
br_poll endp

; br_wait_tick: waits for the next BIOS clock tick (about 55 ms).
br_wait_tick proc
    push ax
    push bx
    push cx
    push dx
    mov ah, 0
    int 1Ah                     ; CX:DX = clock ticks
    mov bx, dx
bw_loop:
    mov ah, 0
    int 1Ah
    cmp dx, bx
    je bw_loop                  ; same tick: keep waiting
    pop dx
    pop cx
    pop bx
    pop ax
    ret
br_wait_tick endp

; bridge_run: bridge mode. Returns after the QUIT command (or at once if the
; files cannot be created).
bridge_run proc
    push ds
    pop es                      ; ES = DS for the string instructions
    call br_make_files
    jc brn_error
    mov dx, offset br_msg_start
    call print_str
brn_loop:
    call br_poll
    cmp byte ptr br_quit, 0
    jne brn_end
    call br_wait_tick
    jmp brn_loop
brn_error:
    mov dx, offset br_msg_error
    call print_str
brn_end:
    ret
bridge_run endp
