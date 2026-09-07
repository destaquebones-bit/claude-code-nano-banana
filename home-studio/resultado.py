# -*- coding: utf-8 -*-
"""Prancha de resultado da etapa 1: RT60 por banda e sobreposicao L/R."""
import numpy as np, math, mdat
from scipy.signal import butter, sosfiltfilt

F="/root/.claude/uploads/b450115a-a6a6-5c6b-a90c-968d275bdc05/ee528f61-with_bass_trap_4_L.mdat"
b=open(F,'rb').read(); ar=mdat.arrays(b)
n=54532; df=48000/131072.0; f=np.arange(n)*df; fs=48000.0
def spl(pos): return [v for o,l,v in ar if l==n and o>pos and 20<v.min()<48 and 88<v.max()<112][0]
def irb(pos): return [(o,v) for o,l,v in ar if l==131072 and o<pos][-1][1]
def sm(y,fr=1/6):
    o=np.empty_like(y)
    for i in range(n):
        if f[i]<=0: o[i]=y[i]; continue
        a,bb=np.searchsorted(f,f[i]*2**(-fr/2)),np.searchsorted(f,f[i]*2**(fr/2))
        o[i]=y[max(0,a):max(a+1,bb)].mean()
    return o
ref=(f>=400)&(f<4000)
SL=sm(spl(11_529_342)); SR=sm(spl(10_071_331))
CL=SL-SL[ref].mean(); CR=SR-SR[ref].mean()
def rt(ir,f0,f1):
    ir=np.asarray(ir,float); p=int(np.argmax(np.abs(ir))); x=ir[max(0,p-int(.005*fs)):]
    sos=butter(4,[f0/(fs/2),min(f1/(fs/2),.99)],btype='band',output='sos')
    e=sosfiltfilt(sos,x)**2; nf=e[int(.75*len(e)):].mean(); idx=np.where(e>nf*10)[0]
    if len(idx)<100: return None
    s=np.cumsum(e[:idx[-1]][::-1])[::-1]; L=10*np.log10(s/s[0]+1e-30)
    t=np.arange(len(L))/fs; m=(L<=-5)&(L>=-25)
    return None if m.sum()<50 else -60.0/np.polyfit(t[m],L[m],1)[0]
BANDS=[(45,90,"63"),(90,180,"125"),(180,355,"250"),(355,710,"500"),(710,1420,"1k"),(1420,2840,"2k"),(2840,5680,"4k")]
irL,irR=irb(11_529_342),irb(10_071_331)
RT=[(nm,rt(irL,a,c),rt(irR,a,c)) for a,c,nm in BANDS]

BLUE="#1670b0"; ORANGE="#c2703a"; INK="#1a1a18"; MUTE="#8a8579"; RULE="#ddd9cd"
o=[]
def a_(s): o.append("  "+s)
def txt(x,y,s,cls="s-dimt",an="start",fill=None,ex=""):
    fl=' fill="%s"'%fill if fill else ""
    a_(f'<text class="{cls}" x="{x:.1f}" y="{y:.1f}" text-anchor="{an}"{fl}{ex}>{s}</text>')
