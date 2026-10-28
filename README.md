# Vending Machine Simulator (8086 Assembly)

A console vending machine written in 16-bit 8086 assembly (MASM syntax). It shows a product menu, takes a product code, accepts payment (in several steps, with cancel and refund), works out the change, breaks it into 50 / 25 / 10 / 5 / 1 coins taken from the machine's coin stock, and prints a receipt. Products can sell out, the machine keeps a coin inventory, a PIN-protected admin mode restocks it, and stock and coins are saved to a file between runs. Several purchases can be made in one run.

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

This creates `vending.exe` in the project folder. The first run also creates a save file, `VENDING.DAT`, in that folder.

**4. Run it in DOSBox.** Start DOSBox and type these three lines, using the real path of your project folder:

```text
mount d "C:\path\to\your\project folder"
d:
vending.exe
```

**5. Try a purchase.** Enter `1` (Chips), then `50`. You should see a receipt with change of 35. Enter `0` at the menu to exit; a summary of the sales made is shown.

> The built `.exe` files are **not** stored in the repository. You must run `build.bat` yourself after cloning, and again whenever the code changes (for example after `git pull`).

---

## Using the program

1. The screen clears and the menu appears. A product with no stock left is marked `(sold out)`.
2. Enter a product code (`1` to `6`), or `0` to exit. Choosing a sold-out product shows a message and asks again.
3. The price is shown. Enter an amount of money to insert (up to 999 per insertion), or `0` to cancel.
4. If the money is not enough, the shortfall is shown and you are asked for more. New amounts are **added** to what you already inserted.
5. When the total covers the price, the change is made from the machine's coins and a receipt is printed with the change broken into 50 / 25 / 10 / 5 / 1 coins.
6. The product leaves the stock, the change coins leave the coin stock, and the inserted money is added to the coin stock.
7. Press any key to return to the menu.
8. Cancelling refunds any money inserted so far. If the machine does not have the coins for exact change, nothing is sold and the money is refunded.
9. Exiting shows the number of sales made and the total taken.

The machine starts full: 3 units of each product and 10 coins of each denomination.

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

Invalid input (letters, an empty line, a number over 999 at a payment prompt, or a product code outside 0 to 6) shows an error and asks again.

### Admin mode

Type the secret number `31415` at the product prompt, then the PIN `1234`. Both are constants in `inc\data.inc` (`ADMIN_CODE`, `ADMIN_PIN`). There is a single attempt: anything other than the right PIN, including unreadable input, returns to the menu. The admin menu offers:

| Choice | Action |
|---|---|
| 1 | View product stock, coins and sales |
| 2 | Refill all products |
| 3 | Refill all coins |
| 4 | Set the stock of one product |
| 5 | Set the coins of one denomination |
| 0 | Leave admin mode |

### Save file

The product stock and coin stock are written to `VENDING.DAT` every time the menu is shown and are loaded at the next start. Sales totals are not saved and restart at zero each run. The file is created in the folder the program runs from (the folder mounted in DOSBox). To reset the machine to full, delete `VENDING.DAT` or use the admin refill options.

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
|   |-- stock.inc          lookup_stock, take_stock, take_coins, put_coins
|   |-- input.inc          read_int, ask_int, read_product
|   |-- menu.inc           display_menu
|   |-- math.inc           calc_change, break_change
|   |-- output.inc         print_change, print_receipt, print_summary
|   |-- admin.inc          admin_mode and the routines it offers
|   `-- file.inc           load_state, save_state
|
|-- tests\                 standalone test programs (one per module)
|   |-- t_utils.asm        tests utils.inc
|   |-- t_lookup.asm       tests lookup.inc
|   |-- t_stock.asm        tests stock.inc
|   |-- t_input.asm        tests input.inc
|   |-- t_math.asm         tests menu.inc and math.inc
|   |-- t_output.asm       tests output.inc
|   |-- t_admin.asm        tests admin.inc
|   `-- t_file.asm         tests file.inc
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
- the save files (`*.dat`), such as `VENDING.DAT`, written when the program runs;
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
| `lookup_stock` | stock | AL = product code | AX = units left, CF=1 if not found |
| `take_stock` | stock | AL = product code | one unit removed; CF=1 if unknown code or already at 0 |
| `take_coins` | stock | `denom_count[]` | those coins removed from `coin_stock[]` |
| `put_coins` | stock | AX = amount | amount added to `coin_stock[]`, largest coins first |
| `read_int` | input | DX = largest value accepted | AX = value, CF=1 if invalid |
| `ask_int` | input | DX = prompt string, BX = largest value | AX = value, asks again until valid |
| `read_product` | input | none | AX = `ADMIN_CODE`, 0 (exit) or a product in stock; asks again until valid |
| `display_menu` | menu | none | none |
| `calc_change` | math | AX = payment, BX = price | CF=0: AX = change; CF=1: AX = shortfall |
| `break_change` | math | AX = change | `denom_count[]` filled from the coin stock, AX preserved; CF=1 if exact change is not possible |
| `print_change` | output | `change`, `denom_count[]` | prints total and breakdown |
| `print_receipt` | output | `sel_code`, `price`, `payment`, `change`, `denom_count[]` | prints the receipt |
| `print_summary` | output | `sales_count`, `sales_total` | prints sales made and total taken |
| `admin_mode` | admin | none | asks the PIN once, then shows the admin menu until 0 |
| `ask_pin` | admin | none | CF=0 if the PIN is right, CF=1 if wrong |
| `admin_view`, `refill_products`, `refill_coins`, `set_product_stock`, `set_coin_count` | admin | none | the admin menu actions (choices 1 to 5) |
| `save_state` | file | none | writes stock and coins to `VENDING.DAT` |
| `load_state` | file | none | reads `VENDING.DAT`; nothing changes if the file is missing or does not match |

