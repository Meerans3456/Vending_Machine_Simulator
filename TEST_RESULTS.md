# Vending Machine Simulator: Test Results

**Version tested:** the current program, with the extended features (stock levels, coin inventory, admin mode, save file) the phase 3 changes (Backspace, aligned menus, saved sales totals, admin lockout, changeable admin code and PIN) and the phase 4 changes (coins 100, 50, 20, 10, 5, 1, bridge mode and the Python window).
**Environment:** Windows, JWasm v2.11 (`-0 -mz`), run in DOSBox.
**How results were obtained:** each check was run by the team in DOSBox and compared with the expected output. "PASS" means the observed output matched the expected output. The checks for the extended features (sections 1.6 and 2.5) for phase 3 (sections 1.7 and 2.6) and for phase 4 (sections 1.8, 2.7 and 2.8) were run in the same way and all passed. The Python window (section 2.8) was tried by hand. Checks marked "Superseded" no longer apply because the coin inventory changed the behaviour; they are replaced by the checks named in the table.

Rebuild any program with `.\build.bat <name>`, then run `<name>.exe` in DOSBox.

---

## How to run the tests

1. **Build every program** (PowerShell, from the project folder):

   ```powershell
   'vending','t_utils','t_lookup','t_stock','t_input','t_math','t_output','t_admin','t_file' | ForEach-Object { .\build.bat $_ }
   ```

2. **Start DOSBox and mount the project folder** (see [Setup.md](Setup.md)). If DOSBox was already open, press **Ctrl+F4** first so it sees the new files.
3. **Run each program** and compare its output with the tables below.

| Program | Interactive? | What to do |
|---|---|---|
| `t_utils.exe` | one key press | Read the printed lines (sections 1.1 and 1.7), including the `print_spaces`, `print_name_col` and `print_int3` lines. When asked, press a key: the screen must clear and the next text start at the top-left. |
| `t_lookup.exe` | no | It prints three blocks (price, name, offset) for the codes 0, 1, 3, 6, 7, 9. Compare with section 1.3. |
| `t_input.exe` | yes | **First phase** (`read_int>` prompt, limit 999): type `42`, `abc`, an empty line (just Enter), `999`, `1000`, `70000`, `12x`, and finally `0` to end the phase. **Second phase** (limit 65535): type `70000`, then `0`. **Third phase** (`read_product`, product 3 is sold out): type `9`, `abc`, `3`, then `4`. **Fourth phase** (`ask_int`): type `abc`, then `25`. In the first phase also try `a`, Backspace, `5`, Enter and `1`, `2`, Backspace, `5`, Enter. Compare with sections 1.2, 1.6 and 1.7. |
| `t_stock.exe` | no | It prints four blocks (`lookup_stock`, `take_stock`, `take_coins`, `put_coins`). Compare with section 1.6. |
| `t_math.exe` | no | It prints the menu (product 3 sold out), then the `calc_change` and `break_change` results, including a block with limited coins (the menu may scroll off the top; scroll up or enlarge the window). Compare with sections 1.4 and 1.6. |
| `t_output.exe` | no | It prints five change breakdowns, three receipts, then two sales summaries. Compare with sections 1.5 and 1.6. |
| `t_admin.exe` | yes | Parts 1 to 4 are automatic (press a key after each screen). Part 5: type `2`, then `7`. Part 6: type `20`, then `4`. Part 7 (`ask_pin`, keys shown as `*`) runs once: type `0`, then run it again with `9999`, and again with `1234`. Part 8 is automatic (lock timing with fake clock values). Part 9: type `12345` twice. Part 10: type `4321` twice. Compare with sections 1.6 and 1.7. |
| `t_file.exe` | no | It prints three blocks (products, coins, sales and admin values) using its own `T_STATE.DAT`, so your real `VENDING.DAT` is not touched. Compare with sections 1.6 and 1.7. |
| `gui\test_bridge.py` | no | Delete `VENDING.DAT`, start `vending.exe /g` in DOSBox, then run `py gui\test_bridge.py` from the project folder. It prints a PASS or FAIL line for each of its checks and ends with `QUIT`. Delete `VENDING.DAT` afterwards. Compare with section 1.8. |
| the Python window | yes | `py gui\vending_gui.py`; follow the scenarios in section 2.8. |
| `vending.exe` | yes | Follow the scenarios in section 2 (for example code `1`, pay `50`). |

Sections 2.5 and 2.6 need a known starting state: delete `VENDING.DAT` before the first scenario so the machine starts full (3 of each product, 10 of each coin), and use the admin refill options between scenarios when needed.

Section 3 (design checks) is done by editing `inc\data.inc`, rebuilding `vending`, and undoing the edit afterwards (delete `VENDING.DAT` after changing the number of products).

