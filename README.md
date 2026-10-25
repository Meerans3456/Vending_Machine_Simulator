# Vending Machine Simulator (8086 Assembly)

A console vending machine written in 16-bit 8086 assembly (MASM syntax). It shows a product menu, takes a product code, accepts payment (in several steps, with cancel and refund), works out the change, breaks it into 50 / 25 / 10 / 5 / 1 units, and prints a receipt. Several purchases can be made in one run.

It is assembled with **JWasm** and runs in **DOSBox**. The build script is a Windows `.bat` file, so the steps below are for **Windows**.

> New to the project? Follow the **Quick start**. If anything goes wrong, [Setup.md](Setup.md) has the full install guide and a troubleshooting table.

---

## Quick start

You need **Git**, **JWasm** and **DOSBox**. [Setup.md](Setup.md) explains where to get them.

**1. Clone the repository and open a terminal in its folder** (PowerShell or the VS Code terminal):

```powershell
git clone <repository URL>
cd <repository folder>
```

**2. Check that JWasm is in place.** This file must exist:

```text
tools\JWasm\JWasm.exe
```

If it is missing, see *Step 2* in [Setup.md](Setup.md).

**3. Build the program:**

```powershell
.\build.bat vending
```

This creates `vending.exe` in the project folder.

**4. Run it in DOSBox.** Start DOSBox and type these three lines, using the real path of your project folder:

```text
mount d "C:\path\to\your\project folder"
d:
vending.exe
```

**5. Try a purchase.** Enter `1` (Chips), then `50`. You should see a receipt with change of 35. Enter `0` at the menu to exit.

> The built `.exe` files are **not** stored in the repository. You must run `build.bat` yourself after cloning, and again whenever the code changes (for example after `git pull`).

---

## Using the program

1. The screen clears and the menu appears.
2. Enter a product code (`1` to `6`), or `0` to exit.
3. The price is shown. Enter an amount of money to insert (up to 999 per insertion), or `0` to cancel.
4. If the money is not enough, the shortfall is shown and you are asked for more. New amounts are **added** to what you already inserted.
5. When the total covers the price, a receipt is printed with the change broken into 50 / 25 / 10 / 5 / 1 units.
6. Press any key to return to the menu.
7. Cancelling refunds any money inserted so far.

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
==== VENDING MACHINE ====
1. Chips - 15
2. Soda - 25
3. Candy - 10
4. Water - 20
5. Coffee - 30
6. Juice - 35
0. Exit

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

## Repository layout

