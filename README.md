# Vending Machine Simulator (8086 Assembly)

A console vending machine written in 16-bit 8086 assembly (MASM syntax), assembled with **JWasm** and run in **DOSBox**.

The program shows a product menu, reads a product code, shows the price, accepts simulated payment (with retries and cancel), calculates change, breaks it into denominations, and prints a receipt. Several purchases can be made in one run.

---

## 1. Requirements

| Tool | Version / note |
|---|---|
| Windows | development machine |
| VS Code | editor only (no extension required) |
| JWasm | v2.11, MASM-compatible assembler; this project expects it at `tools\JWasm\JWasm.exe` |
| DOSBox | runs the 16-bit DOS `.exe` |
| DOSBox-X (optional) | has a built-in debugger; use it when stepping through code |

> **Why JWasm and not `ml.exe`?** The `ml.exe` that ships with Visual Studio 2022 targets 32-bit Windows and cannot produce 16-bit DOS programs. JWasm accepts the same syntax and produces DOS executables.

---

## 2. Project layout

```text
D:\8086VendingMachine\
|
|-- vending.exe, t_*.exe   built programs (output of build.bat)
|-- build.bat              assembles one program: build <name>
|-- README.md              this file
|-- TEST_RESULTS.md        recorded test results
|-- Setup.md               development environment notes
|
|-- src\
|   `-- vending.asm        main program: includes all modules, contains main
|
|-- inc\                   modules, pasted into the program at assembly time
|   |-- data.inc           tables, messages, variables (included first)
|   |-- utils.inc          print_str, newline, print_char, print_int, clear_screen, wait_key
|   |-- input.inc          read_int, read_product, lookup_price, lookup_name, read_payment
|   |-- menu.inc           display_menu
|   |-- math.inc           calc_change, break_change
|   `-- output.inc         print_change, print_receipt
|
|-- tests\                 standalone test programs
|   |-- t_utils.asm        tests utils.inc
|   |-- t_input.asm        tests input.inc
|   |-- t_math.asm         tests menu.inc and math.inc
|   `-- t_output.asm       tests output.inc
|
`-- tools\JWasm\JWasm.exe
```

The `.inc` files in `inc\` are pasted into the program at assembly time by the `include` directive (JWasm finds them through the `-Iinc` option), so there is **one build command** and no separate linking step. The built `.exe` files stay in the main folder, which is the folder you mount in DOSBox. File names stay within 8 characters before the extension because DOS cannot run longer names.

---

## 3. Build

From the VS Code terminal (PowerShell), in the project folder:

```powershell
cd D:\8086VendingMachine
.\build.bat vending
```

This produces `vending.exe` in the main folder. The same script builds the test programs: `.\build.bat t_utils`, `.\build.bat t_input`, `.\build.bat t_math`, `.\build.bat t_output`. `build.bat` looks for `<name>.asm` in `src\` first and then in `tests\`, so the command is the same for every program. Always run it from the project folder (the script changes to its own folder first, which `-Iinc` relies on).

Manual equivalent (what `build.bat` runs), from the project folder:

```powershell
& "D:\8086VendingMachine\tools\JWasm\JWasm.exe" -0 -mz -Iinc -Fo vending.exe src\vending.asm
```

| Option | Meaning |
|---|---|
| `-0` | target the 8086 CPU (only 8086 instructions are accepted) |
| `-mz` | write a DOS MZ executable |
| `-Iinc` | also search the `inc\` folder for `include` files |
| `-Fo` | output file name |

---

## 4. Run

Start DOSBox and enter:

```text
mount d D:\8086VendingMachine
d:
vending.exe
```

**Tip:** DOSBox caches the folder listing when it is mounted. If you add a **new** file while DOSBox is open, press **Ctrl+F4** (rescan drives) before running `dir` or the new program. Rebuilding an existing `.exe` does not need this.

---

## 5. How to use the program

1. The screen clears and the menu appears.
2. Enter a product code (`1` to `6`), or `0` to exit.
3. The price is shown. Enter an amount of money to insert (up to 999 per insertion), or `0` to cancel.
4. If the money is not enough, the shortfall is shown and you are asked for more. New amounts are **added** to what you already inserted.
5. When the total covers the price, a receipt is printed with the change broken into 50 / 25 / 10 / 5 / 1 units.
6. Press any key to return to the menu.
7. Cancelling refunds any money inserted so far.

Products:

| Code | Product | Price |
|---|---|---|
| 1 | Chips | 15 |
| 2 | Soda | 25 |
| 3 | Candy | 10 |
| 4 | Water | 20 |
| 5 | Coffee | 30 |
| 6 | Juice | 35 |

Example session (abbreviated; exact blank lines may differ):

```text
Enter product code: 5
Price: 30
Insert money (0 to cancel): 10
Insufficient funds. Short by: 20
Insert money (0 to cancel): 25

