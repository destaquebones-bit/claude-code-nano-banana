#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Leitor de arquivos .mdat do REW (serializacao Java). Extrai nomes de
medicoes e os vetores de float (SPL, fase, resposta ao impulso)."""
import struct, sys, re
import numpy as np

def strings(b, minlen=3):
    """Strings UTF do fluxo Java: 0x74 + uint16 tamanho + bytes"""
    out=[]; i=0; n=len(b)
    while True:
        i=b.find(b'\x74', i)
        if i<0 or i+3>n: break
        ln=struct.unpack_from('>H', b, i+1)[0]
        if minlen<=ln<=300 and i+3+ln<=n:
            try:
                s=b[i+3:i+3+ln].decode('utf-8')
                if s.isprintable(): out.append((i,s))
            except UnicodeDecodeError: pass
        i+=1
    return out

def arrays(b, minlen=256):
    """Arrays de float: TC_ARRAY(0x75) + TC_REFERENCE(0x71) + handle + len"""
    out=[]; i=0; n=len(b)
    while True:
        i=b.find(b'\x75\x71', i)
        if i<0: break
        if i+10>n: break
        ln=struct.unpack_from('>i', b, i+6)[0]
        if minlen<=ln<=(n-i-10)//4:
            a=np.frombuffer(b, dtype='>f4', count=ln, offset=i+10)
            if np.isfinite(a).all() and np.abs(a).max()<1e12:
                out.append((i,ln,a.astype(np.float64)))
                i+=10+4*ln; continue
        i+=2
    return out

if __name__=="__main__":
    b=open(sys.argv[1],'rb').read()
    print(f"{len(b):,} bytes\n")
    ss=strings(b)
    vistos=set()
    print("--- textos relevantes ---")
    for off,s in ss:
        if s in vistos: continue
        if re.search(r'(Sep|Set|dBFS|Hz|sweep|L |R |noise|Left|Right|mic|MiniFuse|48|512)', s) and len(s)<120:
            vistos.add(s); print(f"  @{off:>9,}  {s}")
    aa=arrays(b)
    print(f"\n--- {len(aa)} vetores de float ---")
    from collections import Counter
    for ln,c in Counter(a[1] for a in aa).most_common(12):
        print(f"  {c:>3} x {ln:,} pontos")