def line(x1,y1,x2,y2,c,w="1",ex=""):
    a_(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{c}" stroke-width="{w}"{ex}/>')
def rect(x,y,w,h,fill,rx=0):
    a_(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{rx}" fill="{fill}"/>')

txt(70,58,"ETAPA 1 &#183; O QUE A MEDI&#199;&#195;O DECIDIU","s-dimt","start",INK,' letter-spacing="1.6"')
txt(70,80,"L e R no mesmo ponto, mesma sess&#227;o, 512k &#215; 8 varreduras, SoundID em bypass","s-dimt","start")

# ---------- HERO ----------
rect(70,112,560,96,"#f0eee7",4)
txt(94,166,"1,84","s-hero","start",ORANGE)
txt(232,166,"s","s-leg","start",ORANGE)
txt(280,146,"o 63 Hz decai em quase dois segundos.","s-leg","start",INK)
txt(280,170,"Acima de 250 Hz, tudo abaixo de 0,47 s.","s-dimt","start")
txt(280,190,"&#201; o &#250;nico problema que sobrou na sala.","s-dimt","start",ORANGE)

# ---------- A · RT60 ----------
AX,AY,AW,AH=100,300,500,330
txt(70,266,"A &#183; RT60 POR BANDA DE OITAVA","s-dimt","start",INK,' letter-spacing="1.5"')
mx=2.0
for v in [0,0.5,1.0,1.5,2.0]:
    y=AY+AH-(v/mx)*AH
    line(AX,y,AX+AW,y,RULE,"1")
    txt(AX-10,y+4,f"{v:.1f}","s-dimt","end",MUTE)
txt(AX-10,AY-14,"s","s-dimt","end",MUTE)
bw=AW/len(RT); barw=(bw-16)/2-1
for i,(nm,l,r) in enumerate(RT):
    x0=AX+i*bw+8
    for j,(v,col) in enumerate([(l,BLUE),(r,ORANGE)]):
        if v is None: continue
        h=(v/mx)*AH; x=x0+j*(barw+2)
        a_(f'<path d="M {x:.1f} {AY+AH:.1f} L {x:.1f} {AY+AH-h+4:.1f} Q {x:.1f} {AY+AH-h:.1f} {x+4:.1f} {AY+AH-h:.1f} L {x+barw-4:.1f} {AY+AH-h:.1f} Q {x+barw:.1f} {AY+AH-h:.1f} {x+barw:.1f} {AY+AH-h+4:.1f} L {x+barw:.1f} {AY+AH:.1f} Z" fill="{col}"/>')
    txt(x0+barw+1,AY+AH+20,nm,"s-dimt","middle",MUTE)
    if nm=="63":
        txt(x0+barw*0.5,AY+AH-(l/mx)*AH-10,f"{l:.2f}","s-leg","middle",BLUE)
        txt(x0+barw*1.5+2,AY+AH-(r/mx)*AH-10,f"{r:.2f}","s-leg","middle",ORANGE)
line(AX,AY+AH,AX+AW,AY+AH,MUTE,"1.4")
txt(AX+AW/2,AY+AH+42,"Hz","s-dimt","middle",MUTE)
# faixa alvo
ya=AY+AH-(0.4/mx)*AH; yb=AY+AH-(0.3/mx)*AH
rect(AX,yb,AW,ya-yb,ORANGE+"22")
line(AX,ya,AX+AW,ya,ORANGE,"1",' stroke-dasharray="5 4"')

# legenda
lx=AX+AW-150
for k,(nm,col) in enumerate([("monitor L",BLUE),("monitor R",ORANGE)]):
    rect(lx,AY+6+k*20,11,11,col,2); txt(lx+17,AY+16+k*20,nm,"s-dimt","start")
line(lx,AY+52,lx+11,AY+52,ORANGE,"1.4",' stroke-dasharray="4 3"')
txt(lx+17,AY+56,"alvo: 0,3 a 0,4 s","s-dimt","start",ORANGE)

# ---------- B · L vs R ----------
BX,BY,BW,BH=760,300,780,330
txt(730,266,"B &#183; RESPOSTA EM FREQU&#202;NCIA &#183; L SOBRE R","s-dimt","start",INK,' letter-spacing="1.5"')
f0,f1=20.0,500.0; ymin,ymax=-26.0,12.0
def X(hz): return BX+BW*(math.log10(hz)-math.log10(f0))/(math.log10(f1)-math.log10(f0))
def Y(db): return BY+BH*(ymax-db)/(ymax-ymin)
for db in range(-25,11,5):
    line(BX,Y(db),BX+BW,Y(db),RULE if db else MUTE,"1" if db else "1.2")
    txt(BX-10,Y(db)+4,f"{db:+d}" if db else "0","s-dimt","end",MUTE)
txt(BX-10,BY-14,"dB","s-dimt","end",MUTE)
for hz in [20,30,50,63,100,200,300,500]:
    line(X(hz),BY,X(hz),BY+BH,RULE,"1")
    txt(X(hz),BY+BH+20,str(hz),"s-dimt","middle",MUTE)
txt(BX+BW/2,BY+BH+42,"Hz","s-dimt","middle",MUTE)
# faixa do 63 Hz
line(X(63),BY,X(63),BY+BH,ORANGE,"1.4",' stroke-dasharray="6 5"')
txt(X(63)+7,BY+44,"63 Hz","s-leg","start",ORANGE)
txt(X(63)+7,BY+60,"os dois modos da sala","s-dimt","start",ORANGE)
def path(C,col):
    pts=[]
    hz=f0
    while hz<=f1:
        i=int(round(hz/df)); pts.append(f"{X(hz):.1f},{Y(max(ymin,min(ymax,C[i]))):.1f}"); hz*=2**(1/48)
    a_(f'<polyline points="{" ".join(pts)}" fill="none" stroke="{col}" stroke-width="2" stroke-linejoin="round" stroke-linecap="round"/>')
path(CR,ORANGE); path(CL,BLUE)
iL=int(round(430/df)); iR=int(round(430/df))
txt(X(455),Y(CL[iL])-8,"L","s-leg","start",BLUE)
txt(X(455),Y(CR[iR])+16,"R","s-leg","start",ORANGE)
txt(BX+BW,BY-14,"normalizado pela m&#233;dia de 400 a 4000 Hz de cada curva","s-dimt","end",MUTE)

# ---------- C · conclusoes ----------
CXX=100; yy=700
txt(70,676,"O QUE MUDA NO PROJETO","s-dimt","start",ORANGE,' letter-spacing="1.5"')
items=[("N&#195;O existe assimetria entre os monitores.",
        "L e R andam juntos: desvio m&#233;dio de 2,8 dB entre 30 e 400 Hz, e n&#237;vel de banda m&#233;dia a 0,4 dB um do outro."),
       ("O buraco de 15 dB era o microfone fora do lugar.",
        "Some quando o mic vai para o ponto certo. A porta e o monitor esquerdo est&#227;o inocentes."),
       ("Os traps de canto j&#225; fizeram o que podiam.",
        "De 250 Hz para cima a sala est&#225; entre 0,24 e 0,47 s. N&#227;o precisa de mais nada nessa faixa."),
       ("Falta atacar 63 Hz &#8212; e s&#243; isso.",
        "Largura 2,75 m d&#225; 62,4 Hz; altura 2,65 m d&#225; 64,7 Hz. Dois modos empilhados. &#201; a membrana.")]
for i,(t1,t2) in enumerate(items):
    x=70+i*400
    a_(f'<circle cx="{x+9:.0f}" cy="{yy-4:.0f}" r="9" fill="{ORANGE}"/>')
    a_(f'<text class="s-tag" x="{x+9:.0f}" y="{yy:.0f}" text-anchor="middle" fill="#faf9f5">{i+1}</text>')
    txt(x+26,yy,t1,"s-leg","start",INK)
    words=t2.split(); lines=[]; cur=""
    for w in words:
        if len(cur)+len(w)>46: lines.append(cur); cur=w
        else: cur=(cur+" "+w).strip()
    lines.append(cur)
    for k,l in enumerate(lines): txt(x+26,yy+20+k*16,l,"s-dimt","start")

open("_resultado.svg","w").write(
 '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1700 810">\n'
 '<style>.s-hero{font-family:"IBM Plex Mono",monospace;font-size:54px;font-weight:600}</style>\n'
 +"\n".join(o)+'\n</svg>')
print("ok  RT60:", [(nm, None if l is None else round(l,2), None if r is None else round(r,2)) for nm,l,r in RT])