**Change history**
- *First version:* core purchase flow, with a test program per module.
- *Payment, receipt and screen features:* running total with retry, cancel with refund, receipt, screen clear and pause. Earlier checks were re-run afterwards.
- *Code refinement:* the menu is printed from the tables, lookups moved into their own module, unused data removed, `break_change` preserves AX, named constants and computed table sizes. All earlier checks were re-run; the checks marked **NEW** were added.
- *Extended features:* stock levels with sold-out products, coin inventory (change taken from the coins in stock, inserted money added to it), exact-change refusal with refund, sales summary, PIN-protected admin mode, and a save file. `read_payment` was replaced by the shared `ask_int`. The checks in sections 1.6 and 2.5 were added and passed, and three earlier checks were superseded.
- *Phase 3:* Backspace support in number entry (the line is checked at Enter), aligned customer menu and admin product list, sales totals saved in `VENDING.DAT`, admin PIN hidden with `*`, `0` to go back, wrong PINs re-ask only the PIN, lockout of 1, 5, 10, 20, 30 and then 60 minutes after 3 wrong PINs, and admin menu choices to change the admin code and the PIN (both saved). The checks in sections 1.7 and 2.6 were added and passed.
- *Phase 4:* the machine's coins are now 100, 50, 20, 10, 5 and 1 (one line in `data.inc`; the change-making, stock, save file and admin code follow the table), bridge mode (`vending.exe /g`, `inc\bridge.inc`) with the `CMD.TXT` / `OUT.TXT` protocol, the Python client and its test (`gui\bridge.py`, `gui\test_bridge.py`), and the tkinter window with its settings file and automatic DOSBox start. The coin values in the earlier checks were updated; the checks in sections 1.8, 2.7 and 2.8 were added and passed.

---

## 1. Module-level checks

Each module has its own standalone test program, so a failure points to one module.

| Test program | Covers | Source module |
|---|---|---|
| `t_utils.exe` | `print_str`, `print_char`, `print_int`, `print_spaces`, `print_name_col`, `print_int3`, `clear_screen`, `wait_key` | `utils.inc` |
| `t_input.exe` | `read_int` (interactive), `read_product`, `ask_int` | `input.inc` |
| `t_lookup.exe` | `code_to_offset`, `lookup_price`, `lookup_name` | `lookup.inc` |
| `t_math.exe` | `display_menu`, `calc_change`, `break_change` | `menu.inc`, `math.inc` |
| `t_output.exe` | `print_change`, `print_receipt`, `print_summary` | `output.inc` |
| `t_stock.exe` | `lookup_stock`, `take_stock`, `take_coins`, `put_coins` | `stock.inc` |
| `t_admin.exe` | `admin_view`, `refill_products`, `refill_coins`, `set_product_stock`, `set_coin_count`, `ask_pin`, `lock_left_at`, `start_lock`, `change_admin_code`, `change_admin_pin` | `admin.inc` |
| `t_file.exe` | `save_state`, `load_state` (stock, coins, sales, admin values) | `file.inc` |

### 1.1 `t_utils` (utils.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 1 | `print_str` | offset of a `$`-terminated text | the text is printed (used by every message in all programs) | PASS |
| 2 | `print_int` | AX = 0 | prints `0` | PASS |
| 3 | `print_int` | AX = 12345 | prints `12345` | PASS |
| 4 | `print_char` | DL = `'A'` | prints `A` | PASS |
| 5 | `print_int` | AX = 7, 10, 65535 | prints `7`, `10`, `65535` | PASS |
| 6 | `clear_screen`, `wait_key` | press a key at the prompt | the prompt waits; the screen clears; the next text starts at the top-left | PASS |

### 1.2 `t_input` (input.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 7 | `read_int` | `42` and Enter | AX = 42, CF = 0 | PASS |
| 8 | `read_int` | `abc` and Enter | CF = 1 | PASS |
| 9 | `read_int` | Enter alone (no digits) | CF = 1 | PASS |
| 10 | `read_int` | `999` | AX = 999, CF = 0 | PASS |
| 11 | `read_int` | `1000` | CF = 1 (over the limit) | PASS |
| 12 | `read_int` | `70000` | CF = 1 | PASS |
| 13 | `read_int` | `12x` | CF = 1 | PASS |
| 14 | `read_int` | `abc`, then the next prompt | the rest of the line is discarded; the next prompt is clean | PASS |
| 15 | `read_product` | `9`, `abc`, then `4` | invalid code, invalid number, then AL = 4 | PASS |
| 16 | `read_payment` | `abc`, then `0` (or `50`) | routine replaced by `ask_int`; see check 40 | Superseded |

### 1.3 `t_lookup` (lookup.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 17 | `lookup_price` | AL = 3 | AX = 10, CF = 0 | PASS |
| 18 | `lookup_price` | AL = 9 | CF = 1 | PASS |
| 19 | `lookup_price` | AL = 0, 1, 6, 7 | CF = 1, 15, 35, CF = 1 | PASS |
| 20 | `lookup_name` | AL = 0, 1, 3, 6, 7, 9 | not found, `Chips`, `Candy`, `Juice`, not found, not found | PASS |
| 21 | `code_to_offset` | AL = 0, 1, 3, 6, 7, 9 | CF = 1, 0, 4, 10, CF = 1, CF = 1 | PASS (**NEW**) |

