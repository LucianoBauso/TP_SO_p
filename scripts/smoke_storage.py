#!/usr/bin/env python3
import socket, struct, sys, time

def recvn(s, n):
    """Lee exactamente n bytes o retorna None si el peer cerró."""
    data = b""
    while len(data) < n:
        chunk = s.recv(n - len(data))
        if not chunk:
            return None
        data += chunk
    return data

def sendf(s, op, p=b""):
    s.sendall(struct.pack("!II", op, len(p)) + p)

def recvf(s, expect=None, label=""):
    h = recvn(s, 8)
    if h is None:
        print(f"[ERR] {label} header: conexión cerrada")
        sys.exit(1)
    op, l = struct.unpack("!II", h)
    p = recvn(s, l)
    if p is None:
        print(f"[ERR] {label} payload: conexión cerrada (faltaban {l} bytes)")
        sys.exit(1)
    if expect is not None and op != expect:
        print(f"[ERR] {label} opcode inesperado: got={op} expected={expect}")
        sys.exit(1)
    return op, p

host = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
port = int(sys.argv[2]) if len(sys.argv) > 2 else 9002
s = socket.create_connection((host, port))
s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

OP_HS, OP_GB, OP_CR, OP_TR, OP_TG, OP_CM, OP_WB, OP_RB, OP_DT = 1,2,3,4,5,6,7,8,9

# 1) HANDSHAKE
print("-> HANDSHAKE")
sendf(s, OP_HS, struct.pack("!I", 42))
op, p = recvf(s, expect=OP_HS, label="HS")
st, bs = struct.unpack("!II", p)
print(f"<- HS status={st} bs={bs}")
if st != 0: sys.exit(1)
print("[OK] Handshake")
time.sleep(0.05)

# 2) GET_BLOCK_SIZE
print("-> GET_BLOCK_SIZE")
sendf(s, OP_GB, b"")
op, p = recvf(s, expect=OP_GB, label="GB")
(bs2,) = struct.unpack("!I", p)
print(f"<- GB bs={bs2}")
print("[OK] GET_BLOCK_SIZE")
time.sleep(0.05)

# 3) READ_BLOCK(0)
print("-> READ_BLOCK(0)")
sendf(s, OP_RB, struct.pack("!I", 0))
op, p = recvf(s, expect=OP_RB, label="RB")
print(f"<- RB len={len(p)}")
if len(p) == bs and set(p) == {ord('X')}:
    print("[OK] READ_BLOCK mock content")
else:
    print("[WARN] READ_BLOCK contenido inesperado")
time.sleep(0.05)

# 4) Ops simples (status OK)
for opx, name in [(OP_CR, "CREATE"), (OP_TR, "TRUNCATE"), (OP_TG, "TAG"),
                  (OP_CM, "COMMIT"), (OP_WB, "WRITE_BLOCK"), (OP_DT, "DELETE_TAG")]:
    print(f"-> {name}")
    sendf(s, opx, b"dummy")
    op, p = recvf(s, expect=opx, label=name)
    (st,) = struct.unpack("!I", p)
    print(f"<- {name} status={st}")
    if st != 0:
        print(f"[ERR] {name} status={st}")
        sys.exit(1)
    print(f"[OK] {name}")
    time.sleep(0.05)

print("\nTodo OK: Storage Check 2 operativo.")
s.close()

