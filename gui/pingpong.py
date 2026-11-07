"""pingpong.py - handshake test for the GUI bridge.

Run pingpong.exe in DOSBox first (it creates CMD.TXT and OUT.TXT in the project
folder), then run this script from another terminal:

    py gui\\pingpong.py            (uses the project folder)
    py gui\\pingpong.py D:\\8086VendingMachine

It sends numbered records to CMD.TXT, waits for the matching reply in OUT.TXT,
prints how long each round trip took, and finally sends QUIT.
"""
import os
import sys
import time

REC_SIZE = 64          # every record is exactly this many bytes
TIMEOUT = 5.0          # seconds to wait for one reply


def send(cmd_path, seq, text):
    """Write 'SEQ|TEXT#' into CMD.TXT, padded to REC_SIZE, in place."""
    record = ("%d|%s#" % (seq, text)).encode("ascii")
    if len(record) > REC_SIZE - 8:
        raise ValueError("command too long for one record")
    record = record.ljust(REC_SIZE, b" ")
    with open(cmd_path, "r+b") as f:      # r+b: overwrite, never create or truncate
        f.write(record)
        f.flush()
        os.fsync(f.fileno())


def wait_reply(out_path, seq, timeout=TIMEOUT):
    """Wait until OUT.TXT holds the reply for 'seq'. Returns (text, seconds) or (None, None)."""
    start = time.perf_counter()
    while time.perf_counter() - start < timeout:
        try:
            with open(out_path, "rb") as f:
                data = f.read(REC_SIZE).decode("ascii", "replace")
        except OSError:
            data = ""
        if "#" in data:
            head, _, rest = data.partition("|")
            if head == str(seq):
                return rest.split("#")[0], time.perf_counter() - start
        time.sleep(0.005)
    return None, None


def main(folder, count=20):
    cmd_path = os.path.join(folder, "CMD.TXT")
    out_path = os.path.join(folder, "OUT.TXT")

    print("Waiting for pingpong.exe to create CMD.TXT and OUT.TXT in", folder)
    for _ in range(300):
        if os.path.exists(cmd_path) and os.path.exists(out_path):
            break
        time.sleep(0.1)
    else:
        print("The files did not appear. Is pingpong.exe running in DOSBox with this folder mounted?")
        return 1

    times, lost = [], 0
    for seq in range(1, count + 1):
        send(cmd_path, seq, "PING %d" % seq)
        text, secs = wait_reply(out_path, seq)
        if text is None:
            lost += 1
            print("%3d  no reply within %.0f s" % (seq, TIMEOUT))
        else:
            times.append(secs)
            print("%3d  %-14s %6.0f ms" % (seq, text, secs * 1000))

    if times:
        print("\nreplies: %d of %d   average %.0f ms   slowest %.0f ms"
              % (len(times), count, sum(times) / len(times) * 1000, max(times) * 1000))
    else:
        print("\nno replies at all")

    send(cmd_path, count + 1, "QUIT")
    text, _ = wait_reply(out_path, count + 1)
    print("QUIT answered:" if text else "QUIT not answered.", text or "")
    return 0 if lost == 0 else 1


if __name__ == "__main__":
    folder = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    sys.exit(main(folder))