### Data (`inc\data.inc`)

- **Constants:** `CR`, `LF`, `MAX_INPUT` (999), `FULL_STOCK` (3), `FULL_COINS` (10), `ADMIN_CODE`, `ADMIN_PIN`, `PIN_MAX`, `TEXT_ATTR`, `SCREEN_END`.
- **Product tables:** `product_prices` and `product_names` are parallel arrays. `NUM_PRODUCTS` is computed by the assembler from `product_prices`, and a build-time check (`.errnz`) stops the build if `product_names` has a different length.
- **Denominations:** `denoms` (50, 25, 10, 5, 1, in descending order) with `NUM_DENOMS` computed from it; `denom_count` is sized from `NUM_DENOMS`.
- **Saved state:** `product_stock` and `coin_stock` follow `save_check` inside one block (`save_block`) that `file.inc` writes to and reads from `VENDING.DAT`. `SAVE_BYTES` is computed from the block, so a save file written for tables of a different size is ignored.
- **Messages:** every string ends with `$`, and all text lives in this file, not in the code.
- **Variables:** `sel_code`, `price`, `payment` (running total), `change`, `shortfall`, `sales_count`, `sales_total`.

### Rules followed in the code

- The product tables are the only place product data is stored; the menu, validation, lookups, stock and receipt all read from them.
- `break_change` only calculates; `take_coins` removes the coins afterwards, so a sale that cannot be paid in exact change leaves the machine unchanged.
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

Rebuild, and the menu, the code validation, the lookups and the receipt all include the new product; its stock is sized from the table. A save file written for a different number of products is ignored, so delete `VENDING.DAT` after adding one. To change a price, edit it in `product_prices` only. If the number of names and prices ever differ, the build stops with an error.

---

## Testing

Each module has its own test program, so a failure points to one module.

| Program | Tests |
|---|---|
| `t_utils` | `print_char`, `print_int` |
| `t_lookup` | `code_to_offset`, `lookup_price`, `lookup_name` |
| `t_stock` | `lookup_stock`, `take_stock`, `take_coins`, `put_coins` |
| `t_input` | `read_int`, `ask_int`, `read_product` (interactive) |
| `t_math` | `display_menu`, `calc_change`, `break_change` |
| `t_output` | `print_change` |
| `t_admin` | admin menu actions (parts 1 to 4 automatic, 5 to 7 interactive) |
| `t_file` | `save_state`, `load_state` (uses its own `T_STATE.DAT`, never your `VENDING.DAT`) |
| `vending` | full end-to-end scenarios |

Build and run them the same way as the main program, for example `.\build.bat t_math` and then `t_math.exe` in DOSBox. [TEST_RESULTS.md](TEST_RESULTS.md) explains how to run each one, what you should see, and records the results.

---

## Contributing

- After `git pull`, run `.\build.bat vending` again; the `.exe` files are not in the repository.
- **Before committing, rebuild and re-run the test programs** for any module you changed (and `vending` for end-to-end behaviour).
- Keep the markdown files in step with the code. If behaviour, folders or commands change, update `README.md`, `Setup.md` and `TEST_RESULTS.md` in the **same commit**. The team commits once per finished phase so the code and the documents always match.
- Do not commit `.exe`, `.dat`, `.obj`, `.lst`, `.map`, `.tmp` or `.bak` files, or `docs\`. `.gitignore` already excludes them.
- Keep the style: no duplicated logic, no repeated data, no unexplained numbers (use a named constant in `data.inc`).

More detail on the Git workflow is in [Setup.md](Setup.md).

---

## Known limitations

- After invalid input at a prompt, only that prompt repeats; the menu is not redrawn until the transaction ends.
- Backspace is treated as an invalid character instead of deleting a digit.
- Each insertion is limited to 999.
- Change is made largest coin first from the coins in stock; if that fails, the sale is refused even when another combination of coins could work.
- The admin PIN allows a single attempt per entry to admin mode.
- Sales totals are not saved between runs.
- Product names in the menu are not column-aligned (lines are built from the tables).
- The screen clear assumes an 80x25 text screen.

## Planned extensions

Exact-change search over all coin combinations; colored text; a graphics-mode interface.