======== RECEIPT ========
Item:   Coffee
Price:  30
Paid:   35
Change: 5
1 x 5
=========================
Press any key to continue...
```

Invalid input (letters, an empty line, a number over 999, or a product code outside 0 to 6) shows an error and asks again.

---

## 6. Features

**Core (MUST):** product menu, code validation, price lookup, payment input, sufficiency check, change calculation, denomination breakdown, repeated purchases, clean exit, error messages.

**Extras (SHOULD):**

| ID | Feature |
|---|---|
| S1 | Cancel, retry payment with a running total, refund on cancel |
| S2 | Explicit rejection of non-numeric input |
| S3 | Receipt-style output with product name |
| S4 | Exact payment shown cleanly as `Change: 0` |
| S5 | Multi-digit amounts up to 999 |
| S6 | Screen clear between transactions, with a keypress pause |
| S7 | Menu redisplayed after every transaction |

---

## 7. Code design

**Calling convention:** arguments in registers, result in a register, and the carry flag (CF) signals an error (`CF=1` means invalid or not found). Routines save and restore every register they use unless their header comment says otherwise.

| Routine | Input | Output |
|---|---|---|
| `print_str` | DX = offset of `$`-terminated string | none |
| `print_char` | DL = character | none |
| `newline` | none | none |
| `print_int` | AX = unsigned integer | none |
| `clear_screen` | none | none |
| `wait_key` | none | waits for a key |
| `read_int` | none | AX = value (0..999), CF=1 if invalid |
| `read_product` | none | AL = code (0 = exit), re-prompts on bad input |
| `lookup_price` | AL = code (1..6) | AX = price, CF=1 if not found |
| `lookup_name` | AL = code (1..6) | DX = name string, CF=1 if not found |
| `read_payment` | none | AX = amount (0 = cancel), re-prompts on bad input |
| `display_menu` | none | none |
| `calc_change` | AX = payment, BX = price | CF=0: AX = change; CF=1: AX = shortfall |
| `break_change` | AX = change | `denom_count[]` filled (AX destroyed) |
| `print_change` | `change` variable, `denom_count[]` | prints total and breakdown |
| `print_receipt` | `sel_code`, `price`, `payment`, `change`, `denom_count[]` | prints the receipt |

**Data** (in `data.inc`): parallel arrays for product prices and name pointers, a denominations table (`50, 25, 10, 5, 1`) with a matching count array, message strings, and the working variables.

**Notable rules in the code**
- `DX` is cleared before every `DIV`, and the payment is compared before it is subtracted.
- Amounts are capped at 999 per entry, so the 16-bit arithmetic cannot overflow.
- Only 8086 instructions are used (JWasm's `-0` option rejects 186+ forms).

---

## 8. Testing

Each module has a standalone test program, so a failure points to one module. Results are recorded in [`TEST_RESULTS.md`](TEST_RESULTS.md).

| Program | Tests |
|---|---|
| `t_utils.exe` | `print_char`, `print_int`, `clear_screen`, `wait_key` |
| `t_input.exe` | `read_int` (interactive), `lookup_price`, `lookup_name`, `read_product`, `read_payment` |
| `t_math.exe` | `display_menu`, `calc_change`, `break_change` |
| `t_output.exe` | `print_change`, `print_receipt` |
| `vending.exe` | full end-to-end scenarios |

Rebuild and re-run the relevant test program after changing a module.

---

## 9. Debugging

Use **DOSBox-X**, which has a built-in debugger. Plain DOSBox usually lacks `debug.exe`. Build with the normal command and run the `.exe` inside DOSBox-X; see the DOSBox-X documentation for how to open its debugger.

---

## 10. Known limitations

- After invalid input at a prompt, only that prompt repeats; the menu is not redrawn until the transaction ends.
- Backspace is treated as an invalid character instead of deleting a digit.
- Each insertion is limited to 999.
- The screen clear assumes an 80x25 text screen.
- No stock levels, coin inventory, or admin mode (not in scope).

---

## 11. Team

| Role | Name | Modules |
|---|---|---|
| Integrator / lead | _(fill in)_ | `vending.asm` (main), `utils.inc` basics |
| Menu and messages | _(fill in)_ | `menu.inc`, message strings in `data.inc` |
| Input and validation | _(fill in)_ | `input.inc` |
| Arithmetic | _(fill in)_ | `math.inc`, data tables |
| Output and receipt | _(fill in)_ | `output.inc`, `print_int` |
| Docs and QA | _(fill in)_ | README, test results, flowchart, report |
