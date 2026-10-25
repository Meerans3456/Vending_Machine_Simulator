# Vending Machine Simulator: Test Results

**Version tested:** MUST features and all SHOULD features (S1 to S7), including retry payment with a running total, cancel with refund, receipt output, and screen clear with a pause between transactions.
**Environment:** Windows, JWasm v2.11 (`-0 -mz`), run in DOSBox.
**How results were obtained:** each test was run by the team in DOSBox and compared with the expected output. "PASS" means the observed output matched the expected output.

Rebuild any program with `.\build.bat <name>`, then run `<name>.exe` in DOSBox.

**Change history**
- *Baseline (MUST only):* all MUST tests passed; E11 was not implemented.
- *After S1:* the payment prompt accepts `0` as cancel, and an insufficient payment returns to the payment prompt instead of the menu. This changed **E3**, **E10** and **A11**, and made **E11** testable. Rows below show the current expected behaviour.
- *After S3 and S6:* a successful sale prints a receipt (`print_receipt`) and pauses for a keypress; the screen is cleared before every menu (`clear_screen`); a cancel also pauses. Tests A19 to A23 and P1 to P7 were added.

---

## 1. Module-level tests

Each module has its own standalone test program, so a failure points to one module.

| Test program | Covers | Source module |
|---|---|---|
| `t_utils.exe` | `print_char`, `print_int`, `clear_screen`, `wait_key` | `utils.inc` |
| `t_input.exe` | `read_int`, `lookup_price`, `lookup_name`, `read_product`, `read_payment` | `input.inc` |
| `t_math.exe` | `display_menu`, `calc_change`, `break_change` | `menu.inc`, `math.inc` |
| `t_output.exe` | `print_change`, `print_receipt` | `output.inc` |

### 1.1 Blueprint unit tests (U1 to U13)

| ID | Module | Input | Expected | Result |
|---|---|---|---|---|
| U1 | `print_str` | offset of `"Hello$"` | prints `Hello` | PASS (exercised by every message in all four test programs) |
| U2 | `print_int` | AX = 0 | prints `0` | PASS |
| U3 | `print_int` | AX = 12345 | prints `12345` | PASS |
| U4 | `read_int` | `42` + Enter | AX = 42, CF = 0 | PASS |
| U5 | `read_int` | `abc` + Enter | CF = 1 | PASS |
| U6 | `lookup_price` | AL = 3 | AX = 10, CF = 0 | PASS |
| U7 | `lookup_price` | AL = 9 | CF = 1 | PASS |
| U8 | `calc_change` | AX = 50, BX = 25 | AX = 25, CF = 0 | PASS |
| U9 | `calc_change` | AX = 10, BX = 25 | AX = 15, CF = 1 | PASS |
| U10 | `break_change` | AX = 35 | counts 0, 1, 1, 0, 0 | PASS |
| U11 | `break_change` | AX = 0 | all counts 0 | PASS |
| U12 | `break_change` | AX = 100 | counts 2, 0, 0, 0, 0 | PASS |
| U13 | `break_change` | AX = 99 | counts 1, 1, 2, 0, 4 | PASS |

### 1.2 Additional module tests (added by the team)

| ID | Module | Input | Expected | Result |
|---|---|---|---|---|
| A1 | `print_char` | DL = `'A'` | prints `A` | PASS |
| A2 | `print_int` | AX = 7 / 10 / 65535 | prints `7` / `10` / `65535` | PASS |
| A3 | `read_int` | Enter alone (no digits) | CF = 1 | PASS |
| A4 | `read_int` | `999` | AX = 999, CF = 0 | PASS |
| A5 | `read_int` | `1000` | CF = 1 (over the 999 cap) | PASS |
| A6 | `read_int` | `70000` | CF = 1 | PASS |
| A7 | `read_int` | `12x` | CF = 1 | PASS |
| A8 | `read_int` | `abc`, then next prompt | rest of line is flushed; next prompt is clean | PASS |
| A9 | `lookup_price` | AL = 0 / 1 / 6 / 7 | CF = 1 / 15 / 35 / CF = 1 | PASS |
| A10 | `read_product` | `9`, `abc`, then `4` | invalid code, invalid number, AL = 4 | PASS |
| A11 | `read_payment` | `abc`, then `0` (or `50`) | `abc` rejected; then AX = 0 (cancel) or AX = 50 | PASS (**changed by S1**: `0` is now accepted as cancel, previously rejected) |
| A12 | `calc_change` | AX = 25, BX = 25 | AX = 0, CF = 0 (exact payment) | PASS |
| A13 | `break_change` | AX = 964 | counts 19, 0, 1, 0, 4 | PASS |
| A14 | `print_change` | change 35, counts 0, 1, 1, 0, 0 | `Change: 35`, `1 x 25`, `1 x 10` | PASS |
| A15 | `print_change` | change 0, all counts 0 | `Change: 0`, no breakdown lines | PASS |
| A16 | `print_change` | change 99, counts 1, 1, 2, 0, 4 | `Change: 99`, `1 x 50`, `1 x 25`, `2 x 10`, `4 x 1` | PASS |
| A17 | `print_change` | change 964, counts 19, 0, 1, 0, 4 | `Change: 964`, `19 x 50`, `1 x 10`, `4 x 1` | PASS |
| A18 | `print_change` | change 1, counts 0, 0, 0, 0, 1 | `Change: 1`, `1 x 1` | PASS |
| A19 | `lookup_name` | AL = 0 / 1 / 3 / 6 / 7 / 9 | not found / `Chips` / `Candy` / `Juice` / not found / not found | PASS |
| A20 | `clear_screen`, `wait_key` | run `t_utils.exe`, press a key at the prompt | prompt waits for a key; screen clears; next text starts at top-left | PASS |
| A21 | `print_receipt` | Chips, price 15, paid 50, change 35 | banner, `Item:   Chips`, `Price:  15`, `Paid:   50`, `Change: 35`, `1 x 25`, `1 x 10`, footer | PASS |
| A22 | `print_receipt` | Soda, price 25, paid 25, change 0 | receipt with `Change: 0` and no breakdown lines | PASS |
| A23 | `print_receipt` | Juice, price 35, paid 100, change 65 | receipt with `Paid:   100`, `Change: 65`, `1 x 50`, `1 x 10`, `1 x 5` (3-digit alignment) | PASS |