### 1.4 `t_math` (menu.inc, math.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 22 | `calc_change` | AX = 50, BX = 25 | AX = 25, CF = 0 | PASS |
| 23 | `calc_change` | AX = 10, BX = 25 | AX = 15, CF = 1 | PASS |
| 24 | `calc_change` | AX = 25, BX = 25 | AX = 0, CF = 0 (exact payment) | PASS |
| 25 | `break_change` | AX = 35 | counts 0, 0, 1, 1, 1, 0 (coins 100, 50, 20, 10, 5, 1) | PASS |
| 26 | `break_change` | AX = 0 | all counts 0 | PASS |
| 27 | `break_change` | AX = 100 | counts 1, 0, 0, 0, 0, 0 | PASS |
| 28 | `break_change` | AX = 99 | counts 0, 1, 2, 0, 1, 4 | PASS |
| 29 | `break_change` | AX = 964 | counts 9, 1, 0, 1, 0, 4 | PASS |
| 30 | `break_change` | each amount above | AX has the same value after the call | PASS (**NEW**) |

The menu printed at the top of `t_math.exe` is also checked by eye; see end-to-end check 19.

### 1.5 `t_output` (output.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 31 | `print_change` | change 35, counts 0, 0, 1, 1, 1, 0 | `Change: 35`, `1 x 20`, `1 x 10`, `1 x 5` | PASS |
| 32 | `print_change` | change 0, all counts 0 | `Change: 0`, no breakdown lines | PASS |
| 33 | `print_change` | change 99, counts 0, 1, 2, 0, 1, 4 | `Change: 99`, `1 x 50`, `2 x 20`, `1 x 5`, `4 x 1` | PASS |
| 34 | `print_change` | change 964, counts 9, 1, 0, 1, 0, 4 | `Change: 964`, `9 x 100`, `1 x 50`, `1 x 10`, `4 x 1` | PASS |
| 35 | `print_change` | change 1, counts 0, 0, 0, 0, 0, 1 | `Change: 1`, `1 x 1` | PASS |
| 36 | `print_receipt` | Chips, price 15, paid 50, change 35 | banner, `Item:   Chips`, `Price:  15`, `Paid:   50`, `Change: 35`, `1 x 20`, `1 x 10`, `1 x 5`, footer | PASS |
| 37 | `print_receipt` | Soda, price 25, paid 25, change 0 | receipt with `Change: 0` and no breakdown lines | PASS |
| 38 | `print_receipt` | Juice, price 35, paid 100, change 65 | `Paid:   100`, `Change: 65`, `1 x 50`, `1 x 10`, `1 x 5` (three-digit alignment) | PASS |

### 1.6 Extended features (module level)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 39 | `read_product` | `9`, `abc`, `3`, then `4` (product 3 is sold out) | invalid code, invalid number, sold-out message, then AX = 4 | PASS |
| 40 | `ask_int` | limit 999: `abc`, then `25` | `abc` is rejected and the prompt repeats; then AX = 25 | PASS |
| 41 | `read_product` | `31415` | AX = 31415 (the admin code is accepted) | PASS |
| 42 | `lookup_stock` | AL = 1, 6, 0, 9 | 3, 3, not found, not found | PASS |
| 43 | `take_stock` | product 2, four times; then code 9 | `ok` three times (2, 1, 0 left); then `refused`, still 0; code 9 refused; product 1 still 3 | PASS |
| 44 | `take_coins` | coins 10 10 10 10 10 10; counts 1 0 2 0 3 0 taken twice | `9 10 8 10 7 10`, then `8 10 6 10 4 10` | PASS |
| 45 | `put_coins` | AX = 35, then 64 | `8 10 7 11 5 10`, then `8 11 7 12 5 14` | PASS |
| 46 | `break_change` (limited coins) | two 1-coins left: AX = 2, then 3; no 20-coins: AX = 30, then 35 | `0 0 0 0 0 2`; no exact change; `0 0 0 3 0 0`; `0 0 0 3 1 0` | PASS |
| 47 | `break_change` | AX = 964 with 10 of each coin | counts 9, 1, 0, 1, 0, 4 (the machine holds enough 100-coins) | PASS |
| 48 | `display_menu` | product 3 sold out | the Candy line ends with `(sold out)`; other lines normal | PASS |
| 49 | `print_summary` | 3 sales / total 70; then 0 / 0 | shows 3 and 70; then 0 and 0 | PASS |
| 50 | `admin_view` | starting state | every product 3, every coin 10, sales 0 | PASS |
| 51 | `admin_view` | Chips 0, Soda 1, 100-coins 0, 50-coins 3 set by hand | the view shows those values | PASS |
| 52 | `refill_products` | after check 51 | every product back to 3; coin counts unchanged | PASS |
| 53 | `refill_coins` | after check 52 | every coin back to 10 | PASS |
| 54 | `set_product_stock` | `2`, then `7` | product 2 shows 7 | PASS |
| 55 | `set_coin_count` | `20`, then `4` | the 20-coins show 4 | PASS |
| 56 | `ask_pin` | `0`, then `9999`, then `1234` (keys shown as `*`) | `PIN cancelled` for 0, `PIN rejected` for 9999, `PIN accepted` for 1234 | PASS |
| 57 | `load_state` | no save file | nothing changes: products 3 3 3 3 3 3, coins 10 10 10 10 10 10, sales 0 0, admin 31415 1234 | PASS |
| 58 | `save_state`, `load_state` | save `0 2 3 3 3 3 / 7 10 10 10 10 10 / sales 4 65 / admin 12345 4321`, scramble every value to 9, load | the saved values come back | PASS |
| 59 | `load_state` | a 3-byte file | the file is ignored; the values stay as they were (all 9) | PASS |

