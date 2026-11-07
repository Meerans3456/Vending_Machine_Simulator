"""vending_gui.py - graphical front end for the vending machine.

The vending machine itself is vending.exe running in DOSBox in bridge mode
(vending.exe /g). This window only sends commands through bridge.py and draws
what the program answers; all selling, stock, coin and PIN logic stays in assembly.

Run:   py gui\\vending_gui.py            starts DOSBox and vending.exe /g by itself
       py gui\\vending_gui.py --manual   you start vending.exe /g yourself
Closing the window stops vending.exe and DOSBox.
"""
import glob
import os
import queue
import shutil
import subprocess
import sys
import threading
import time
import tkinter as tk
from tkinter import ttk

import settings
from bridge import Bridge, BridgeError
from settings import (AUTO_LAUNCH, CARD_COLUMNS, COLORS, DOSBOX_MINIMIZED, DOSBOX_PATH, FONT, FONT_BOLD,
                      FONT_DISPLAY, FONT_RECEIPT, PAY_BUTTONS, WINDOW_MIN_SIZE, WINDOW_TITLE)

# ---------------------------------------------------------------- settings
# (all settings are in settings.py)
PAY_BUTTONS = [value for value in PAY_BUTTONS if 1 <= value <= 999]
PROJECT_FOLDER = settings.PROJECT_FOLDER or os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# ------------------------------------------------------- starting DOSBox

def find_dosbox():
    """The path of DOSBox: DOSBOX_PATH, the DOSBOX_PATH environment variable, DOSBox.exe in the
    usual install folders, the PATH, and last DOSBox-X (dosbox-x.exe). None if nothing is found."""
    candidates = [DOSBOX_PATH, os.environ.get("DOSBOX_PATH", "")]
    folders = []
    for base in (os.environ.get("ProgramFiles(x86)"), os.environ.get("ProgramFiles"), "C:\\"):
        if base:
            folders += sorted(glob.glob(os.path.join(base, "DOSBox*")))
    candidates += [os.path.join(folder, "DOSBox.exe") for folder in folders]
    candidates.append(shutil.which("DOSBox.exe") or shutil.which("dosbox") or "")
    candidates += [os.path.join(folder, "dosbox-x.exe") for folder in folders]
    candidates.append(shutil.which("dosbox-x") or "")
    for path in candidates:
        if path and os.path.isfile(path):
            return path
    return None


def minimize_window_of(pid):
    """Windows only: minimizes the visible windows of the process 'pid'.
    Returns True if a window was found (DOSBox needs a moment before it has one)."""
    import ctypes
    from ctypes import wintypes
    user32 = ctypes.windll.user32
    found = []

    def callback(hwnd, lparam):
        owner = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(owner))
        if owner.value == pid and user32.IsWindowVisible(hwnd):
            found.append(hwnd)
        return True

    user32.EnumWindows(ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)(callback), 0)
    for hwnd in found:
        user32.ShowWindow(hwnd, 6)             # SW_MINIMIZE
    return bool(found)


def dos_path(folder):
    """The folder name in a form DOSBox's mount command accepts (no spaces)."""
    if " " in folder and os.name == "nt":
        import ctypes
        buffer = ctypes.create_unicode_buffer(260)
        if ctypes.windll.kernel32.GetShortPathNameW(folder, buffer, 260):
            return buffer.value
    return folder


def launch_dosbox(exe, folder):
    """Start DOSBox with the project folder mounted as D: and vending.exe /g running.
    DOSBox closes by itself when vending.exe ends (after the QUIT command)."""
    for name in ("CMD.TXT", "OUT.TXT"):          # remove files left by an earlier run
        try:
            os.remove(os.path.join(folder, name))
        except OSError:
            pass
    command = [exe, "-noconsole", "-c", "mount d " + dos_path(folder),
               "-c", "d:", "-c", "vending.exe /g", "-c", "exit"]
    return subprocess.Popen(command)


# ------------------------------------------------- text shown to the user
# (plain functions, so they do not depend on the window)

def seconds_text(seconds):
    minutes, secs = divmod(int(seconds), 60)
    return "%d min %02d sec" % (minutes, secs)


