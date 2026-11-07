"""test_bridge.py - checks bridge mode of vending.exe, command by command.

Before running it:
  1. delete VENDING.DAT (so the machine starts full with the default admin code 31415 and PIN 1234)
  2. start the program in DOSBox:   vending.exe /g
Then, from the project folder:
  py gui\\test_bridge.py            (or: py gui\\test_bridge.py D:\\8086VendingMachine)
The test ends with QUIT, so vending.exe stops. Delete VENDING.DAT again afterwards:
the test leaves 2 sales in it.
"""
import os
import sys

from bridge import Bridge, BridgeError

passed = failed = 0


def check(name, ok, detail=""):
    global passed, failed
    if ok:
        passed += 1
        print("PASS  " + name)
    else:
        failed += 1
        print("FAIL  %s   %s" % (name, detail))


def expect(b, command, status, lines=None):
    """Send a command and compare the status line (and the data lines, if given)."""
    r = b.command(command)
    ok = r.status == status and (lines is None or r.lines == lines)
    check("%-28s -> %s" % (command, status), ok, "got %r %r" % (r.status, r.lines))
    return r


def coins(state):
    return [c["count"] for c in state["coins"]]


def stocks(state):
    return [p["stock"] for p in state["products"]]


def run(folder):
    b = Bridge(folder)
    print("Waiting for vending.exe /g ...")
    b.wait_ready()

    # --- the starting state ---
    s = b.state()
    check("STATE: 6 products, Chips 15 x3", len(s["products"]) == 6 and
          s["products"][0] == {"code": 1, "name": "Chips", "price": 15, "stock": 3}, str(s["products"][:1]))
    check("STATE: 6 coins (100 50 20 10 5 1), 10 each", [c["value"] for c in s["coins"]] == [100, 50, 20, 10, 5, 1]
          and coins(s) == [10] * 6, str(s["coins"]))
    check("STATE: no sales, not locked, no session, no purchase",
          (s["sales_count"], s["sales_total"], s["locked"], s["admin"], s["purchase_open"]) == (0, 0, 0, False, False))

    # --- a purchase with change ---
    expect(b, "BUY 1", "OK TX 1 15")
    check("STATE: purchase open on product 1", (lambda t: t["purchase_open"] and t["purchase_product"] == 1)(b.state()))
    expect(b, "BUY 2", "ERR busy")
    expect(b, "PAY 5", "OK SHORT 10")
    expect(b, "PAY 50", "OK SOLD", ["R Chips 15 55 40", "K 2 20"])
    s = b.state()
    check("STATE after the sale: Chips 2 left, 1 sale of 15",
          s["products"][0]["stock"] == 2 and (s["sales_count"], s["sales_total"]) == (1, 15))
    check("STATE after the sale: coins 10 11 8 10 11 10", coins(s) == [10, 11, 8, 10, 11, 10], str(coins(s)))
    check("STATE after the sale: purchase closed", not s["purchase_open"] and s["inserted"] == 0)

    # --- exact payment ---
    expect(b, "BUY 3", "OK TX 3 10")
    expect(b, "PAY 10", "OK SOLD", ["R Candy 10 10 0"])

    # --- errors and cancel ---
    expect(b, "PAY 5", "ERR notx")
    for bad in ("BUY 9", "BUY 0", "BUY abc", "BUY"):
        expect(b, bad, "ERR invalid")
    expect(b, "BUY 2", "OK TX 2 25")
    for bad in ("PAY 0", "PAY 1000", "PAY x"):
        expect(b, bad, "ERR amount")
    expect(b, "PAY 5", "OK SHORT 20")
    expect(b, "CANCEL", "OK REFUND 5")
    expect(b, "CANCEL", "OK REFUND 0")
    check("Soda stock unchanged after the cancel", b.state()["products"][1]["stock"] == 3)
    expect(b, "HELLO", "ERR unknown")

    # --- admin needs a login ---
    expect(b, "REFILL COINS", "ERR login")
    expect(b, "SETSTOCK 1 5", "ERR login")
    expect(b, "LOGIN 31415 1234", "OK")
    check("STATE: admin session open", b.state()["admin"])

    # --- admin commands ---
    expect(b, "SETSTOCK 3 7", "OK")
    check("Candy stock is 7", b.state()["products"][2]["stock"] == 7)
    expect(b, "SETSTOCK 9 1", "ERR invalid")
    expect(b, "SETSTOCK 1 5000", "ERR range")
    expect(b, "SETCOIN 20 4", "OK")
    check("20-coins are 4", coins(b.state())[2] == 4)
    expect(b, "SETCOIN 25 1", "ERR denom")
    expect(b, "SETCOIN 7 1", "ERR denom")

    # --- the machine lacks the coins for exact change ---
    for coin in (20, 10, 5, 1):
        expect(b, "SETCOIN %d 0" % coin, "OK")
    expect(b, "BUY 3", "OK TX 3 10")
    expect(b, "PAY 50", "OK NOCHANGE 50")
    s = b.state()
    check("No exact change: nothing sold, purchase closed", s["products"][2]["stock"] == 7 and not s["purchase_open"])

    # --- refills ---
    expect(b, "REFILL COINS", "OK")
    check("Coins are 10 each again", coins(b.state()) == [10] * 6)
    expect(b, "REFILL PRODUCTS", "OK")
    check("Products are 3 each again", stocks(b.state()) == [3] * 6)
    expect(b, "REFILL XYZ", "ERR invalid")

    # --- rules for the admin code and PIN (nothing is changed) ---
    expect(b, "SETCODE 5", "ERR range")
    expect(b, "SETPIN 12", "ERR range")
    expect(b, "SETPIN 12345", "ERR range")
    expect(b, "LOGOUT", "OK")
    expect(b, "REFILL COINS", "ERR login")

    # --- wrong PINs and the lockout (last, because it locks the admin for a minute) ---
    expect(b, "LOGIN 31415 1", "ERR WRONG 2")      # right code, wrong PIN
    expect(b, "LOGIN 1 1234", "ERR WRONG 1")      # wrong code, right PIN
    expect(b, "LOGIN 31415 3", "ERR LOCKED 60")
    r = b.command("LOGIN 31415 1234")
    check("LOGIN while locked -> ERR LOCKED n (1..60)", r.words[:2] == ["ERR", "LOCKED"] and 1 <= int(r.words[2]) <= 60, r.status)
    check("STATE: locked for 1..60 s", 1 <= b.state()["locked"] <= 60)

    expect(b, "QUIT", "OK")


if __name__ == "__main__":
    folder = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    try:
        run(folder)
    except BridgeError as error:
        print("STOPPED:", error)
        failed += 1
    print("\n%d passed, %d failed" % (passed, failed))
    sys.exit(0 if failed == 0 else 1)
