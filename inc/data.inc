; ==========================================================
; data.inc - data segment: tables, messages, variables
; Requires: nothing (include FIRST)
; ==========================================================

.data

; --- Constants ---
CR              equ 0Dh         ; carriage return (also the Enter key)
LF              equ 0Ah         ; line feed
MAX_INPUT       equ 999         ; largest amount typed at a payment or count prompt
FULL_STOCK      equ 3           ; units of each product when full (the machine starts full)
FULL_COINS      equ 10          ; coins of each denomination when full (the machine starts full)
ADMIN_CODE      equ 31415       ; secret number typed at the product prompt to enter admin mode
ADMIN_PIN       equ 1234        ; PIN asked after the secret number
PIN_MAX         equ 9999        ; largest number the PIN prompt accepts
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

; --- State saved between runs: written to and read from the save file as one block ---
save_block      label byte
save_check      dw 0                                ; holds SAVE_BYTES in the file: tells load_state it matches these tables
product_stock   dw NUM_PRODUCTS dup (FULL_STOCK)    ; units left of each product
coin_stock      dw NUM_DENOMS dup (FULL_COINS)      ; coins left of each denomination
SAVE_BYTES      equ $ - save_block                  ; size of the block
load_buffer     db SAVE_BYTES dup (0)               ; load_state reads the file here first
state_file      db 'VENDING.DAT', 0                 ; the save file, in the folder the program runs from

; --- Messages (every string ends with '$') ---

; Menu pieces (the product lines are built from the tables by display_menu)
msg_menu_top    db CR, LF, '==== VENDING MACHINE ====', CR, LF, '$'
msg_menu_exit   db '0. Exit', CR, LF, '$'
msg_dot         db '. $'
msg_dash        db ' - $'
msg_sold_tag    db ' (sold out)$'

msg_select      db CR, LF, 'Enter product code: $'
msg_pay         db CR, LF, 'Insert money (0 to cancel): $'
msg_price       db CR, LF, 'Price: $'
msg_change      db CR, LF, 'Change: $'
msg_short       db CR, LF, 'Insufficient funds. Short by: $'
msg_invalid     db CR, LF, 'Invalid product code. Try again.$'
msg_badnum      db CR, LF, 'Invalid number. Try again.$'
msg_soldout     db CR, LF, 'Sorry, that product is sold out.$'
msg_cancel      db CR, LF, 'Transaction cancelled.$'
msg_nochange    db CR, LF, 'Sorry, no exact change available.$'

; --- Admin mode messages ---
msg_admin_menu  db CR, LF, '==== ADMIN MODE ====', CR, LF
                db '1. View stock, coins and sales', CR, LF
                db '2. Refill all products', CR, LF
                db '3. Refill all coins', CR, LF
                db '4. Set the stock of one product', CR, LF
                db '5. Set the coins of one denomination', CR, LF
                db '0. Leave admin mode', CR, LF, '$'
msg_admin_ask   db CR, LF, 'Admin choice: $'
msg_pin         db CR, LF, 'Enter PIN: $'
msg_wrongpin    db CR, LF, 'Wrong PIN.$'
msg_ask_prod    db CR, LF, 'Product code (0 to go back): $'
msg_ask_stock   db CR, LF, 'New stock: $'
msg_ask_coin    db CR, LF, 'Coin value (0 to go back): $'
msg_ask_count   db CR, LF, 'New number of coins: $'
msg_nocoin      db CR, LF, 'No such coin.$'
msg_done        db CR, LF, 'Done.$'
msg_hdr_stock   db CR, LF, 'Product stock:', CR, LF, '$'
msg_hdr_coins   db 'Coins:', CR, LF, '$'
msg_colon       db ': $'
msg_refund      db ' Refunded: $'
msg_bye         db CR, LF, 'Thank you. Goodbye.$'
msg_sales       db CR, LF, 'Sales made: $'
msg_taken       db CR, LF, 'Total taken: $'
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
sales_count     dw 0            ; sales completed this session
sales_total     dw 0            ; total price of those sales
