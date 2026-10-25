; ==========================================================
; data.inc - data segment: tables, messages, variables
; Requires: nothing (include FIRST)
; ==========================================================

.data

; --- Constants ---
CR              equ 0Dh         ; carriage return (also the Enter key)
LF              equ 0Ah         ; line feed
MAX_INPUT       equ 999         ; largest number read_int accepts
TEXT_ATTR       equ 07h         ; screen attribute: light grey on black
SCREEN_END      equ 184Fh       ; bottom-right corner of the 80x25 screen (row 24, col 79)

; --- Product table (parallel arrays; code i -> index i-1) ---
product_prices  dw 15, 25, 10, 20, 30, 35
NUM_PRODUCTS    equ ($ - product_prices) / 2    ; number of products

; --- Product names (for the receipt; '$'-terminated) ---
pname1          db 'Chips$'
pname2          db 'Soda$'
pname3          db 'Candy$'
pname4          db 'Water$'
pname5          db 'Coffee$'
pname6          db 'Juice$'
product_names   dw pname1, pname2, pname3, pname4, pname5, pname6
.errnz (($ - product_names) / 2) - NUM_PRODUCTS   ; build stops if names and prices differ in number

; --- Denomination table ---
denoms          dw 50, 25, 10, 5, 1             ; must stay in descending order
NUM_DENOMS      equ ($ - denoms) / 2
denom_count     dw NUM_DENOMS dup (0)

; --- Messages (every string ends with '$') ---

; Menu pieces (the product lines are built from the tables by display_menu)
msg_menu_top    db CR, LF, '==== VENDING MACHINE ====', CR, LF, '$'
msg_menu_exit   db '0. Exit', CR, LF, '$'
msg_dot         db '. $'
msg_dash        db ' - $'

msg_select      db CR, LF, 'Enter product code: $'
msg_pay         db CR, LF, 'Insert money (0 to cancel): $'
msg_price       db CR, LF, 'Price: $'
msg_change      db CR, LF, 'Change: $'
msg_short       db CR, LF, 'Insufficient funds. Short by: $'
msg_invalid     db CR, LF, 'Invalid product code. Try again.$'
msg_badnum      db CR, LF, 'Invalid number. Try again.$'
msg_cancel      db CR, LF, 'Transaction cancelled.$'
msg_refund      db ' Refunded: $'
msg_bye         db CR, LF, 'Thank you. Goodbye.$'
msg_x           db ' x $'

; --- Receipt and pause messages ---
msg_rcpt_top    db CR, LF, '======== RECEIPT ========', CR, LF, '$'
msg_rcpt_end    db '=========================', CR, LF, '$'
msg_r_item      db 'Item:   $'
msg_r_price     db 'Price:  $'
msg_r_paid      db 'Paid:   $'
msg_press       db CR, LF, 'Press any key to continue...$'

; --- Variables ---
sel_code        db 0
price           dw 0
payment         dw 0
change          dw 0
shortfall       dw 0