---

## 2. End-to-end tests (`vending.exe`)

### 2.1 Blueprint tests (E1 to E12)

| ID | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| E1 | Exact payment | code 2, pay 25 | `Change: 0`, no breakdown lines | PASS |
| E2 | Change due | code 1, pay 50 | `Change: 35`, `1 x 25`, `1 x 10` | PASS |
| E3 | Insufficient funds | code 5, pay 10 | `Insufficient funds. Short by: 20`, then the payment prompt again | PASS (**changed by S1**: previously returned to the menu) |
| E4 | Invalid code | code 9 | `Invalid product code. Try again.`, re-prompt | PASS |
| E5 | Non-numeric code | `abc` | `Invalid number. Try again.`, re-prompt | PASS |
| E6 | Exit | code 0 | `Thank you. Goodbye.`, return to DOS | PASS |
| E7 | Multiple purchases | two sales, then 0 | both correct, then exit | PASS |
| E8 | Large payment | code 1, pay 100 | `Change: 85`, `1 x 50`, `1 x 25`, `1 x 10` | PASS |
| E9 | Maximum payment | code 6, pay 999 | `Change: 964`, `19 x 50`, `1 x 10`, `4 x 1` | PASS |
| E10 | Zero payment | code 1, pay 0 | `Transaction cancelled.`, no refund line, back to menu | PASS (**changed by S1**: previously rejected as invalid) |
| E11 | Cancel | code 5, pay 10, then 0 | `Transaction cancelled. Refunded: 10`, back to menu | PASS |
| E12 | Input overflow | pay 70000 | `Invalid number. Try again.`, payment re-prompt | PASS |

### 2.2 Tests added with S1 (retry and cancel)

| ID | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| E3b | Retry, overpay | code 5, pay 10, then insert 25 | total 35 vs price 30: `Change: 5`, `1 x 5` | PASS |
| E3c | Retry, exact | code 5, pay 10, then insert 20 | total 30 vs price 30: `Change: 0`, no breakdown lines | PASS |
| E13 | Multiple retries | code 6, pay 10, then 10, then 15 | `Short by: 25`, `Short by: 15`, then `Change: 0` | PASS |
| E14 | Bad input during payment | code 3, then `abc`, `70000`, `0` | `Invalid number` twice, then `Transaction cancelled.` | PASS |
| R1 | Regression | E1, E2, E4 to E9, E12 re-run after S1 | same results as before, apart from the reworded prompt | PASS |

### 2.3 Extra input checks

| ID | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| X1 | Empty code | Enter alone at the code prompt | `Invalid number. Try again.`; program does **not** exit | PASS |
| X2 | Trailing junk in payment | `12x` at the payment prompt | `Invalid number. Try again.`, payment re-prompt | PASS |

### 2.4 Tests added with S3 and S6 (receipt and screen clear)

| ID | Scenario | Input | Expected | Result |
|---|---|---|---|---|
| P1 | Startup | run `vending.exe` | screen clears; menu appears at the top | PASS |
| P2 | Sale with receipt and pause | code 1, pay 50 (E2) | Chips receipt with `Change: 35`, `1 x 25`, `1 x 10`; `Press any key to continue...`; after a key the screen clears and the menu returns | PASS |
| P3 | Exact payment receipt | code 2, pay 25 (E1) | receipt shows `Change: 0`, no breakdown lines | PASS |
| P4 | Cancel with pause | code 5, pay 10, then 0 (E11) | `Transaction cancelled. Refunded: 10`; pause; cleared menu | PASS |
| P5 | No clear mid-transaction | code 5, pay 10, then 25 (E3b) | `Short by: 20` stays on screen; receipt shows `Paid:   35`, `Change: 5`, `1 x 5` | PASS |
| P6 | No clear on invalid input | code 9, then `abc` | error messages shown; screen is not cleared | PASS |
| P7 | Exit | code 0 | `Thank you. Goodbye.`; clean return to DOS | PASS |

