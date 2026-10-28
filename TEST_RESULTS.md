# Vending Machine Simulator: Test Results

**Version tested:** the current program, with the extended features (stock levels, coin inventory, admin mode, save file).
**Environment:** Windows, JWasm v2.11 (`-0 -mz`), run in DOSBox.
**How results were obtained:** each check was run by the team in DOSBox and compared with the expected output. "PASS" means the observed output matched the expected output. The checks for the extended features (sections 1.6 and 2.5) were run in the same way and all passed. Checks marked "Superseded" no longer apply because the coin inventory changed the behaviour; they are replaced by the checks named in the table.

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
| `t_utils.exe` | one key press | Read the printed lines (section 1.1). When asked, press a key: the screen must clear and the next text start at the top-left. |
| `t_lookup.exe` | no | It prints three blocks (price, name, offset) for the codes 0, 1, 3, 6, 7, 9. Compare with section 1.3. |
| `t_input.exe` | yes | **First phase** (`read_int>` prompt, limit 999): type `42`, `abc`, an empty line (just Enter), `999`, `1000`, `70000`, `12x`, and finally `0` to end the phase. **Second phase** (limit 65535): type `70000`, then `0`. **Third phase** (`read_product`, product 3 is sold out): type `9`, `abc`, `3`, then `4`. **Fourth phase** (`ask_int`): type `abc`, then `25`. Compare with sections 1.2 and 1.6. |
| `t_stock.exe` | no | It prints four blocks (`lookup_stock`, `take_stock`, `take_coins`, `put_coins`). Compare with section 1.6. |
| `t_math.exe` | no | It prints the menu (product 3 sold out), then the `calc_change` and `break_change` results, including a block with limited coins (the menu may scroll off the top; scroll up or enlarge the window). Compare with sections 1.4 and 1.6. |
| `t_output.exe` | no | It prints five change breakdowns, three receipts, then two sales summaries. Compare with sections 1.5 and 1.6. |
| `t_admin.exe` | yes | Parts 1 to 4 are automatic (press a key after each screen). Part 5: type `2`, then `7`. Part 6: type `25`, then `4`. Part 7: type any PIN. Compare with section 1.6. |
| `t_file.exe` | no | It prints three blocks using its own `T_STATE.DAT`, so your real `VENDING.DAT` is not touched. Compare with section 1.6. |
| `vending.exe` | yes | Follow the scenarios in section 2 (for example code `1`, pay `50`). |

Section 2.5 needs a known starting state: delete `VENDING.DAT` before the first scenario so the machine starts full (3 of each product, 10 of each coin), and use the admin refill options between scenarios when needed.

Section 3 (design checks) is done by editing `inc\data.inc`, rebuilding `vending`, and undoing the edit afterwards (delete `VENDING.DAT` after changing the number of products).

**Change history**
- *First version:* core purchase flow, with a test program per module.
- *Payment, receipt and screen features:* running total with retry, cancel with refund, receipt, screen clear and pause. Earlier checks were re-run afterwards.
- *Code refinement:* the menu is printed from the tables, lookups moved into their own module, unused data removed, `break_change` preserves AX, named constants and computed table sizes. All earlier checks were re-run; the checks marked **NEW** were added.
- *Extended features:* stock levels with sold-out products, coin inventory (change taken from the coins in stock, inserted money added to it), exact-change refusal with refund, sales summary, PIN-protected admin mode, and a save file. `read_payment` was replaced by the shared `ask_int`. The checks in sections 1.6 and 2.5 were added and passed, and three earlier checks were superseded.

---

## 1. Module-level checks

Each module has its own standalone test program, so a failure points to one module.

| Test program | Covers | Source module |
|---|---|---|
| `t_utils.exe` | `print_str`, `print_char`, `print_int`, `clear_screen`, `wait_key` | `utils.inc` |
| `t_input.exe` | `read_int` (interactive), `read_product`, `ask_int` | `input.inc` |
| `t_lookup.exe` | `code_to_offset`, `lookup_price`, `lookup_name` | `lookup.inc` |
| `t_math.exe` | `display_menu`, `calc_change`, `break_change` | `menu.inc`, `math.inc` |
| `t_output.exe` | `print_change`, `print_receipt`, `print_summary` | `output.inc` |
| `t_stock.exe` | `lookup_stock`, `take_stock`, `take_coins`, `put_coins` | `stock.inc` |
| `t_admin.exe` | `admin_view`, `refill_products`, `refill_coins`, `set_product_stock`, `set_coin_count`, `ask_pin` | `admin.inc` |
| `t_file.exe` | `save_state`, `load_state` | `file.inc` |

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
| 25 | `break_change` | AX = 35 | counts 0, 1, 1, 0, 0 | PASS |
| 26 | `break_change` | AX = 0 | all counts 0 | PASS |
| 27 | `break_change` | AX = 100 | counts 2, 0, 0, 0, 0 | PASS |
| 28 | `break_change` | AX = 99 | counts 1, 1, 2, 0, 4 | PASS |
| 29 | `break_change` | AX = 964 | the machine holds only 10 of each coin; see check 47 | Superseded |
| 30 | `break_change` | each amount above | AX has the same value after the call | PASS (**NEW**) |