### 1.7 Phase 3 (module level)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 60 | `read_int` | `a`, Backspace, `5`, Enter | the letter is erased; AX = 5, CF = 0 | PASS |
| 61 | `read_int` | `1`, `2`, Backspace, `5`, Enter | AX = 15, CF = 0 | PASS |
| 62 | `read_int` | `12x`, Backspace, Enter | AX = 12, CF = 0 | PASS |
| 63 | `read_int` | Backspace with nothing typed, then `7`, Enter | nothing is erased; AX = 7, CF = 0 | PASS |
| 64 | `print_spaces` | AX = 3, then AX = 0 | three spaces between the brackets; nothing printed for 0 | PASS |
| 65 | `print_name_col` | `Soda` | `Soda` followed by spaces up to the name column width | PASS |
| 66 | `print_int3` | AX = 5, 15, 150 | `  5`, ` 15`, `150` (right-aligned in 3 columns) | PASS |
| 67 | `display_menu` | product 3 sold out | names, dashes and prices line up in columns; `(sold out)` still shown | PASS |
| 68 | `admin_view`, `set_product_stock` | open the admin view and choice 4 | the product list is shown in the same aligned columns, with stock right-aligned | PASS |
| 69 | `lock_left_at` | lock started at 1000, length 300, now 1100 | AX = 200 | PASS |
| 70 | `lock_left_at` | same lock, now 1300 | AX = 0 and the lock is cleared | PASS |
| 71 | `lock_left_at` | no lock running, now 1100 | AX = 0 | PASS |
| 72 | `lock_left_at` | lock started 10 s before midnight, length 60, now 20 s after midnight | AX = 30 | PASS |
| 73 | `lock_left_at` | same lock, now 2 s after midnight | AX = 48 | PASS |
| 74 | `start_lock` | lock level 0 | `locked for 1 minute(s)`; lock level becomes 1 | PASS |
| 75 | `start_lock` | lock level at the top | `locked for 60 minute(s)`; lock level stays 5 | PASS |
| 76 | `ask_pin` | `0` | CF = 1, AX = 0 (`PIN cancelled`) | PASS |
| 77 | `ask_pin` | `9999` | CF = 1, AX = 1 (`PIN rejected`); keys shown as `*` | PASS |
| 78 | `ask_pin` | `1234` | CF = 0 (`PIN accepted`); keys shown as `*` | PASS |
| 79 | `change_admin_code` | `12345` twice | `admin_code` = 12345 | PASS |
| 80 | `change_admin_pin` | `4321` twice (shown as `*`) | `admin_pin` = 4321 | PASS |

### 1.8 Python bridge (`gui\test_bridge.py`)

The test runs against `vending.exe /g` started with no `VENDING.DAT` (all 10 coins, 3 of each product, default admin code 31415 and PIN 1234) and prints one PASS or FAIL line per check. It ends with `QUIT`.

| Group | What is checked | Checks | Result |
|---|---|---|---|
| `STATE` | 6 products (Chips 15 x 3), 6 coins (100, 50, 20, 10, 5, 1) with 10 each, no sales, no lock, no session, no purchase open | 3 | PASS |
| Purchase with change | `BUY 1` (`OK TX 1 15`), purchase open in `STATE`, `BUY` refused while open (`ERR busy`), `PAY 5` (`OK SHORT 10`), `PAY 50` (`OK SOLD`, `R Chips 15 55 40`, `K 2 20`), then stock, sales (1 of 15) and coins `10 11 8 10 11 10` | 9 | PASS |
| Exact payment | `BUY 3`, `PAY 10` gives `R Candy 10 10 0` and no `K` line | 2 | PASS |
| Errors and cancel | `PAY` without a purchase (`ERR notx`), bad `BUY` codes (`ERR invalid`), bad `PAY` amounts (`ERR amount`), `PAY` short then `CANCEL` (`OK REFUND 5`), second `CANCEL` (`OK REFUND 0`), stock unchanged, unknown command (`ERR unknown`) | 15 | PASS |
| Admin needs a login | `REFILL COINS` and `SETSTOCK` before `LOGIN` give `ERR login`; `LOGIN 31415 1234` gives `OK` and `STATE` shows the session | 4 | PASS |
| Admin commands | `SETSTOCK` (valid, bad product, `ERR range`), `SETCOIN 20 4`, `SETCOIN 25 1` and `SETCOIN 7 1` (`ERR denom`) | 8 | PASS |
| No exact change | 20-, 10-, 5- and 1-coins set to 0, then `BUY 3` and `PAY 50` gives `OK NOCHANGE 50`; nothing sold, purchase closed | 7 | PASS |
| Refills | `REFILL COINS`, `REFILL PRODUCTS` (everything back to 10 and 3), `REFILL XYZ` (`ERR invalid`) | 5 | PASS |
| Admin code and PIN rules | `SETCODE 5`, `SETPIN 12` and `SETPIN 12345` give `ERR range`; `LOGOUT`; `REFILL COINS` gives `ERR login` again | 5 | PASS |
| Lockout | `LOGIN 31415 1` (`ERR WRONG 2`), `LOGIN 1 1234` (`ERR WRONG 1`), `LOGIN 31415 3` (`ERR LOCKED 60`), a right login while locked gives `ERR LOCKED` with 1 to 60 seconds, `STATE` shows the lock | 4 | PASS |
| `QUIT` | `OK`, and `vending.exe` ends | 1 | PASS |
| **Total** | | **62** | **62 passed** |