---

## 3. Summary

| Group | Tests | Passed | Failed / pending |
|---|---|---|---|
| Blueprint unit tests (U1 to U13) | 13 | 13 | 0 |
| Additional module tests (A1 to A23) | 23 | 23 | 0 |
| Blueprint end-to-end tests (E1 to E12) | 12 | 12 | 0 |
| S1 end-to-end tests (E3b, E3c, E13, E14, R1) | 5 | 5 | 0 |
| Extra input checks (X1, X2) | 2 | 2 | 0 |
| S3 and S6 end-to-end tests (P1 to P7) | 7 | 7 | 0 |
| **Total** | **62** | **62** | **0** |

**Acceptance (from the blueprint):** all MUST tests pass, and at least 90% of SHOULD tests pass. Status: **met** (all MUST and all SHOULD tests pass).

---

## 4. Feature status

### 4.1 MUST features

| ID | Feature | Status |
|---|---|---|
| M1 | Display product menu | Done |
| M2 | Read and validate product code (0 = exit, 1 to 6, else reject) | Done |
| M3 | Look up and display price | Done |
| M4 | Read payment (multi-digit input) | Done |
| M5 | Check payment >= price, show shortfall | Done |
| M6 | Calculate change with underflow guard | Done |
| M7 | Break change into 50 / 25 / 10 / 5 / 1 and display | Done |
| M8 | Loop back to menu after each sale | Done |
| M9 | Clean exit via `4Ch` | Done |
| M10 | Basic error messages | Done |
| M11 | Commented source and README | Source commented; README pending |
| M12 | Flowchart, test table, short report | Test table done; flowchart and report pending |
| M13 | Build works and `.exe` runs in DOSBox | Done (with JWasm; see 5.1) |

### 4.2 SHOULD features

| ID | Feature | Status |
|---|---|---|
| S1 | Cancel transaction (extended: retry payment with running total, refund on cancel) | Done |
| S2 | Reject non-numeric input explicitly | Done (`Invalid number. Try again.`) |
| S3 | Nicer receipt-style output | Done (`print_receipt`, product names table) |
| S4 | Clean "exact payment" case (`Change: 0`) | Done |
| S5 | Multi-digit amounts up to 999 | Done |
| S6 | Screen clear between transactions | Done (`clear_screen` via INT 10h, with `wait_key` pause) |
| S7 | Re-display menu after each transaction | Done |

All SHOULD features (S1 to S7) are complete.

---

## 5. Deviations from the blueprint and known behaviour

### 5.1 Deviations

1. **Assembler:** JWasm v2.11 is used instead of Visual Studio's `ml.exe`. The VS 2022 `ml.exe` targets 32-bit Windows and cannot produce 16-bit DOS executables. JWasm is MASM-compatible and produces DOS MZ executables.
2. **Multi-file layout:** the blueprint suggests a single file. We use `include` files (`data.inc`, `utils.inc`, `input.inc`, `menu.inc`, `math.inc`, `output.inc`) assembled into one program, so there is still one build command and no `PUBLIC`/`EXTERN` linking.
3. **`read_int` fixes** over the blueprint pseudocode: empty input is invalid, the rest of the line is flushed after a bad character, and the 999 cap is checked after every digit.
4. **`read_product`:** the blueprint's illegal `[product_codes + ax - 1]` instruction was removed. AL already holds the code after validation.
5. **`lookup_price`:** uses BX as the index register (saved and restored) instead of SI, which was not in its contract.
6. **`read_payment`:** accepts `0` (returned as AX = 0 to mean cancel) instead of rejecting it, to support S1.
7. **`main`:** after insufficient funds it returns to the payment prompt with a running total, instead of the blueprint's return to the menu. A cancel refunds the money inserted so far.
8. **Receipt and pause (S3, S6):** after a completed sale or a cancel, the program waits for a keypress before clearing the screen. Without the pause the receipt would be erased before it could be read. Product names live in a `product_names` table in `data.inc`.
9. **File names:** test programs are named `t_*.asm` because DOS allows at most 8 characters before the extension.

### 5.2 Known behaviour (not bugs)

- After invalid input at a prompt, only that prompt repeats. The menu is not redrawn until the transaction ends.
- Backspace is treated as a non-digit, so it makes the entry invalid rather than deleting a character.
- Each single insertion is capped at 999.
- `clear_screen` assumes an 80x25 text screen (the DOS default in DOSBox).
- The running total cannot overflow 16 bits: a retry only happens while the total is below the price (at most 35), and each insertion is at most 999.

---

## 6. Evidence to add before submission

- [ ] Screenshots: menu, receipt after a sale, insufficient funds with retry, cancel with refund, invalid input, exit.
- [ ] Tester name and date.
