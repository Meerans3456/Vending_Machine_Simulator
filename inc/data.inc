; ==========================================================
; data.inc - data segment: tables, messages, variables
; Requires: nothing (include FIRST)
; ==========================================================

.data
NUM_PRODUCTS    equ 6
NUM_DENOMS      equ 5

; --- Product table (parallel arrays; code i -> index i-1) ---
product_codes   db 1, 2, 3, 4, 5, 6
product_prices  dw 15, 25, 10, 20, 30, 35

; --- Product names (for the receipt; '$'-terminated) ---
pname1          db 'Chips$'
pname2          db 'Soda$'
pname3          db 'Candy$'
pname4          db 'Water$'
pname5          db 'Coffee$'
pname6          db 'Juice$'
product_names   dw pname1, pname2, pname3, pname4, pname5, pname6

; --- Denomination table ---
denoms          dw 50, 25, 10, 5, 1
denom_count     dw 0, 0, 0, 0, 0

; --- Messages (every string ends with '$') ---
crlf            db 0Dh, 0Ah, '$'

msg_menu        db 0Dh, 0Ah
                db '==== VENDING MACHINE ====', 0Dh, 0Ah
                db '1. Chips  - 15', 0Dh, 0Ah
                db '2. Soda   - 25', 0Dh, 0Ah
                db '3. Candy  - 10', 0Dh, 0Ah
                db '4. Water  - 20', 0Dh, 0Ah
                db '5. Coffee - 30', 0Dh, 0Ah
                db '6. Juice  - 35', 0Dh, 0Ah
                db '0. Exit', 0Dh, 0Ah, '$'

msg_select      db 0Dh, 0Ah, 'Enter product code: $'
msg_pay         db 0Dh, 0Ah, 'Insert money (0 to cancel): $'
msg_price       db 0Dh, 0Ah, 'Price: $'
msg_change      db 0Dh, 0Ah, 'Change: $'
msg_short       db 0Dh, 0Ah, 'Insufficient funds. Short by: $'
msg_invalid     db 0Dh, 0Ah, 'Invalid product code. Try again.$'
msg_badnum      db 0Dh, 0Ah, 'Invalid number. Try again.$'
msg_cancel      db 0Dh, 0Ah, 'Transaction cancelled.$'
msg_refund      db ' Refunded: $'
msg_bye         db 0Dh, 0Ah, 'Thank you. Goodbye.$'
msg_x           db ' x $'

; --- Receipt and pause messages ---
msg_rcpt_top    db 0Dh, 0Ah, '======== RECEIPT ========', 0Dh, 0Ah, '$'
msg_rcpt_end    db '=========================', 0Dh, 0Ah, '$'
msg_r_item      db 'Item:   $'
msg_r_price     db 'Price:  $'
msg_r_paid      db 'Paid:   $'
msg_press       db 0Dh, 0Ah, 'Press any key to continue...$'

; --- Variables ---
sel_code        db 0
price           dw 0
payment         dw 0
change          dw 0
shortfall       dw 0