---

## 2. End-to-end checks (`vending.exe`)

### 2.1 Sales

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 1 | Exact payment | code 2, pay 25 | receipt with `Change: 0`, no breakdown lines | PASS |
| 2 | Change due | code 1, pay 50 | `Change: 35`: `1 x 20`, `1 x 10`, `1 x 5` | PASS |
| 3 | Large payment | code 1, pay 100 | `Change: 85`: `1 x 50`, `1 x 20`, `1 x 10`, `1 x 5` | PASS |
| 4 | Maximum payment | code 6, pay 999 | `Change: 964`: `9 x 100`, `1 x 50`, `1 x 10`, `4 x 1` | PASS |
| 5 | Multiple purchases | two sales, then 0 | both correct, then exit | PASS |

### 2.2 Payment flow

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 6 | Insufficient funds | code 5, pay 10 | `Short by: 20`, then the payment prompt again | PASS |
| 7 | Retry, overpay | code 5, pay 10, then 25 | total 35 against 30: `Change: 5`, `1 x 5` | PASS |
| 8 | Retry, exact | code 5, pay 10, then 20 | total 30: `Change: 0`, no breakdown lines | PASS |
| 9 | Several retries | code 6, pay 10, then 10, then 15 | `Short by: 25`, `Short by: 15`, then `Change: 0` | PASS |
| 10 | Zero payment | code 1, pay 0 | `Transaction cancelled.`, no refund line, back to menu | PASS |
| 11 | Cancel with refund | code 5, pay 10, then 0 | `Transaction cancelled. Refunded: 10`, back to menu | PASS |
| 12 | Bad input during payment | code 3, then `abc`, `70000`, `0` | `Invalid number` twice, then `Transaction cancelled.` | PASS |

### 2.3 Input checks

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 13 | Invalid code | code 9 | `Invalid product code. Try again.`, re-prompt | PASS |
| 14 | Non-numeric code | `abc` | `Invalid number. Try again.`, re-prompt | PASS |
| 15 | Oversized payment | pay 70000 | `Invalid number. Try again.`, payment re-prompt | PASS |
| 16 | Empty code | Enter alone at the code prompt | `Invalid number. Try again.`; the program does **not** exit | PASS |
| 17 | Trailing junk in payment | `12x` at the payment prompt | `Invalid number. Try again.`, payment re-prompt | PASS |

### 2.4 Exit, receipt and screen handling

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 18 | Exit | code 0 | `Thank you. Goodbye.`; return to DOS | PASS |
| 19 | Startup | run `vending.exe` | the screen clears; the menu appears at the top | PASS |
| 20 | Sale with receipt and pause | code 1, pay 50 | Chips receipt with `Change: 35`, `1 x 20`, `1 x 10`, `1 x 5`; `Press any key to continue...`; after a key the screen clears and the menu returns | PASS |
| 21 | Exact payment receipt | code 2, pay 25 | the receipt shows `Change: 0` | PASS |
| 22 | Cancel with pause | code 5, pay 10, then 0 | `Transaction cancelled. Refunded: 10`; pause; cleared menu | PASS |
| 23 | No clear mid-transaction | code 5, pay 10, then 25 | `Short by: 20` stays on screen; receipt shows `Paid:   35`, `Change: 5`, `1 x 5` | PASS |
| 24 | No clear on invalid input | code 9, then `abc` | error messages are shown; the screen is not cleared | PASS |
| 25 | Exit with screen clear in place | code 0 after the menu has been redrawn | `Thank you. Goodbye.`; clean return to DOS | PASS |
| 26 | Earlier scenarios re-run | checks 1 to 5 and 13 to 15 after the payment changes | same results as before | PASS |

