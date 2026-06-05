#!/usr/bin/env python3
"""Faithful platform-style runner: starts observer on a unix socket (PLAIN
protocol, mvcc trx) in a clean data dir, sends each SQL statement, prints the
response. Mirrors test/case/miniob_test.py wire protocol (SQL + '\\0', read
until trailing '\\0'; POLLHUP/POLLERR => server crash)."""
import os, sys, socket, subprocess, tempfile, time, select, shutil, signal

ROOT = "/mnt/c/Users/11463/Downloads/miniob-2023/miniob-2023-09fe80d885b23450fdb01d7b9276ad0f0b6151c0"
OBS = ROOT + "/build_debug/bin/observer"
CFG = ROOT + "/etc/observer.ini"

def split_sql(text):
    # Split on ';' but keep statements; ignore blank/comment lines.
    stmts, cur = [], ""
    for line in text.splitlines():
        s = line.strip()
        if not s or s.startswith("--") or s.startswith("#"):
            continue
        cur += " " + line if cur else line
        if s.endswith(";"):
            stmts.append(cur.strip())
            cur = ""
    if cur.strip():
        stmts.append(cur.strip())
    return stmts

def recv_resp(sock, timeout=10):
    poller = select.poll()
    poller.register(sock, select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR)
    result = ""
    while True:
        events = poller.poll(timeout * 1000)
        if not events:
            return "<TIMEOUT>"
        _, event = events[0]
        if event & (select.POLLHUP | select.POLLERR):
            return "<SERVER CRASH: POLLHUP/POLLERR>"
        data = sock.recv(65536)
        if len(data) > 0:
            result += data.decode("utf-8", errors="replace")
            if data[-1] == 0:
                return result[:-1].strip()
        else:
            return "<CONNECTION CLOSED>"

def main():
    sqlfile = sys.argv[1]
    with open(sqlfile) as f:
        stmts = split_sql(f.read())
    datadir = tempfile.mkdtemp(prefix="miniob_sock.")
    sockpath = os.path.join(datadir, "miniob.sock")
    proc = subprocess.Popen([OBS, "-s", sockpath, "-f", CFG, "-t", "mvcc"],
                            cwd=datadir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        # wait for socket
        for _ in range(100):
            if os.path.exists(sockpath):
                break
            if proc.poll() is not None:
                print("observer exited early code", proc.returncode); return
            time.sleep(0.05)
        sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        for _ in range(50):
            if sock.connect_ex(sockpath) == 0:
                break
            time.sleep(0.05)
        sock.setblocking(False)
        for stmt in stmts:
            print(stmt)
            if proc.poll() is not None:
                print("<SERVER ALREADY DEAD>"); break
            try:
                sock.sendall(stmt.encode("utf-8") + b"\0")
            except Exception as e:
                print("<SEND FAIL: %s>" % e); break
            print(recv_resp(sock))
        sock.close()
    finally:
        try: proc.send_signal(signal.SIGINT); proc.wait(timeout=5)
        except Exception:
            proc.kill()
        shutil.rmtree(datadir, ignore_errors=True)

if __name__ == "__main__":
    main()