The menu printed at the top of `t_math.exe` is also checked by eye; see end-to-end check 19.

### 1.5 `t_output` (output.inc)

| No. | Routine | Input | Expected | Result |
|---|---|---|---|---|
| 31 | `print_change` | change 35, counts 0, 1, 1, 0, 0 | `Change: 35`, `1 x 25`, `1 x 10` | PASS |
| 32 | `print_change` | change 0, all counts 0 | `Change: 0`, no breakdown lines | PASS |
| 33 | `print_change` | change 99, counts 1, 1, 2, 0, 4 | `Change: 99`, `1 x 50`, `1 x 25`, `2 x 10`, `4 x 1` | PASS |
| 34 | `print_change` | change 964, counts 19, 0, 1, 0, 4 | `Change: 964`, `19 x 50`, `1 x 10`, `4 x 1` | PASS |
| 35 | `print_change` | change 1, counts 0, 0, 0, 0, 1 | `Change: 1`, `1 x 1` | PASS |
| 36 | `print_receipt` | Chips, price 15, paid 50, change 35 | banner, `Item:   Chips`, `Price:  15`, `Paid:   50`, `Change: 35`, `1 x 25`, `1 x 10`, footer | PASS |
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
| 44 | `take_coins` | coins 10 10 10 10 10; counts 1 0 2 0 3 taken twice | `9 10 8 10 7`, then `8 10 6 10 4` | PASS |
| 45 | `put_coins` | AX = 35, then 64 | `8 11 7 10 4`, then `9 11 8 10 8` | PASS |
| 46 | `break_change` (limited coins) | two 1-coins left: AX = 2, then 3; no 25-coins: AX = 30, then 35 | `0 0 0 0 2`; no exact change; `0 0 3 0 0`; `0 0 3 1 0` | PASS |
| 47 | `break_change` | AX = 964 with 10 of each coin | no exact change | PASS |
| 48 | `display_menu` | product 3 sold out | the Candy line ends with `(sold out)`; other lines normal | PASS |
| 49 | `print_summary` | 3 sales / total 70; then 0 / 0 | shows 3 and 70; then 0 and 0 | PASS |
| 50 | `admin_view` | starting state | every product 3, every coin 10, sales 0 | PASS |
| 51 | `admin_view` | Chips 0, Soda 1, 50-coins 0, 25-coins 3 set by hand | the view shows those values | PASS |
| 52 | `refill_products` | after check 51 | every product back to 3; coin counts unchanged | PASS |
| 53 | `refill_coins` | after check 52 | every coin back to 10 | PASS |
| 54 | `set_product_stock` | `2`, then `7` | product 2 shows 7 | PASS |
| 55 | `set_coin_count` | `25`, then `4` | the 25-coins show 4 | PASS |
| 56 | `ask_pin` | any input | `PIN accepted` only for `1234`; anything else, including `abc`, gives `PIN rejected` | PASS |
| 57 | `load_state` | no save file | nothing changes: products 3 3 3 3 3 3, coins 10 10 10 10 10 | PASS |
| 58 | `save_state`, `load_state` | save `0 2 3 3 3 3 / 7 10 10 10 10`, scramble every value to 9, load | the saved values come back | PASS |
| 59 | `load_state` | a 3-byte file | the file is ignored; the values stay as they were (all 9) | PASS |

---

## 2. End-to-end checks (`vending.exe`)