### 2.5 Extended features (`vending.exe`)

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 27 | Sold out | code 2, pay 25, three times; then look at the menu; then choose 2 | the menu shows `2. Soda - 25 (sold out)`; choosing 2 gives `Sorry, that product is sold out.` and asks again | PASS |
| 28 | Exit summary | code 1 pay 50, code 2 pay 25, then 0 | `Sales made: 2`, `Total taken: 40`, `Thank you. Goodbye.` | PASS |
| 29 | No exact change | admin choice 5: set the 100-coins to 0 (and the 50-coins to 0); then code 6, pay 999 | `Sorry, no exact change available.` and `Refunded: 999`; no receipt; the stock of product 6 is unchanged | PASS |
| 30 | Change coins leave the stock | code 1, pay 50, then admin view | coins `100: 10`, `50: 11`, `20: 9`, `10: 9`, `5: 9`, `1: 10` | PASS |
| 31 | Inserted money joins the coins | code 2, pay 25, then admin view | the 20-coins and the 5-coins are each one higher than before the sale (25 is stored as 20 + 5) | PASS |
| 32 | Wrong PIN | `31415`, then `9999` | `Wrong PIN. Tries left: 2` and the PIN prompt again, not the admin code | PASS |
| 33 | Admin menu | `31415`, `1234`, choice 1, then 0 | the PIN shows as `*`; the admin menu appears with choices 1 to 7; choice 1 shows stock, coins and sales; 0 returns to the product menu | PASS |
| 34 | Admin refill and set | sell out a product, then admin choice 2; admin choice 4 with `2`, `7` | the product can be bought again; product 2 shows 7 | PASS |
| 35 | Save file | code 1 pay 15, exit, run again, admin view | Chips stock 2; the sales count and total are kept | PASS |
| 36 | Reset | delete `VENDING.DAT`, run, admin view | every product 3, every coin 10, sales 0 0, admin code and PIN back to the defaults | PASS |

### 2.6 Phase 3 (`vending.exe`)

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 37 | Backspace at payment | code 1, then `5`, Backspace, `5`, `0`, Enter | the first 5 is erased; paid 50; `Change: 35` | PASS |
| 38 | Aligned menu | start the program | names, dashes and prices line up in columns | PASS |
| 39 | Hidden PIN | `31415`, then `1234` | the admin code is shown as typed; the PIN shows as `*`; the admin menu opens | PASS |
| 40 | Go back from the PIN | `31415`, then `0` | back to the product menu; no try is used | PASS |
| 41 | Wrong PIN re-asks the PIN | `31415`, then `9999` | `Wrong PIN. Tries left: 2` and the PIN prompt again | PASS |
| 42 | Lockout | three wrong PINs | `Too many wrong PINs. Admin is locked for 1 minute(s).`; back to the menu | PASS |
| 43 | Locked countdown | `31415` while locked | `Admin is locked. Try again in 0 min NN sec.`; after a key, the menu | PASS |
| 44 | Lock ends | `31415` after the lock time has passed | the PIN prompt is asked again | PASS |
| 45 | Longer second lock | three more wrong PINs after the first lock ended | `locked for 5 minute(s)` | PASS |
| 46 | Right PIN resets the level | a right PIN, then three wrong PINs | `locked for 1 minute(s)` again | PASS |
| 47 | Restart clears the lock | get locked, exit with 0, run again, `31415` | the PIN prompt is asked straight away | PASS |
| 48 | Change the admin code | admin choice 6: `12345` twice | `Done.`; `31415` is now an invalid product code; `12345` opens admin mode, also after a restart | PASS |
| 49 | Change the PIN | admin choice 7: `4321` twice (shown as `*`) | `Done.`; the old PIN is rejected; `4321` works, also after a restart | PASS |
| 50 | Change refused | codes and PINs that are too small, or two entries that differ | `The admin code must be at least 10.`, `The PIN must be 1000 to 9999.`, `The two entries do not match. Nothing changed.`; nothing changes | PASS |
| 51 | Sales kept across runs | sale of code 1 with pay 15, exit, run again, admin view | the sales count and total include the earlier sale | PASS |

### 2.7 Phase 4 (`vending.exe`, keyboard version)

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 52 | The keyboard version is unchanged | run `vending.exe` without `/g` and repeat checks 2, 3 and 4 of section 2.1 | the same menu and receipts as before, with the 100, 50, 20, 10, 5, 1 coins | PASS |
| 53 | Six coins in the admin view | admin choice 1 | the coin list shows 100, 50, 20, 10, 5 and 1 | PASS |
| 54 | Set coins with a new value | admin choice 5: `20`, then `4`; then `25` | `20` is accepted; `25` is not a coin of the machine | PASS |

---

### 2.8 The Python window (`gui\vending_gui.py`, tried by hand)

