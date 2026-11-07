# Setup Guide

This guide takes a new team member from "nothing installed" to "program running" on **Windows**. Allow about 15 minutes.

**Contents:** [1. Tools](#1-tools-you-need) · [2. Get the code](#2-get-the-code) · [3. JWasm](#3-install-jwasm) · [4. DOSBox](#4-install-dosbox) · [5. Build](#5-build-the-program) · [6. Run](#6-run-it-in-dosbox) · [7. Tests](#7-run-the-tests) · [8. Python window](#8-the-python-window-gui) · [9. Debugging](#9-debugging-optional) · [10. Git workflow](#10-working-with-git) · [11. Troubleshooting](#11-troubleshooting)

---

## 1. Tools you need

| Tool | What it is for | Required? | Where to get it |
|---|---|---|---|
| Git | download and share the code | yes | git-scm.com |
| JWasm v2.11 | the assembler: turns `.asm` source into a DOS `.exe` | yes | github.com/Baron-von-Riedesel/JWasm (see step 3) |
| DOSBox | runs the 16-bit DOS program | yes | dosbox.com |
| DOSBox-X | like DOSBox, with a built-in debugger | optional | dosbox-x.com |
| Python 3 (with tkinter) | the optional window `gui\vending_gui.py` (tested with 3.13) | only for the window | python.org |
| VS Code | editing the source (any editor works) | optional | code.visualstudio.com |

**You do not need Visual Studio.** Its `ml.exe` assembler produces 32-bit Windows code and cannot build 16-bit DOS programs, which is why this project uses JWasm.

---

## 2. Get the code

In PowerShell (or the VS Code terminal):

```powershell
git clone <repository URL>
cd <repository folder>
```

Every command below is run from this **project folder** unless it says otherwise.

---

## 3. Install JWasm

First check whether it is already there:

```text
tools\JWasm\JWasm.exe
```

**If that file exists, skip to step 4.**

Otherwise:

1. Download the Windows binary package of JWasm. The official source is the JWasm project on GitHub (`Baron-von-Riedesel/JWasm`, releases page); the Windows v2.11 package is named `JWasm211bw.zip` and is also published on the JWasm SourceForge page.
2. Unzip it and copy **`JWasm.exe`** into the project at `tools\JWasm\` (create the folders if they do not exist). Do not rename the file.
3. Check that it runs:

   ```powershell
   .\tools\JWasm\JWasm.exe
   ```

   It should print JWasm's version and a list of options.

The project was built and tested with **JWasm v2.11**. Newer 2.x releases should work too, but they have not been tried with this project.

---

## 4. Install DOSBox

Install DOSBox with the default options. If you also want to step through the code, install DOSBox-X as well (see step 9). Both are separate programs from the project; nothing needs configuring.

---

## 5. Build the program

From the project folder:

```powershell
.\build.bat vending
```

(In PowerShell you must type the `.\` in front of `build.bat`.)

What it does: `build.bat` finds `src\vending.asm`, runs JWasm on it, and writes **`vending.exe`** into the project folder. Success means no error lines and a new `vending.exe`.

The same script builds every test program. Give it the program name without an extension:

| Name | Source |
|---|---|
| `vending` | `src\vending.asm` |
| `t_utils`, `t_lookup`, `t_stock`, `t_input`, `t_math`, `t_output`, `t_admin`, `t_file` | `tests\<name>.asm` |

What `build.bat` runs (you never need to type this yourself):

```powershell
tools\JWasm\JWasm.exe -0 -mz -Iinc -Fo vending.exe src\vending.asm
```

| Option | Meaning |
|---|---|
| `-0` | target the 8086 CPU (only 8086 instructions are accepted) |
| `-mz` | write a DOS MZ executable |
| `-Iinc` | also search the `inc\` folder for `include` files |
| `-Fo` | output file name |

---

## 6. Run it in DOSBox

DOSBox cannot see your Windows folders until you "mount" one. Start DOSBox and type, using the real path of your project folder (keep the quotes if the path has spaces):

```text
mount d "C:\path\to\your\project folder"
d:
dir
vending.exe
```

- `mount d ...` makes your project folder the drive **D:** inside DOSBox.
- `d:` switches to that drive, and `dir` lists its files. You should see `VENDING.EXE`.
- `vending.exe` starts the program. Enter `0` at the menu to exit it.
- The program creates `VENDING.DAT` in the mounted folder to save its stock, coins, sales totals and admin code and PIN. Delete that file to start again with a full machine and the default admin code and PIN.
- Type `exit` to close DOSBox.

> **Important:** DOSBox reads the folder when you mount it and remembers the listing. If you build a **new** program while DOSBox is open, press **Ctrl+F4** (rescan drives) before running it. Rebuilding a program that already exists does not need this.

---

## 7. Run the tests

Every module has its own test program. To build all of them in one go (PowerShell, from the project folder):

```powershell
'vending','t_utils','t_lookup','t_stock','t_input','t_math','t_output','t_admin','t_file' | ForEach-Object { .\build.bat $_ }
```

Press **Ctrl+F4** in DOSBox if it was already open, then run each `.exe` and compare the output with the expected results. [TEST_RESULTS.md](TEST_RESULTS.md) lists, for every test program, what to type (some are interactive), what you should see, and the recorded results.

---

## 8. The Python window (GUI)

This step is optional: the program works without it. The window starts DOSBox and `vending.exe` by itself and lets you use the machine with the mouse.

1. **Check Python.** In PowerShell run `py --version` (any recent Python 3; 3.13 was used) and `py -m tkinter`, which should open a small test window. If tkinter is missing, reinstall Python from python.org and keep the *tcl/tk* option ticked.
2. **Build the program** (`.\build.bat vending`, step 5).
3. **Start the window** from the project folder:

```powershell
py gui\vending_gui.py
```

The window finds DOSBox, deletes any old `CMD.TXT` and `OUT.TXT`, starts DOSBox (minimized) with your project folder mounted as **D:** and runs `vending.exe /g`. The status bar changes to *Connected to vending.exe* and the product cards appear. Closing the window stops the program and DOSBox.

- **If DOSBox is not found,** the display says so. Put the full path of `DOSBox.exe` in `DOSBOX_PATH` in `gui\settings.py`, or set a `DOSBOX_PATH` environment variable. `dosbox-x.exe` is tried as a last resort; DOSBox-X may ask you to confirm something the first time it is started this way.
- **Manual mode.** `py gui\vending_gui.py --manual` does not start DOSBox. Start `vending.exe /g` in DOSBox yourself first, then run the command.
- **Settings.** Buttons, colors, fonts, window size and the DOSBox options are in `gui\settings.py`. Edit it, save it and start the window again.
- **Check the link without the window.** Delete `VENDING.DAT`, start `vending.exe /g` in DOSBox, then run `py gui\test_bridge.py`. It prints a PASS or FAIL line for every command and ends with `QUIT`. Delete `VENDING.DAT` again afterwards, because the test leaves sales in it.

---

## 9. Debugging (optional)

DOSBox-X has a built-in debugger; plain DOSBox usually does not. Build as usual, mount the same folder in DOSBox-X, run the `.exe`, and use DOSBox-X's debugger (see the DOSBox-X documentation for how to open it).

---

## 10. Working with Git

**What is in the repository:** the source (`src\`, `inc\`, `tests\`, `gui\`), `build.bat`, the markdown files, `.gitignore` and `.gitattributes`.

**What is not** (listed in `.gitignore`):

- built programs: `*.exe` in the project folder;
- save files: `*.dat` (for example `VENDING.DAT`, written when the program runs);
- bridge files `CMD.TXT` and `OUT.TXT` (created each time the program runs in bridge mode) and Python's `__pycache__\` folders; add `CMD.TXT`, `OUT.TXT` and `__pycache__/` to `.gitignore`;
- `docs\` (the project report and flowcharts, which the team keeps outside the repository);
- assembler leftovers: `*.obj`, `*.o`, `*.map`, `*.lst`, `*.tmp`, `*.bak`.

Because the `.exe` files are not stored, **rebuild after every clone or pull**.

Typical workflow:

```powershell
git pull                              # get the latest code
.\build.bat vending                   # rebuild (the .exe files are not in the repository)

# ... edit the code ...

.\build.bat t_math                    # rebuild and re-run the test program of the module you changed
.\build.bat vending                   # and the main program

git status                            # no .exe files should appear here
git add -A
git commit -m "Short description of the change"
git push
```

Team rules:

- **Code and markdown files change together.** If behaviour, folders or commands change, update `README.md`, `Setup.md` and `TEST_RESULTS.md` in the same commit. The team commits once per finished phase, so the repository always matches its documents.
- Re-run the tests before committing.
- If `git status` shows `.exe` files, something is wrong with `.gitignore`. Do not commit them.
- `.gitattributes` keeps `build.bat` in Windows line endings on every machine. Leave it as it is.

---

## 11. Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| PowerShell says `build.bat` "is not recognized" | PowerShell does not run scripts from the current folder by name | Type `.\build.bat vending` |
| `JWasm.exe not found at tools\JWasm\JWasm.exe` | JWasm is not in the expected place | Do step 3 |
| `Cannot find xxx.asm in src\ or tests\` | Misspelled program name | Use one of: `vending`, `t_utils`, `t_lookup`, `t_stock`, `t_input`, `t_math`, `t_output`, `t_admin`, `t_file` (no extension) |
| JWasm cannot open an include file such as `data.inc` | JWasm was run by hand from another folder, or without `-Iinc` | Use `build.bat` from the project folder, or add `-Iinc` and run from the project folder |
| The assembler stops with a "forced error" in `data.inc` | `product_names` and `product_prices` have a different number of entries | Make the two tables the same length (README, *Adding or changing a product*) |
| DOSBox says `Illegal command: xxx.exe` | The file is not in the mounted folder (not built yet, wrong folder), DOSBox has not seen it yet, or the name is longer than 8 characters | Run `dir` to check; build it; press **Ctrl+F4**; keep names to 8 characters |
| `dir` in DOSBox does not show a file you just built | DOSBox cached the folder listing at mount time | Press **Ctrl+F4**, or `exit` and mount again |
| `mount` reports an error | Wrong path, or a path with spaces without quotes | Check the path exists and put it in quotes |
| The program behaves like an old version | The `.exe` was not rebuilt after `git pull` or an edit | Run `.\build.bat vending` again |
| The machine starts with less stock or fewer coins than expected, or the admin code or PIN is not the default | The previous run saved its state in `VENDING.DAT` | Delete `VENDING.DAT` (this also restores the default admin code and PIN), or use the admin refill options |
| `py` is not recognized | Python is not installed, or without the `py` launcher | Install Python from python.org (keep the *py launcher* option ticked) |
| `No module named tkinter` | Python was installed without tkinter | Reinstall Python with the *tcl/tk* option |
| The window says *Waiting for vending.exe /g* and nothing happens | DOSBox did not start the program: `vending.exe` is not built, DOSBox is the wrong one, or its path is wrong | Build `vending.exe`; check `DOSBOX_PATH` in `gui\settings.py`; or use `--manual` to see DOSBox's own messages |
| The display says *DOSBox.exe was not found* | DOSBox is not in a usual folder or on the `PATH` | Set `DOSBOX_PATH` in `gui\settings.py` |
| The display says the program has stopped | DOSBox or `vending.exe` ended | Close the window and start it again |
| `--manual` mode never connects | `vending.exe` was started without `/g` | Run `vending.exe /g` in DOSBox |
| `test_bridge.py` fails in the admin checks | `VENDING.DAT` from earlier runs holds other stock or a changed code or PIN | Delete `VENDING.DAT`, restart `vending.exe /g` and run it again |
| `CMD.TXT` and `OUT.TXT` stay in the folder | Normal: they are recreated at every bridge start | Ignore them; they are not committed |
