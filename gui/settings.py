"""settings.py - the settings of the vending machine GUI.

Change a value, save this file, and start the GUI again (py gui\\vending_gui.py).
Nothing else needs to be rebuilt. Product names, prices and the machine's coin
values are not here: they come from vending.exe (inc\\data.inc).
"""

# ---- Money buttons on the Customer tab ----------------------------------------
# One button per value; every value must be from 1 to 999.
PAY_BUTTONS = [1, 5, 10, 20, 50, 100]

# ---- DOSBox -------------------------------------------------------------------
AUTO_LAUNCH = True            # start DOSBox and vending.exe /g by itself
DOSBOX_PATH = ""              # full path of DOSBox.exe (or dosbox-x.exe); empty = look in the usual places
DOSBOX_MINIMIZED = True       # minimize the DOSBox window once it has opened

# ---- Project folder -------------------------------------------------------------
PROJECT_FOLDER = ""           # empty = the folder above gui\ (where vending.exe is)

# ---- Window ---------------------------------------------------------------------
WINDOW_TITLE = "Vending Machine"
WINDOW_MIN_SIZE = (820, 600)  # width, height in pixels
CARD_COLUMNS = 3              # product cards per row

# ---- Colors (#rrggbb) -----------------------------------------------------------
COLORS = {
    "window": "#eef1f5",          # background of the window
    "card": "#ffffff",            # a product card
    "card_selected": "#cfe3ff",   # the product being bought
    "card_soldout": "#e4e4e4",    # a sold-out product
    "text": "#1d2733",
    "muted": "#8a94a0",           # status bar and sold-out text
    "display_bg": "#10241a",      # the display of the Customer tab
    "display_ok": "#7dff9a",      # normal messages on the display
    "display_error": "#ff8a80",   # error messages on the display
}

# ---- Fonts (name, size[, style]) --------------------------------------------------
FONT = ("Segoe UI", 10)
FONT_BOLD = ("Segoe UI", 11, "bold")
FONT_DISPLAY = ("Consolas", 13, "bold")   # the display
FONT_RECEIPT = ("Consolas", 10)           # the receipt