```text
<project folder>\
|-- src\
|   `-- vending.asm        main program: includes all modules, contains main
|
|-- inc\                   modules, pasted into the program at assembly time
|   |-- data.inc           constants, tables, messages, variables (included first)
|   |-- utils.inc          print_str, newline, print_char, print_int, clear_screen, wait_key
|   |-- lookup.inc         code_to_offset, lookup_price, lookup_name
|   |-- input.inc          read_int, read_product, read_payment
|   |-- menu.inc           display_menu
|   |-- math.inc           calc_change, break_change
|   `-- output.inc         print_change, print_receipt
|
|-- tests\                 standalone test programs (one per module)
|   |-- t_utils.asm        tests utils.inc
|   |-- t_lookup.asm       tests lookup.inc
|   |-- t_input.asm        tests input.inc
|   |-- t_math.asm         tests menu.inc and math.inc
|   `-- t_output.asm       tests output.inc
|
|-- tools\JWasm\           the assembler (JWasm.exe), see Setup.md
|-- build.bat              assembles one program: build <name>
|-- README.md              this file
|-- Setup.md               install guide and troubleshooting
|-- TEST_RESULTS.md        how to run the tests, and the recorded results
|-- .gitignore, .gitattributes
```

**Created on your machine, not stored in the repository:**

- the built programs (`*.exe`), made by `build.bat`;
- `docs\` (the project report and flowcharts), which the team keeps outside the repository;
- assembler leftovers such as `*.obj`, `*.lst` and `*.map`.

File names stay within 8 characters before the extension, because DOS cannot run longer names.

---

## How the code is organised

The code is split into module files. `vending.asm` pulls them in with the `include` directive, so **one build command** produces the whole program and no separate linking step is needed. Every module has exactly one test program.

### Calling convention

Arguments go in registers, results come back in registers, and the carry flag (CF) signals an error (`CF=1` means invalid or not found). Routines save and restore every register they use, except those documented as outputs.

### Routines

| Routine | Module | Input | Output |
|---|---|---|---|
| `print_str` | utils | DX = offset of `$`-terminated string | none |
| `print_char` | utils | DL = character | none |
| `newline` | utils | none | none |
| `print_int` | utils | AX = unsigned integer | none |
| `clear_screen` | utils | none | none |
| `wait_key` | utils | none | waits for a key |
| `code_to_offset` | lookup | AL = product code | BX = table offset, CF=1 if the code is not valid |
| `lookup_price` | lookup | AL = product code | AX = price, CF=1 if not found |
| `lookup_name` | lookup | AL = product code | DX = name string, CF=1 if not found |
| `read_int` | input | none | AX = value (0 to `MAX_INPUT`), CF=1 if invalid |
| `read_product` | input | none | AL = code (0 = exit), asks again until valid |
| `read_payment` | input | none | AX = amount (0 = cancel), asks again until valid |
| `display_menu` | menu | none | none |
| `calc_change` | math | AX = payment, BX = price | CF=0: AX = change; CF=1: AX = shortfall |
| `break_change` | math | AX = change | `denom_count[]` filled; AX preserved |
| `print_change` | output | `change`, `denom_count[]` | prints total and breakdown |
| `print_receipt` | output | `sel_code`, `price`, `payment`, `change`, `denom_count[]` | prints the receipt |

### Data (`inc\data.inc`)

- **Constants:** `CR`, `LF`, `MAX_INPUT` (999), `TEXT_ATTR`, `SCREEN_END`.
- **Product tables:** `product_prices` and `product_names` are parallel arrays. `NUM_PRODUCTS` is computed by the assembler from `product_prices`, and a build-time check (`.errnz`) stops the build if `product_names` has a different length.
- **Denominations:** `denoms` (50, 25, 10, 5, 1, in descending order) with `NUM_DENOMS` computed from it; `denom_count` is sized from `NUM_DENOMS`.
- **Messages:** every string ends with `$`, and all text lives in this file, not in the code.
- **Variables:** `sel_code`, `price`, `payment` (running total), `change`, `shortfall`.

### Rules followed in the code

- The product tables are the only place product data is stored; the menu, validation, lookups and receipt all read from them.
- `DX` is cleared before every `DIV`, and the payment is compared with the price before it is subtracted.
- Input is capped at `MAX_INPUT` per entry, so the 16-bit arithmetic cannot overflow.
- Only 8086 instructions are used (JWasm's `-0` option rejects 186+ forms).

---

## Adding or changing a product

Everything is driven by three lines in `inc\data.inc`. To add a seventh product, add its price, its name string and a pointer to it:

```asm
product_prices  dw 15, 25, 10, 20, 30, 35, 40        ; add the price
pname7          db 'Gum$'                             ; add the name (next to the other names)
product_names   dw pname1, pname2, pname3, pname4, pname5, pname6, pname7   ; add the pointer
```

Rebuild, and the menu, the code validation, the lookups and the receipt all include the new product. To change a price, edit it in `product_prices` only. If the number of names and prices ever differ, the build stops with an error.

---

## Testing

Each module has its own test program, so a failure points to one module.

| Program | Tests |
|---|---|
| `t_utils` | `print_char`, `print_int`, `clear_screen`, `wait_key` |
| `t_lookup` | `code_to_offset`, `lookup_price`, `lookup_name` |
| `t_input` | `read_int` (interactive), `read_product`, `read_payment` |
| `t_math` | `display_menu`, `calc_change`, `break_change` |
| `t_output` | `print_change`, `print_receipt` |
| `vending` | full end-to-end scenarios |

Build and run them the same way as the main program, for example `.\build.bat t_math` and then `t_math.exe` in DOSBox. [TEST_RESULTS.md](TEST_RESULTS.md) explains how to run each one, what you should see, and records the results (67 checks, all passing).

---

## Contributing

- After `git pull`, run `.\build.bat vending` again; the `.exe` files are not in the repository.
- **Before committing, rebuild and re-run the test programs** for any module you changed (and `vending` for end-to-end behaviour).
- Keep the markdown files in step with the code. If behaviour, folders or commands change, update `README.md`, `Setup.md` and `TEST_RESULTS.md` in the **same commit**. The team commits once per finished phase so the code and the documents always match.
- Do not commit `.exe`, `.obj`, `.lst`, `.map`, `.tmp` or `.bak` files, or `docs\`. `.gitignore` already excludes them.
- Keep the style: no duplicated logic, no repeated data, no unexplained numbers (use a named constant in `data.inc`).

More detail on the Git workflow is in [Setup.md](Setup.md).

---

## Known limitations

- After invalid input at a prompt, only that prompt repeats; the menu is not redrawn until the transaction ends.
- Backspace is treated as an invalid character instead of deleting a digit.
- Each insertion is limited to 999.
- Product names in the menu are not column-aligned (lines are built from the tables).
- The screen clear assumes an 80x25 text screen.

## Planned extensions

Sales counter and total; out-of-stock products; coin inventory that depletes; admin and restock mode; file input and output so stock and sales survive between runs; colored text; a graphics-mode interface.

---

## Team

| Member | Name | Modules and documents |
|---|---|---|
| 1 | _(fill in)_ | _(fill in)_ |
| 2 | _(fill in)_ | _(fill in)_ |
| 3 | _(fill in)_ | _(fill in)_ |
| 4 | _(fill in)_ | _(fill in)_ |