### 2.1 Sales

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 1 | Exact payment | code 2, pay 25 | receipt with `Change: 0`, no breakdown lines | PASS |
| 2 | Change due | code 1, pay 50 | `Change: 35`: `1 x 25`, `1 x 10` | PASS |
| 3 | Large payment | code 1, pay 100 | `Change: 85`: `1 x 50`, `1 x 25`, `1 x 10` | PASS |
| 4 | Maximum payment | code 6, pay 999 | change 964 needs more coins than the machine holds; see check 29 | Superseded |
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
| 20 | Sale with receipt and pause | code 1, pay 50 | Chips receipt with `Change: 35`, `1 x 25`, `1 x 10`; `Press any key to continue...`; after a key the screen clears and the menu returns | PASS |
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
| 29 | No exact change | code 6, pay 999 | `Sorry, no exact change available.` and `Refunded: 999`; no receipt; the stock of product 6 is unchanged | PASS |
| 30 | Change coins leave the stock | code 1, pay 50, then admin view | coins `50: 11`, `25: 9`, `10: 9`, `5: 10`, `1: 10` | PASS |
| 31 | Inserted money joins the coins | code 2, pay 25, then admin view | 25-coins 11 | PASS |
| 32 | Wrong PIN | `31415`, then `0000` | `Wrong PIN.`; after a key the menu returns | PASS |
| 33 | Admin menu | `31415`, `1234`, choice 1, then 0 | the admin menu appears; choice 1 shows stock, coins and sales; 0 returns to the product menu | PASS |
| 34 | Admin refill and set | sell out a product, then admin choice 2; admin choice 4 with `2`, `7` | the product can be bought again; product 2 shows 7 | PASS |
| 35 | Save file | code 1 pay 15, exit, run again, admin view | Chips stock 2; the sales counter is back to 0 | PASS |
| 36 | Reset | delete `VENDING.DAT`, run, admin view | every product 3, every coin 10 | PASS |

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
| Module-level checks (section 1, checks 1 to 59) | 59 | 57 | 0 |
| End-to-end checks (section 2, checks 1 to 36) | 36 | 35 | 0 |
| Design checks (section 3) | 3 | 3 | 0 |
| **Total** | **98** | **95** | **0** |

The remaining 3 checks are marked Superseded (module checks 16 and 29, end-to-end check 4); they are replaced by checks 40, 47 and 29 of section 2.5.

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
| Change breakdown into 50, 25, 10, 5, 1 | Done |
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
| Exact-change search, colored text, graphics mode | Planned extensions |

---

## 6. Design decisions and known behaviour

### 6.1 Design decisions

1. **Assembler:** JWasm is used instead of Visual Studio's `ml.exe`, because the `ml.exe` in Visual Studio 2022 targets 32-bit Windows and cannot produce 16-bit DOS executables.
2. **Module files:** the code is split into `include` files assembled into one program, so there is one build command and no separate linking step.
3. **`read_int`:** empty input is invalid, the rest of the line is discarded after a bad character, and the limit is checked after every digit.
4. **Payment flow:** insufficient funds returns to the payment prompt with a running total, and `read_payment` returns 0 as the cancel signal, which refunds what was inserted.
5. **Lookups:** the range check and index calculation live in one shared routine, `code_to_offset`, in their own module.
6. **Menu:** printed from `product_names` and `product_prices`, so product data is stored in one place only. As a result the names are not column-aligned.
7. **Constants and sizes:** `CR`, `LF`, `MAX_INPUT`, `TEXT_ATTR` and `SCREEN_END` are named; `NUM_PRODUCTS` and `NUM_DENOMS` are computed by the assembler; a build-time check keeps the name and price tables the same length.
8. **`break_change`:** saves and restores AX, so callers do not lose their value.
9. **Stock and coins:** one table of units per product (`product_stock`) and one of coins per denomination (`coin_stock`); `break_change` only calculates, and `take_coins`, `take_stock` and `put_coins` change the stock after a sale succeeds, so a refused sale leaves the machine unchanged.
10. **Payment prompts:** `read_payment` was replaced by the shared `ask_int` (prompt and limit passed in), also used by the admin prompts.
11. **Admin mode:** entered by typing a secret number (`ADMIN_CODE`) at the product prompt, then a fixed PIN (`ADMIN_PIN`), with a single attempt.
12. **Save file:** only stock and coins are saved (`VENDING.DAT`), every time the menu is shown; sales totals restart each run. A file of the wrong size or for different table sizes is ignored.
13. **File names:** test programs are named `t_*.asm`, because DOS allows at most eight characters before the extension.

### 6.2 Known behaviour (not bugs)

- After invalid input at a prompt, only that prompt repeats. The menu is not redrawn until the transaction ends.
- Backspace is treated as a non-digit, so it makes the entry invalid rather than deleting a character.
- Each single insertion is limited to 999.
- The running total cannot overflow 16 bits: a retry only happens while the total is below the price, and each insertion is at most 999.
- `clear_screen` assumes an 80x25 text screen (the DOS default in DOSBox).
- Change is made largest coin first from the coins in stock. If that fails, the sale is refused even when another combination of coins could work.
- The machine saves its state, so stock and coins carry over between runs until `VENDING.DAT` is deleted.

---

## 7. Evidence to add before submission

- [ ] Screenshots: menu, receipt after a sale, insufficient funds with retry, cancel with refund, invalid input, exit (kept with the report in `docs\`, which is outside the repository).
- [ ] Tester name and date.