def product_of(state, code):
    for product in state["products"]:
        if product["code"] == code:
            return product
    return None


def sold_line(reply):
    """The 'R item price paid change' line of a completed sale, as a list of words."""
    for line in reply.lines:
        part = line.split()
        if part and part[0] == "R" and len(part) >= 5:
            return part
    return ["R", "?", "0", "0", "0"]


def format_receipt(reply):
    """The receipt text for the reply of a completed sale (OK SOLD)."""
    _, item, price, paid, change = sold_line(reply)[:5]
    lines = ["========== RECEIPT ==========",
             "Item:    " + item, "Price:   " + price,
             "Paid:    " + paid, "Change:  " + change]
    for line in reply.lines:
        part = line.split()
        if part and part[0] == "K" and len(part) >= 3:
            lines.append("   %s x %s" % (part[1], part[2]))
    lines.append("=============================")
    return "\n".join(lines)


def customer_message(verb, reply, state):
    """(text, is_error) for the display after a customer command."""
    words = reply.words
    why = words[1] if len(words) > 1 else ""
    if verb == "BUY":
        if reply.ok:
            product = product_of(state, state["purchase_product"])
            return "%s selected, price %d. Insert money." % (product["name"], product["price"]), False
        return {"soldout": "Sorry, that product is sold out.",
                "busy": "Finish or cancel the current purchase first.",
                "invalid": "That is not a valid product."}.get(why, "Could not start the purchase."), True
    if verb == "PAY":
        if reply.ok and why == "SHORT":
            return "Inserted %d. Insert %s more." % (state["inserted"], words[2]), False
        if reply.ok and why == "SOLD":
            item, change = sold_line(reply)[1], int(sold_line(reply)[4])
            if change > 0:
                return "Enjoy your %s! Take your change: %d." % (item, change), False
            return "Enjoy your %s!" % item, False
        if reply.ok and why == "NOCHANGE":
            return "Sorry, exact change is not available. Refunded %s." % words[2], True
        return {"notx": "Choose a product first.",
                "amount": "That amount is not accepted."}.get(why, "Payment failed."), True
    if verb == "CANCEL":
        refunded = int(words[2]) if reply.ok and len(words) > 2 else 0
        return ("Purchase cancelled. Refunded %d." % refunded if refunded else "Purchase cancelled."), False
    return reply.status, not reply.ok


def admin_message(verb, reply):
    """(text, is_error) for the admin tab after an admin command."""
    words = reply.words
    if reply.ok:
        return {"LOGIN": "Logged in.", "LOGOUT": "Logged out."}.get(verb, "Done."), False
    why = words[1] if len(words) > 1 else ""
    if why == "WRONG":
        return "Wrong admin code or PIN. %s tries left." % words[2], True
    if why == "LOCKED":
        return "Admin is locked for %s." % seconds_text(words[2]), True
    return {"login": "Please log in first.",
            "range": "That value is not allowed.",
            "invalid": "That is not valid.",
            "denom": "That is not a coin of this machine."}.get(why, reply.status), True


# ------------------------------------------------------------- the window