| No. | Scenario | Action | Expected | Result |
|---|---|---|---|---|
| G1 | Start | `py gui\vending_gui.py` | DOSBox starts and minimizes by itself; the status bar says *Connected to vending.exe*; 6 product cards with code, name, price and stock | PASS |
| G2 | Sale with change | click a product, insert 10, then 5 | `Inserted 10. Insert 5 more.`, then a receipt with the change and its coins, and the stock goes down by one | PASS |
| G3 | Large payment | click Chips (15), press 100 | change 85 shown as 1 x 50, 1 x 20, 1 x 10, 1 x 5 | PASS |
| G4 | Money buttons | no product chosen | all money buttons are disabled; they enable when a product is chosen | PASS |
| G5 | Cancel | choose a product, insert something, press Cancel | `Purchase cancelled. Refunded n.` | PASS |
| G6 | Sold out | buy the same product until it runs out | its card is grayed out, says SOLD OUT, and cannot be chosen | PASS |
| G7 | No exact change | admin sets the small coins to 0, then a customer overpays | `Sorry, exact change is not available. Refunded n.`; nothing is sold | PASS |
| G8 | Admin login | Admin tab | asks for the admin code and the PIN, both shown as `*`; a wrong code or PIN shows the tries left | PASS |
| G9 | Lockout | three wrong logins | a live countdown replaces the message and Log in is disabled; no empty gap is shown before that | PASS |
| G10 | Admin tables | log in, use Set and Refill | six coin rows and six product rows update at once | PASS |
| G11 | Change code and PIN | type two different values, then two equal values | the message names the entry that did not match; the equal values are accepted and work at the next login | PASS |
| G12 | Open Admin during a purchase | choose a product, insert money, click the Admin tab | the window stays on Customer and the money is refunded | PASS |
| G13 | Leave the Admin tab | type a code and PIN without logging in, go to Customer and back | the Admin boxes are empty; a logged-in admin is logged out | PASS |
| G14 | Clean screen | open the Admin tab after a sale | the black display and the receipt are empty; the display says `Choose a product.` again on return | PASS |
| G15 | Close the window | close it | `vending.exe` and DOSBox both end; no DOSBox is left in Task Manager | PASS |
| G16 | Manual mode and DOSBox-X | `--manual`; then `DOSBOX_PATH` set to `dosbox-x.exe` | both work (DOSBox-X starts and minimizes) | PASS |
| G17 | Settings | change `PAY_BUTTONS` in `gui\settings.py`, restart | the buttons change | PASS |

---

## 3. Design checks (build-time, NEW)

These confirm that the product data has a single source of truth.

| No. | Check | Action | Expected | Result |
|---|---|---|---|---|
| 1 | One place for product data | change a price in `product_prices`, rebuild `vending` | the menu line, the `Price:` line and the receipt all show the new price | PASS |
| 2 | Table-length guard | remove one name from `product_names`, rebuild | the assembler stops with a forced error; no `.exe` is produced | PASS |
| 3 | Adding a product | add one price, one name string and one pointer, rebuild | a seventh menu line appears, code 7 is accepted, and the receipt prints the new name | PASS |

---

## 4. Summary

| Group | Checks | Passed | Failed |
|---|---|---|---|
| Module-level checks (section 1, checks 1 to 80) | 80 | 79 | 0 |
| Bridge checks (section 1.8, `gui\test_bridge.py`) | 62 | 62 | 0 |
| End-to-end checks (section 2, checks 1 to 54) | 54 | 54 | 0 |
| The Python window (section 2.8, checks G1 to G17) | 17 | 17 | 0 |
| Design checks (section 3) | 3 | 3 | 0 |
| **Total** | **216** | **215** | **0** |

The remaining check is marked Superseded (module check 16, replaced by check 40).

---

## 5. Feature status

| Feature | Status |
|---|---|
| Product menu, printed from the tables | Done |
| Product code validation (0 exits) | Done |
| Price lookup | Done |
| Payment with a running total over several insertions | Done |
| Cancel with refund | Done |
| Sufficiency check with shortfall message | Done |
| Change calculation (compare before subtracting) | Done |
| Change breakdown into 100, 50, 20, 10, 5, 1 | Done |
| Receipt with product name | Done |
| Screen clear and key-press pause | Done |
| Input validation (letters, empty input, over 999) | Done |
| Repeated purchases and clean exit | Done |
| Build script and a test program per module | Done |
| Stock levels and sold-out products | Done |
| Coin inventory: change taken from the coins in stock, inserted money added | Done |
| Refusal with refund when exact change is not possible | Done |
| Sales counter and total, shown at exit | Done |
| Admin mode (secret code and PIN): view, refill, set stock and coins | Done |
| File persistence of stock and coins | Done |
| Backspace in number entry | Done |
| Aligned customer menu and admin product list | Done |
| Sales totals saved between runs | Done |
| Admin PIN hidden, `0` to go back, wrong PIN re-asks the PIN | Done |
| Admin lockout (1, 5, 10, 20, 30, then 60 minutes) | Done |
| Change the admin code and the PIN (both saved) | Done |
| Bridge mode (`vending.exe /g`) with the `CMD.TXT` / `OUT.TXT` protocol | Done |
| Python window: customer and admin tabs, settings file, automatic DOSBox start | Done |
| Exact-change search | Planned extension |

---

