"""bridge.py - talks to vending.exe running in bridge mode (vending.exe /g).

The two programs share the project folder:
  CMD.TXT  one record of 64 bytes   "SEQ|COMMAND args#"        (this script writes it)
  OUT.TXT  one record of 1024 bytes "SEQ|status", data lines, "#"  (vending.exe writes it)
Both files are created by vending.exe when it starts and are only overwritten in
place here, so DOSBox always sees them. Every command has a new sequence number
and the reply repeats it. The commands are listed in README.md.
"""
import os
import time

CMD_SIZE = 64          # size of the command record
OUT_SIZE = 1024        # size of the reply record


class BridgeError(Exception):
    """The program did not answer, or the answer could not be read."""


class Reply:
    """One reply. 'ok' is True for OK..., 'status' is the first line, 'lines' the data lines."""

    def __init__(self, seq, status, lines):
        self.seq = seq
        self.status = status
        self.lines = lines
        self.ok = status.startswith("OK")

    @property
    def words(self):
        return self.status.split()

    def __repr__(self):
        return "Reply(%d, %r, %r)" % (self.seq, self.status, self.lines)


def parse_reply(text):
    """Turn the text of OUT.TXT into a Reply, or None if it is not a complete reply."""
    if "#" not in text:
        return None
    seq, bar, rest = text.partition("|")
    if not bar or not seq.isdigit():
        return None
    body = rest.split("#")[0]
    lines = [line.strip() for line in body.replace("\r", "").split("\n")]
    lines = [line for line in lines if line]
    if not lines:
        return None
    return Reply(int(seq), lines[0], lines[1:])


def parse_state(reply):
    """Turn the reply of STATE into a dictionary."""
    state = {"products": [], "coins": [], "sales_count": 0, "sales_total": 0,
             "locked": 0, "admin": False, "purchase_open": False,
             "purchase_product": 0, "inserted": 0}
    for line in reply.lines:
        part = line.split()
        if part[0] == "P":
            state["products"].append({"code": int(part[1]), "name": part[2],
                                      "price": int(part[3]), "stock": int(part[4])})
        elif part[0] == "C":
            state["coins"].append({"value": int(part[1]), "count": int(part[2])})
        elif part[0] == "S":
            state["sales_count"], state["sales_total"] = int(part[1]), int(part[2])
        elif part[0] == "L":
            state["locked"] = int(part[1])
        elif part[0] == "A":
            state["admin"] = part[1] == "1"
        elif part[0] == "X":
            state["purchase_open"] = part[1] == "1"
            state["purchase_product"] = int(part[2])
            state["inserted"] = int(part[3])
    return state


class Bridge:
    def __init__(self, folder, timeout=5.0):
        self.cmd_path = os.path.join(folder, "CMD.TXT")
        self.out_path = os.path.join(folder, "OUT.TXT")
        self.timeout = timeout
        self.seq = 0

    def _read_out(self):
        try:
            with open(self.out_path, "rb") as f:
                return f.read(OUT_SIZE).decode("ascii", "replace")
        except OSError:
            return ""

    def wait_ready(self, timeout=30.0):
        """Wait until vending.exe /g has created fresh CMD.TXT and OUT.TXT."""
        end = time.time() + timeout
        while time.time() < end:
            if os.path.exists(self.cmd_path) and self._read_out().startswith("0|READY#"):
                self.seq = 0
                return
            time.sleep(0.1)
        raise BridgeError("vending.exe /g did not start in %s" % os.path.dirname(self.cmd_path))

    def command(self, text):
        """Send one command and return its Reply."""
        self.seq += 1
        record = ("%d|%s#" % (self.seq, text)).encode("ascii")
        if len(record) > CMD_SIZE - 8:
            raise ValueError("command too long for one record")
        record = record.ljust(CMD_SIZE, b" ")
        with open(self.cmd_path, "r+b") as f:        # r+b: overwrite, never create or truncate
            f.write(record)
            f.flush()
            os.fsync(f.fileno())
        end = time.time() + self.timeout
        while time.time() < end:
            reply = parse_reply(self._read_out())
            if reply is not None and reply.seq == self.seq:
                return reply
            time.sleep(0.005)
        raise BridgeError("no reply to %r within %.0f s" % (text, self.timeout))

    def state(self):
        return parse_state(self.command("STATE"))
