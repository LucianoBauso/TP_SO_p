#!/usr/bin/env python3
import socket, struct, sys
def sendf(s,op,p=b""): s.sendall(struct.pack("!II", op, len(p))+p)
def recvf(s):
  h=s.recv(8); 
  if len(h)<8: raise SystemExit("corte de conexión")
  op,l=struct.unpack("!II",h); d=b""
  while len(d)<l: 
    c=s.recv(l-len(d)); 
    if not c: raise SystemExit("payload incompleto")
    d+=c
  return op,d
host=sys.argv[1] if len(sys.argv)>1 else "127.0.0.1"; port=int(sys.argv[2]) if len(sys.argv)>2 else 9002
s=socket.create_connection((host,port))
OP_HS,OP_GB,OP_CR,OP_TR,OP_TG,OP_CM,OP_WB,OP_RB,OP_DT=1,2,3,4,5,6,7,8,9
# Handshake
sendf(s,OP_HS,struct.pack("!I",42)); op,p=recvf(s); st,bs=struct.unpack("!II",p); assert op==OP_HS and st==0; print("[OK] Handshake. BLOCK_SIZE=",bs)
# Block size
sendf(s,OP_GB); op,p=recvf(s); assert op==OP_GB and len(p)==4; (bs2,)=struct.unpack("!I",p); assert bs2==bs; print("[OK] GET_BLOCK_SIZE=",bs2)
# Read mock
sendf(s,OP_RB,struct.pack("!I",0)); op,p=recvf(s); assert op==OP_RB and len(p)==bs and set(p)=={ord('X')}; print("[OK] READ_BLOCK len=",len(p))
# Ops mock con status OK
for op,name in [(OP_CR,"CREATE"),(OP_TR,"TRUNCATE"),(OP_TG,"TAG"),(OP_CM,"COMMIT"),(OP_WB,"WRITE_BLOCK"),(OP_DT,"DELETE_TAG")]:
  sendf(s,op,b"dummy"); op2,p=recvf(s); assert op2==op and len(p)==4 and struct.unpack("!I",p)[0]==0; print("[OK]",name)
s.close(); print("\nTodo OK: Storage Check 2 operativo.")
