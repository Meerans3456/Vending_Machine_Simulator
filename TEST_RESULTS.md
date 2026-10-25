# Vending Machine Simulator: Test Results

**Version tested:** the current program, after the code refinement round (all features complete).
**Environment:** Windows, JWasm v2.11 (`-0 -mz`), run in DOSBox.
**How results were obtained:** each check was run by the team in DOSBox and compared with the expected output. "PASS" means the observed output matched the expected output.

Rebuild any program with `.\build.bat <name>`, then run `<name>.exe` in DOSBox.

---

## How to run the tests

1. **Build every program** (PowerShell, from the project folder):

   ```powershell
   'vending','t_utils','t_lookup','t_input','t_math','t_output' | ForEach-Object { .\build.bat $_ }
   ```

2. **Start DOSBox and mount the project folder** (see [Setup.md](Setup.md)). If DOSBox was already open, press **Ctrl+F4** first so it sees the new files.
3. **Run each program** and compare its output with the tables below.

| Program | Interactive? | What to do |
|---|---|---|
| `t_utils.exe` | one key press | Read the printed lines (section 1.1). When asked, press a key: the screen must clear and the next text start at the top-left. |
| `t_lookup.exe` | no | It prints three blocks (price, name, offset) for the codes 0, 1, 3, 6, 7, 9. Compare with section 1.3. |
| `t_input.exe` | yes | **First phase** (`read_int>` prompt): type `42`, `abc`, an empty line (just Enter), `999`, `1000`, `70000`, `12x`, and finally `0` to end the phase. **Second phase** (`read_product`): type `9`, `abc`, then `4`. **Third phase** (`read_payment`): type `abc`, then `0`. Compare with section 1.2. |
| `t_math.exe` | no | It prints the menu, then the `calc_change` and `break_change` results (the menu may scroll off the top; scroll up or enlarge the window). Compare with section 1.4. |
| `t_output.exe` | no | It prints five change breakdowns, then three receipts. Compare with section 1.5. |
| `vending.exe` | yes | Follow the scenarios in section 2 (for example code `1`, pay `50`). |

Section 3 (design checks) is done by editing `inc\data.inc`, rebuilding `vending`, and undoing the edit afterwards.

**Change history**
- *First version:* core purchase flow, with a test program per module.
- *Payment, receipt and screen features:* running total with retry, cancel with refund, receipt, screen clear and pause. Earlier checks were re-run afterwards.
- *Code refinement:* the menu is printed from the tables, lookups moved into their own module, unused data removed, `break_change` preserves AX, named constants and computed table sizes. All earlier checks were re-run; the checks marked **NEW** were added.

---

## 1. Module-level checks

Each module has its own standalone test program, so a failure points to one module.

| Test program | Covers | Source module |
|---|---|---|
| `t_utils.exe` | `print_str`, `print_char`, `print_int`, `clear_screen`, `wait_key` | `utils.inc` |
| `t_input.exe` | `read_int` (interactive), `read_product`, `read_payment` | `input.inc` |
| `t_lookup.exe` | `code_to_offset`, `lookup_price`, `lookup_name` | `lookup.inc` |
| `t_math.exe` | `display_menu`, `calc_change`, `break_change` | `menu.inc`, `math.inc` |
| `t_output.exe` | `print_change`, `print_receipt` | `output.inc` |

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
| 16 | `read_payment` | `abc`, then `0` (or `50`) | `abc` is rejected; then AX = 0 (cancel) or AX = 50 | PASS |

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
| 29 | `break_change` | AX = 964 | counts 19, 0, 1, 0, 4 | PASS |
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

---

## 2. End-to-end checks (`vending.exe`)

### 2.1 Sales

| No. | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| 1 | Exact payment | code 2, pay 25 | receipt with `Change: 0`, no breakdown lines | PASS |
| 2 | Change due | code 1, pay 50 | `Change: 35`: `1 x 25`, `1 x 10` | PASS |
| 3 | Large payment | code 1, pay 100 | `Change: 85`: `1 x 50`, `1 x 25`, `1 x 10` | PASS |
| 4 | Maximum payment | code 6, pay 999 | `Change: 964`: `19 x 50`, `1 x 10`, `4 x 1` | PASS |
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
| Module-level checks (section 1) | 38 | 38 | 0 |
| End-to-end checks (section 2) | 26 | 26 | 0 |
| Design checks (section 3) | 3 | 3 | 0 |
| **Total** | **67** | **67** | **0** |

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
| Stock levels, coin inventory, admin mode, sales counter, colored text, file persistence, graphics mode | Planned extensions |

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
9. **File names:** test programs are named `t_*.asm`, because DOS allows at most eight characters before the extension.

### 6.2 Known behaviour (not bugs)

- After invalid input at a prompt, only that prompt repeats. The menu is not redrawn until the transaction ends.
- Backspace is treated as a non-digit, so it makes the entry invalid rather than deleting a character.
- Each single insertion is limited to 999.
- The running total cannot overflow 16 bits: a retry only happens while the total is below the price, and each insertion is at most 999.
- `clear_screen` assumes an 80x25 text screen (the DOS default in DOSBox).

---

## 7. Evidence to add before submission

- [ ] Screenshots: menu, receipt after a sale, insufficient funds with retry, cancel with refund, invalid input, exit (kept with the report in `docs\`, which is outside the repository).
- [ ] Tester name and date.