class VendingApp:
    def __init__(self, root, folder, auto_launch=True):
        self.root = root
        self.folder = folder
        self.dosbox = None               # the DOSBox process, when this window started it
        self.exit_reported = False
        self.minimize_tries = 0          # how often minimizing the DOSBox window was tried
        self.bridge = Bridge(folder)
        self.jobs = queue.Queue()        # commands waiting to be sent
        self.results = queue.Queue()     # answers waiting to be shown
        self.state = None
        self.lock_end = 0.0              # when the admin lock ends (time.time() value)
        self.closing = False
        self.cards = []                  # one dictionary of widgets per product card
        self.stock_entries = []
        self.coin_entries = []
        self.admin_tables_built = False

        root.title(WINDOW_TITLE)
        root.configure(bg=COLORS["window"])
        root.minsize(*WINDOW_MIN_SIZE)
        try:
            ttk.Style().theme_use("clam")
        except tk.TclError:
            pass
        self.build_window()
        if auto_launch:
            self.start_dosbox()
        root.protocol("WM_DELETE_WINDOW", self.on_close)
        threading.Thread(target=self.worker, daemon=True).start()
        root.after(50, self.pump)
        root.after(1000, self.tick)

    # ---------------------------------------------------- starting and stopping DOSBox
    def start_dosbox(self):
        exe = find_dosbox()
        if exe is None:
            self.status.set("DOSBox not found: start vending.exe /g yourself")
            self.show_message("DOSBox.exe was not found. Set DOSBOX_PATH at the top of "
                              "vending_gui.py, or start vending.exe /g yourself.", True)
            return
        try:
            self.dosbox = launch_dosbox(exe, self.folder)
            self.status.set("Starting DOSBox ...")
        except OSError as error:
            self.status.set("DOSBox could not be started")
            self.show_message("DOSBox could not be started: %s" % error, True)

    def finish_close(self):
        if self.dosbox is not None and self.dosbox.poll() is None:
            self.dosbox.terminate()          # vending.exe did not end by itself
        self.root.destroy()

    # ---------------------------------------------------- talking to the program
    def worker(self):
        """Runs in its own thread: sends commands one at a time and queues the answers."""
        while not self.closing:
            try:
                self.bridge.wait_ready(timeout=2.0)
                break
            except BridgeError:
                continue
        else:
            return
        try:
            self.results.put(("connected", self.bridge.state()))
        except (BridgeError, OSError) as error:
            self.results.put(("error", str(error)))
            return
        while True:
            job = self.jobs.get()
            if job is None:
                return
            command, callback = job
            try:
                reply = self.bridge.command(command)
                state = None if command == "QUIT" else self.bridge.state()
                self.results.put(("reply", command, reply, state, callback))
            except (BridgeError, OSError) as error:
                self.results.put(("error", str(error)))

    def send(self, command, callback=None):
        self.jobs.put((command, callback))

    def pump(self):
        """Runs in the window thread: shows the answers that have arrived."""
        try:
            while True:
                item = self.results.get_nowait()
                if item[0] == "connected":
                    self.status.set("Connected to vending.exe")
                    self.apply_state(item[1])
                    self.show_message("Choose a product.", False)
                elif item[0] == "reply":
                    _, command, reply, state, callback = item
                    if state is not None:
                        self.apply_state(state)
                    if callback:
                        callback(reply, state)
                else:
                    self.status.set("Connection problem")
                    self.show_message(item[1], True)
        except queue.Empty:
            pass
        self.root.after(50, self.pump)

    def tick(self):
        """Once a second: counts down the admin lock, minimizes the DOSBox window and
        notices if DOSBox has closed."""
        if (DOSBOX_MINIMIZED and os.name == "nt" and self.dosbox is not None
                and self.dosbox.poll() is None and self.minimize_tries < 30):
            self.minimize_tries = 99 if minimize_window_of(self.dosbox.pid) else self.minimize_tries + 1
        if (self.dosbox is not None and self.dosbox.poll() is not None
                and not self.closing and not self.exit_reported):
            self.exit_reported = True
            self.status.set("DOSBox has closed")
            self.show_message("The vending machine program has stopped. "
                              "Close this window and start it again.", True)
        left = int(self.lock_end - time.time())
        if left > 0:
            self.lock_label.config(text="Admin is locked. Try again in %s." % seconds_text(left))
            self.lock_label.grid()
            self.login_button.state(["disabled"])
        else:
            self.lock_label.grid_remove()
            if self.state is not None:
                self.login_button.state(["!disabled"])
        self.root.after(1000, self.tick)

    def clear_admin_fields(self):
        """Empties every box of the Admin tab and its message, so nothing typed there stays behind."""
        entries = [self.code_entry, self.pin_entry, self.code1, self.code2, self.pin1, self.pin2]
        entries += [entry for _, entry in self.stock_entries + self.coin_entries]
        for entry in entries:
            entry.delete(0, "end")
        self.admin_message_var.set("")

    def on_tab_changed(self, event):
        """Leaving the Admin tab empties its boxes and ends the admin session, so nothing typed
        there stays behind and the session is never left open by mistake. Opening the Admin tab
        is refused (the purchase is cancelled and refunded) while a purchase is open, and
        otherwise clears the customer screen."""
        tab = self.notebook.index(self.notebook.select())
        if tab == 0:
            self.clear_admin_fields()
            if self.state and self.state["admin"]:
                self.admin_command("LOGOUT")
                self.show_message("Choose a product.", False)
        elif self.state:
            if self.state["purchase_open"]:
                self.notebook.select(0)          # stay on the Customer tab
                self.cancel()                    # CANCEL refunds what was inserted
                return
            self.clear_admin_fields()
            self.show_message("", False)         # clear the display and the receipt
            self.set_receipt("")

    def on_close(self):
        self.closing = True
        self.send("QUIT")                      # ends vending.exe /g
        self.root.after(1500, self.finish_close)

    # ------------------------------------------------------------ building the window
    def build_window(self):
        notebook = ttk.Notebook(self.root)
        self.notebook = notebook
        notebook.pack(fill="both", expand=True, padx=8, pady=(8, 0))
        customer = ttk.Frame(notebook, padding=12)
        admin = ttk.Frame(notebook, padding=12)
        notebook.add(customer, text="  Customer  ")
        notebook.add(admin, text="  Admin  ")
        self.build_customer(customer)
        self.build_admin(admin)
        notebook.bind("<<NotebookTabChanged>>", self.on_tab_changed)

        bar = tk.Frame(self.root, bg=COLORS["window"])
        bar.pack(fill="x", padx=10, pady=4)
        self.status = tk.StringVar(value="Waiting for vending.exe /g ...")
        self.sales = tk.StringVar(value="")
        tk.Label(bar, textvariable=self.status, bg=COLORS["window"], fg=COLORS["muted"], font=FONT).pack(side="left")
        tk.Label(bar, textvariable=self.sales, bg=COLORS["window"], fg=COLORS["muted"], font=FONT).pack(side="right")

    def build_customer(self, parent):
        parent.columnconfigure(0, weight=3)
        parent.columnconfigure(1, weight=2)
        parent.rowconfigure(0, weight=1)
        self.grid_frame = tk.Frame(parent, bg=COLORS["window"])
        self.grid_frame.grid(row=0, column=0, sticky="nsew")
        right = tk.Frame(parent, bg=COLORS["window"])
        right.grid(row=0, column=1, sticky="nsew", padx=(14, 0))

        self.display = tk.Label(right, text="Waiting for vending.exe /g ...", font=FONT_DISPLAY,
                                bg=COLORS["display_bg"], fg=COLORS["display_ok"], height=3,
                                wraplength=300, justify="left", anchor="w", padx=10)
        self.display.pack(fill="x")

        tk.Label(right, text="Insert money", font=FONT_BOLD, bg=COLORS["window"], fg=COLORS["text"],
                 anchor="w").pack(fill="x", pady=(12, 0))
        money = tk.Frame(right, bg=COLORS["window"])
        money.pack(fill="x", pady=4)
        self.pay_buttons = []
        for number, value in enumerate(PAY_BUTTONS):
            button = ttk.Button(money, text=str(value), width=6, command=lambda v=value: self.pay(v))
            button.grid(row=number // 3, column=number % 3, padx=3, pady=3)
            self.pay_buttons.append(button)
        self.cancel_button = ttk.Button(right, text="Cancel and refund", command=self.cancel)
        self.cancel_button.pack(fill="x", pady=(4, 8))

        tk.Label(right, text="Receipt and change", font=FONT_BOLD, bg=COLORS["window"], fg=COLORS["text"],
                 anchor="w").pack(fill="x")
        self.receipt = tk.Text(right, height=11, width=32, font=FONT_RECEIPT, state="disabled",
                               bg=COLORS["card"], fg=COLORS["text"], relief="flat")
        self.receipt.pack(fill="both", expand=True)

    def build_admin(self, parent):
        self.admin_message_var = tk.StringVar(value="")
        self.admin_message_label = ttk.Label(parent, textvariable=self.admin_message_var, font=FONT, anchor="w")
        self.admin_message_label.pack(side="bottom", fill="x")

        # shown while no admin session is open
        self.login_frame = ttk.Frame(parent)
        self.login_frame.pack(fill="x")
        ttk.Label(self.login_frame, text="Admin code").grid(row=0, column=0, sticky="w", pady=2)
        self.code_entry = ttk.Entry(self.login_frame, show="*", width=12)
        self.code_entry.grid(row=0, column=1, padx=8, pady=2)
        self.code_entry.bind("<Return>", lambda event: self.pin_entry.focus_set())
        ttk.Label(self.login_frame, text="Admin PIN").grid(row=1, column=0, sticky="w", pady=2)
        self.pin_entry = ttk.Entry(self.login_frame, show="*", width=12)
        self.pin_entry.grid(row=1, column=1, padx=8, pady=2)
        self.pin_entry.bind("<Return>", lambda event: self.login())
        self.login_button = ttk.Button(self.login_frame, text="Log in", command=self.login)
        self.login_button.grid(row=1, column=2)
        # only shown while the admin is locked (it is hidden otherwise, so it leaves no gap)
        self.lock_label = ttk.Label(self.login_frame, text="", font=FONT, foreground="#c0392b")
        self.lock_label.grid(row=2, column=0, columnspan=3, sticky="w", pady=6)
        self.lock_label.grid_remove()

        # shown while an admin session is open
        self.panel = ttk.Frame(parent)
        top = ttk.Frame(self.panel)
        top.pack(fill="x")
        self.admin_sales = tk.StringVar(value="")
        ttk.Label(top, textvariable=self.admin_sales, font=FONT_BOLD).pack(side="left")
        ttk.Button(top, text="Log out", command=lambda: self.admin_command("LOGOUT")).pack(side="right")

        tables = ttk.Frame(self.panel)
        tables.pack(fill="x", pady=8)
        self.stock_frame = ttk.LabelFrame(tables, text="Product stock", padding=8)
        self.stock_frame.grid(row=0, column=0, sticky="nsew", padx=(0, 8))
        self.coin_frame = ttk.LabelFrame(tables, text="Coins in the machine", padding=8)
        self.coin_frame.grid(row=0, column=1, sticky="nsew")

        refill = ttk.Frame(self.panel)
        refill.pack(fill="x")
        ttk.Button(refill, text="Refill all products",
                   command=lambda: self.admin_command("REFILL PRODUCTS")).pack(side="left", padx=(0, 8))
        ttk.Button(refill, text="Refill all coins",
                   command=lambda: self.admin_command("REFILL COINS")).pack(side="left")

        secrets = ttk.LabelFrame(self.panel, text="Change the admin code or PIN", padding=8)
        secrets.pack(fill="x", pady=10)
        ttk.Label(secrets, text="New admin code (at least 10)").grid(row=0, column=0, sticky="w")
        self.code1 = ttk.Entry(secrets, width=10)
        self.code1.grid(row=0, column=1, padx=6)
        self.code2 = ttk.Entry(secrets, width=10)
        self.code2.grid(row=0, column=2, padx=6)
        ttk.Button(secrets, text="Change code", command=self.change_code).grid(row=0, column=3)
        ttk.Label(secrets, text="New PIN (1000 to 9999)").grid(row=1, column=0, sticky="w", pady=(6, 0))
        self.pin1 = ttk.Entry(secrets, width=10, show="*")
        self.pin1.grid(row=1, column=1, padx=6, pady=(6, 0))
        self.pin2 = ttk.Entry(secrets, width=10, show="*")
        self.pin2.grid(row=1, column=2, padx=6, pady=(6, 0))
        ttk.Button(secrets, text="Change PIN", command=self.change_pin).grid(row=1, column=3, pady=(6, 0))
        ttk.Label(secrets, text="(type the new value twice)", foreground=COLORS["muted"]).grid(
            row=2, column=0, columnspan=4, sticky="w", pady=(4, 0))

    # ------------------------------------------------------------ showing the state
    def show_message(self, text, is_error):
        self.display.config(text=text, fg=COLORS["display_error"] if is_error else COLORS["display_ok"])

    def show_admin_message(self, text, is_error):
        self.admin_message_var.set(text)
        self.admin_message_label.config(foreground="#c0392b" if is_error else "#1e8449")

    def set_receipt(self, text):
        self.receipt.config(state="normal")
        self.receipt.delete("1.0", "end")
        self.receipt.insert("1.0", text)
        self.receipt.config(state="disabled")

    def apply_state(self, state):
        self.state = state
        self.lock_end = time.time() + state["locked"] if state["locked"] > 0 else 0.0
        self.sales.set("Sales: %d    Total: %d" % (state["sales_count"], state["sales_total"]))
        self.update_cards(state)
        open_purchase = state["purchase_open"]
        for button in self.pay_buttons:
            button.state(["!disabled"] if open_purchase else ["disabled"])
        self.cancel_button.state(["!disabled"] if open_purchase else ["disabled"])
        self.update_admin(state)

    def update_cards(self, state):
        if not self.cards:                       # first state: build one card per product
            for number, product in enumerate(state["products"]):
                card = tk.Frame(self.grid_frame, bg=COLORS["card"], highlightthickness=1,
                                highlightbackground="#c9d1da", padx=10, pady=10)
                card.grid(row=number // CARD_COLUMNS, column=number % CARD_COLUMNS,
                          padx=6, pady=6, sticky="nsew")
                widgets = {"frame": card}
                for key, font in (("code", FONT), ("name", FONT_BOLD), ("price", FONT), ("stock", FONT)):
                    label = tk.Label(card, font=font, bg=COLORS["card"], fg=COLORS["text"])
                    label.pack(anchor="w")
                    widgets[key] = label
                for widget in list(widgets.values()):
                    widget.bind("<Button-1>", lambda event, c=product["code"]: self.choose(c))
                self.cards.append(widgets)
            for column in range(CARD_COLUMNS):
                self.grid_frame.columnconfigure(column, weight=1)
        open_purchase = state["purchase_open"]
        for product, widgets in zip(state["products"], self.cards):
            sold_out = product["stock"] == 0
            chosen = open_purchase and state["purchase_product"] == product["code"]
            colour = (COLORS["card_selected"] if chosen
                      else COLORS["card_soldout"] if sold_out else COLORS["card"])
            for widget in widgets.values():
                widget.config(bg=colour)
            widgets["code"].config(text="Code %d" % product["code"])
            widgets["name"].config(text=product["name"])
            widgets["price"].config(text="Price %d" % product["price"])
            widgets["stock"].config(text="SOLD OUT" if sold_out else "%d left" % product["stock"],
                                    fg=COLORS["muted"] if sold_out else COLORS["text"])
            widgets["frame"].config(cursor="arrow" if (sold_out or open_purchase) else "hand2")

    def update_admin(self, state):
        if state["admin"]:
            self.login_frame.pack_forget()
            self.panel.pack(fill="both", expand=True)
            if not self.admin_tables_built:
                self.build_admin_tables(state)
            self.admin_sales.set("Sales: %d    Total: %d" % (state["sales_count"], state["sales_total"]))
            for product, (label, _) in zip(state["products"], self.stock_entries):
                label.config(text=str(product["stock"]))
            for coin, (label, _) in zip(state["coins"], self.coin_entries):
                label.config(text=str(coin["count"]))
        else:
            self.panel.pack_forget()
            self.login_frame.pack(fill="x")

    def build_admin_tables(self, state):
        for row, product in enumerate(state["products"]):
            ttk.Label(self.stock_frame, text="%d  %s" % (product["code"], product["name"]), width=14).grid(row=row, column=0, sticky="w")
            current = ttk.Label(self.stock_frame, width=4)
            current.grid(row=row, column=1)
            entry = ttk.Entry(self.stock_frame, width=6)
            entry.grid(row=row, column=2, padx=4)
            ttk.Button(self.stock_frame, text="Set", width=5,
                       command=lambda c=product["code"], e=entry: self.set_value("SETSTOCK %d" % c, e)).grid(row=row, column=3)
            self.stock_entries.append((current, entry))
        for row, coin in enumerate(state["coins"]):
            ttk.Label(self.coin_frame, text="Coin %d" % coin["value"], width=10).grid(row=row, column=0, sticky="w")
            current = ttk.Label(self.coin_frame, width=4)
            current.grid(row=row, column=1)
            entry = ttk.Entry(self.coin_frame, width=6)
            entry.grid(row=row, column=2, padx=4)
            ttk.Button(self.coin_frame, text="Set", width=5,
                       command=lambda v=coin["value"], e=entry: self.set_value("SETCOIN %d" % v, e)).grid(row=row, column=3)
            self.coin_entries.append((current, entry))
        self.admin_tables_built = True

    # ------------------------------------------------------------ customer actions
    def choose(self, code):
        if self.state is None or self.state["purchase_open"]:
            return
        product = product_of(self.state, code)
        if product is None or product["stock"] == 0:
            self.show_message("Sorry, that product is sold out.", True)
            return
        self.send("BUY %d" % code, lambda r, s, c="BUY %d" % code: self.customer_done(c, r, s))

    def pay(self, amount):
        self.send("PAY %d" % amount, lambda r, s, c="PAY %d" % amount: self.customer_done(c, r, s))

    def cancel(self):
        self.send("CANCEL", lambda r, s: self.customer_done("CANCEL", r, s))

    def customer_done(self, command, reply, state):
        verb = command.split()[0]
        text, is_error = customer_message(verb, reply, state)
        self.show_message(text, is_error)
        if verb == "BUY" and reply.ok:
            self.set_receipt("")
        elif verb == "PAY" and reply.ok and reply.words[1] == "SOLD":
            self.set_receipt(format_receipt(reply))
        elif verb == "PAY" and reply.ok and reply.words[1] == "NOCHANGE":
            self.set_receipt("")

    # ------------------------------------------------------------ admin actions
    def admin_command(self, command):
        verb = command.split()[0]
        self.send(command, lambda r, s: self.show_admin_message(*admin_message(verb, r)))

    def login(self):
        code, pin = self.code_entry.get().strip(), self.pin_entry.get().strip()
        self.code_entry.delete(0, "end")
        self.pin_entry.delete(0, "end")
        if not (code.isdigit() and pin.isdigit()):
            self.show_admin_message("Type the admin code and the PIN (digits only).", True)
            return
        self.admin_command("LOGIN %s %s" % (code, pin))

    def set_value(self, command, entry):
        value = entry.get().strip()
        entry.delete(0, "end")
        if not value.isdigit():
            self.show_admin_message("Type a number first.", True)
            return
        self.admin_command("%s %s" % (command, value))

    def change_code(self):
        first, second = self.code1.get().strip(), self.code2.get().strip()
        self.code1.delete(0, "end")
        self.code2.delete(0, "end")
        if not (first.isdigit() and 10 <= int(first) <= 65535):
            self.show_admin_message("The new admin code must be a number from 10 to 65535.", True)
        elif first != second:
            self.show_admin_message("The two admin code entries do not match. Nothing changed.", True)
        else:
            self.admin_command("SETCODE " + first)

    def change_pin(self):
        first, second = self.pin1.get().strip(), self.pin2.get().strip()
        self.pin1.delete(0, "end")
        self.pin2.delete(0, "end")
        if not (first.isdigit() and 1000 <= int(first) <= 9999):
            self.show_admin_message("The new PIN must be a number from 1000 to 9999.", True)
        elif first != second:
            self.show_admin_message("The two PIN entries do not match. Nothing changed.", True)
        else:
            self.admin_command("SETPIN " + first)


def main():
    arguments = [a for a in sys.argv[1:] if not a.startswith("--")]
    folder = arguments[0] if arguments else PROJECT_FOLDER
    auto_launch = AUTO_LAUNCH and "--manual" not in sys.argv
    root = tk.Tk()
    VendingApp(root, folder, auto_launch)
    root.mainloop()


if __name__ == "__main__":
    main()