## 6. Design decisions and known behaviour

### 6.1 Design decisions

1. **Assembler:** JWasm is used instead of Visual Studio's `ml.exe`, because the `ml.exe` in Visual Studio 2022 targets 32-bit Windows and cannot produce 16-bit DOS executables.
2. **Module files:** the code is split into `include` files assembled into one program, so there is one build command and no separate linking step.
3. **`read_int`:** empty input is invalid; keys are kept in a line buffer (`rbuf`, 16 characters) and checked when Enter is pressed, so Backspace can erase any key; the limit is checked after every digit; `echo_mask` hides the keys for the PIN.
4. **Payment flow:** insufficient funds returns to the payment prompt with a running total, and `read_payment` returns 0 as the cancel signal, which refunds what was inserted.
5. **Lookups:** the range check and index calculation live in one shared routine, `code_to_offset`, in their own module.
6. **Menu:** printed from `product_names`, `product_prices` and `product_stock`, so product data is stored in one place only. `print_name_col` and `print_int3` (in `utils.inc`) align the columns in both the customer menu and the admin product list.
7. **Constants and sizes:** `CR`, `LF`, `MAX_INPUT`, `TEXT_ATTR` and `SCREEN_END` are named; `NUM_PRODUCTS` and `NUM_DENOMS` are computed by the assembler; a build-time check keeps the name and price tables the same length.
8. **`break_change`:** saves and restores AX, so callers do not lose their value.
9. **Stock and coins:** one table of units per product (`product_stock`) and one of coins per denomination (`coin_stock`); `break_change` only calculates, and `take_coins`, `take_stock` and `put_coins` change the stock after a sale succeeds, so a refused sale leaves the machine unchanged.
10. **Payment prompts:** `read_payment` was replaced by the shared `ask_int` (prompt and limit passed in), also used by the admin prompts.
11. **Admin mode:** entered by typing the admin code (`admin_code`) at the product prompt, then the PIN (`admin_pin`), which is shown as `*`. `0` goes back, and a wrong PIN re-asks only the PIN. After `PIN_TRIES` (3) wrong PINs the admin is locked for a time that grows (`lock_times`: 1, 5, 10, 20, 30, 60 minutes), measured with the DOS clock; the lock is kept in memory only.
12. **Save file:** product stock, coin stock, sales count and total, admin code and PIN are saved (`VENDING.DAT`) every time the menu is shown. A file of the wrong size or for different table sizes is ignored.
13. **Changing the code and PIN:** the new value is typed twice (the PIN as `*`); the admin code must be at least `ADMIN_CODE_MIN` (10) so it never clashes with a product code, and the PIN must be `PIN_MIN` to `PIN_MAX` (1000 to 9999).
14. **Coins:** the machine's coins are the `denoms` table (100, 50, 20, 10, 5, 1); every routine reads its size from it, so changing the table changes the whole program. The window's money buttons are separate (`PAY_BUTTONS`), and inserted money is stored as the machine's coins.
15. **Bridge:** two fixed-size files overwritten in place (64-byte commands, 1024-byte replies) with sequence numbers; a reply is wiped to spaces first and its `#` is written last, so a reply being written is never read as finished.
16. **Program output as the reply:** in bridge mode the console handle is redirected into `OUT.TXT`, so the existing print routines write the replies.
17. **Window:** it only sends commands and draws replies; all rules stay in assembly. The admin login needs both the admin code and the PIN. Opening the Admin tab during a purchase cancels it; leaving the tab logs out and empties its boxes.
18. **File names:** test programs are named `t_*.asm`, because DOS allows at most eight characters before the extension.

### 6.2 Known behaviour (not bugs)

- After invalid input at a prompt, only that prompt repeats. The menu is not redrawn until the transaction ends.
- Each single insertion is limited to 999.
- The running total cannot overflow 16 bits: a retry only happens while the total is below the price, and each insertion is at most 999.
- `clear_screen` assumes an 80x25 text screen (the DOS default in DOSBox).
- Change is made largest coin first from the coins in stock. If that fails, the sale is refused even when another combination of coins could work.
- A number prompt holds at most 16 typed characters; extra keys are ignored.
- The sales total is a 16-bit number and wraps after 65535.
- The admin code and PIN are stored as plain numbers in `VENDING.DAT`.
- The admin code is shown as typed at the product prompt; only the PIN is hidden.
- The admin lock is kept in memory only, so restarting the program clears it.
- Bridge mode serves one window at a time, and every reply holds at most 1024 bytes.
- The bridge login sends the admin code and PIN as plain text in `CMD.TXT`.
- The window's automatic DOSBox start and minimizing are written for Windows.
- The product list is fixed when the program is assembled.
- The machine saves its state, so stock and coins carry over between runs until `VENDING.DAT` is deleted.

---

## 7. Evidence to add before submission

- [ ] Screenshots: menu, receipt after a sale, insufficient funds with retry, cancel with refund, invalid input, exit (kept with the report in `docs\`, which is outside the repository).
- [ ] Tester name and date.
