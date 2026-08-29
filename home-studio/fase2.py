# -*- coding: utf-8 -*-
import math
from iso3d import *
R2=math.sqrt(2)
K0=0.424; PASSO=0.05*R2; NC=4; BW=0.05; BT=0.025
CUT=[0.60,0.45,0.30,0.15]
U=(-1/R2,1/R2,0); N=(1/R2,1/R2,0)
WOOD="var(--wood)"; PET="var(--ray)"; WALL="var(--sunk)"; FLOOR="var(--rule)"
FAB="var(--felt)"; FLAG="var(--flag)"; MET="var(--mute)"
o=[]
CLIPS=[]
def clipped(faces,cid,x,y,w,h):
    CLIPS.append('<clipPath id="%s"><rect x="%.0f" y="%.0f" width="%.0f" height="%.0f"/></clipPath>'%(cid,x,y,w,h))
    return ['  <g clip-path="url(#%s)">'%cid]+faces+['  </g>']
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    o.append('  <text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    o.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def rc(x,y,w,h,fill,stroke="var(--ink)",sw="1",extra=""):
    o.append('  <rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(x,y,w,h,fill,stroke,sw,extra))
def pth(d,fill,stroke,sw="1",extra=""):
    o.append('  <path d="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(d,fill,stroke,sw,extra))
def tg(n,x,y,col="var(--ink)",r=12.5):
    o.append('  <circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.4"/>'%(x,y,r,col))
    o.append('  <text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def callout(p,dx,dy,lines,col="var(--ink)",anch="start"):
    if not p: return
    x2,y2=p[0]+dx,p[1]+dy
    li(p[0],p[1],x2,y2,col,"1.1")
    o.append('  <circle cx="%.1f" cy="%.1f" r="3.2" fill="%s"/>'%(p[0],p[1],col))
    for i,l in enumerate(lines):
        txt(x2+(6 if anch=="start" else -6),y2+4+i*14,l,"s-lbl" if i==0 else "s-dimt",anch,col if i==0 else None)

def walls(sc,ext=0.92,hh=1.42):
    sc.quad([(0,0,0),(0,0,hh),(ext,0,hh),(ext,0,0)],WALL,"var(--rule)",".8",1.0,True,0)
    sc.quad([(0,0,0),(0,ext,0),(0,ext,hh),(0,0,hh)],WALL,"var(--rule)",".8",1.0,True,0)
    sc.quad([(0,0,0),(ext,0,0),(ext,ext,0),(0,ext,0)],FLOOR,"var(--rule)",".8",1.0,True,0)
def petlayer(sc,i,shift=0.0,h=1.20,z0=0.0):
    k0=K0-i*PASSO; w=CUT[i]
    org=add(add((k0/2,k0/2,z0),mul(U,-w/2)),mul(N,shift))
    sc.box(org,mul(U,w),mul(N,-0.0707),(0,0,h),PET,"var(--ray)",".7")
def ladder(sc,shift=0.0,z0=0.0,h=1.20,covered=False):
    base=(K0+N[0]*shift,N[1]*shift,z0)
    if covered:
        sc.box(add(base,mul(N,-0.005)),mul(U,0.60),mul(N,BT*2+0.01),(0,0,h),FAB,"var(--ink)",".9")
        return
    for u0 in (0.0,0.60-BW):
        sc.box(add(base,mul(U,u0)),mul(U,BW),mul(N,BT),(0,0,h),WOOD,"var(--ink)",".8")
    for zz in (0.0,h/2-BW/2,h-BW):
        sc.box(add(add(base,mul(N,BT)),(0,0,zz)),mul(U,0.60),mul(N,BT),(0,0,BW),WOOD,"var(--ink)",".8")

# ======== VISTA 1 : A MANTA ENTRANDO ========
W1,H1=560,640
c1=Cam((2.35,1.40,2.45),(0.30,0.30,0.40),40,W1,H1); s1=Scene(c1,ox=40,oy=190)
walls(s1,ext=0.90,hh=1.40)
for i,sh in zip(range(NC),[0.60,0.42,0.24,0.0]):
    petlayer(s1,i,shift=sh)
o+=clipped(s1.out(),'c1',40,196,560,650)
for i,(sh,lab) in enumerate(zip([0.0,0.24,0.42,0.60],["4&#186;","3&#186;","2&#186;","1&#186;"])):
    pass
callout(s1.pt(add((K0/2,K0/2,1.24),mul(N,0.60))),70,-84,["1&#186; a mais estreita, 0,15","vai no fundo do v&#233;rtice"],PET)
callout(s1.pt((0.10,0.10,0.30)),-6,120,["4&#186; a de 0,60 fecha","por &#250;ltimo, na frente"],PET,"start")
txt(60,168,"1 &#183; A MANTA ENTRA DE DENTRO PARA FORA","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(60,186,"Sempre a mais estreita primeiro. Corte 1,5 cm a mais na","s-dimt","start")
txt(60,202,"largura, para ela comprimir contra as paredes.","s-dimt","start")

# ======== VISTA 2 : MONTADO, SEM MADEIRA A MOSTRA ========
W2,H2=470,700
c2=Cam((2.80,1.70,2.80),(0.28,0.28,0.98),36,W2,H2); s2=Scene(c2,ox=630,oy=170)
walls(s2,ext=0.92,hh=2.55)
for z0 in (0.0,1.20):
    for i in range(NC): petlayer(s2,i,z0=z0)
    ladder(s2,z0=z0,covered=True)
o+=clipped(s2.out(),'c2',628,166,478,710)
callout(s2.pt(add((K0,0.0,1.70),mul(N,0.06))),64,-30,["O tecido envelopa o quadro","e &#233; grampeado ATR&#193;S.","Nenhuma madeira aparece."],FAB)
txt(650,150,"2 &#183; COMO FICA","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ======== VISTA 3 : CORTE DO ESTOFAMENTO ========
BX=1150.0; BY=250.0; SC=1500.0
txt(BX-40,168,"3 &#183; O ENVELOPE DO TECIDO &#183; corte pela borda do quadro","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(BX-40,186,"O tecido d&#225; a volta na borda e morre na face de tr&#225;s, onde ningu&#233;m v&#234;.","s-dimt","start")
mx,my=BX+60,BY+90
rc(mx,my,SC*0.05,SC*0.025,WOOD,"var(--ink)","1.6")                  # montante
rc(mx,my-SC*0.025,SC*0.05,SC*0.025,WOOD,"var(--ink)","1.6")         # travessa
rc(mx+SC*0.05,my-SC*0.02,SC*0.16,SC*0.062,PET,"var(--ray)","1.4")   # manta atras
pth("M %.1f %.1f L %.1f %.1f Q %.1f %.1f %.1f %.1f L %.1f %.1f Q %.1f %.1f %.1f %.1f L %.1f %.1f"%(
    mx-SC*0.10, my-SC*0.05,
    mx-SC*0.004, my-SC*0.05,
    mx-SC*0.016, my-SC*0.05, mx-SC*0.016, my-SC*0.038,
    mx-SC*0.016, my+SC*0.014,
    mx-SC*0.016, my+SC*0.026, mx-SC*0.004, my+SC*0.026,
    mx+SC*0.055, my+SC*0.026), "none", FAB, "3.4")
for i,(px,py) in enumerate([(mx+SC*0.020,my+SC*0.021),(mx+SC*0.042,my+SC*0.021)]):
    li(px-5,py-9,px-5,py+3,FLAG,"2.4"); li(px+5,py-9,px+5,py+3,FLAG,"2.4"); li(px-5,py-9,px+5,py-9,FLAG,"2.4")
tg("T",mx-SC*0.055,my-SC*0.062,FAB,11)
tg("A",mx+SC*0.025,my+SC*0.012,WOOD,11)
tg("G",mx+SC*0.031,my+SC*0.046,FLAG,11)
txt(mx-SC*0.10,my-SC*0.085,"frente &#183; olha para a sala","s-dimt","start")
txt(mx+SC*0.075,my-SC*0.052,"atr&#225;s &#183; olha para o canto","s-dimt","start")
txt(BX-40,BY+SC*0.125,"T  tecido, cortado 10 cm maior em cada dire&#231;&#227;o","s-dimt","start")
txt(BX-40,BY+SC*0.125+17,"A  quadro de ripa","s-dimt","start")
txt(BX-40,BY+SC*0.125+34,"G  grampos a cada 8 cm, s&#243; na face de tr&#225;s","s-dimt","start")

# ======== VISTA 4 : FIXACAO ESCONDIDA ========
W4,H4=440,322
c4=Cam((1.95,1.42,1.35),(0.26,0.24,0.42),34,W4,H4); s4=Scene(c4,ox=1150,oy=556)
s4.quad([(0,0,0),(0,0,0.78),(0.70,0,0.78),(0.70,0,0)],WALL,"var(--rule)",".8",1.0,True,0)
s4.quad([(0,0,0),(0,0.70,0),(0,0.70,0.78),(0,0,0.78)],WALL,"var(--rule)",".8",1.0,True,0)
s4.quad([(0,0,0),(0.70,0,0),(0.70,0.70,0),(0,0.70,0)],FLOOR,"var(--rule)",".8",1.0,True,0)
for i in range(NC): petlayer(s4,i,h=0.78)
ladder(s4,h=0.78)
# fita perfurada: da parede ate a face de tras do montante
s4.box((K0-0.11,0.0,0.44),(0.11,0,0),(0,0.02,0),(0,0,0.024),MET,"var(--ink)",".8")
s4.box((K0-0.006,0.0,0.44),(0.032,0.032,0),(0,0.02,0),(0,0,0.024),MET,"var(--ink)",".8")
o+=clipped(s4.out(),'c4',1148,552,444,322)
callout(s4.pt((K0-0.055,0.01,0.465)),176,-112,["Fita perfurada, dobrada &#224; m&#227;o","um parafuso na parede,","outro no montante &#8212; tudo","dentro da cavidade"],MET,"start")
txt(1150,540,"4 &#183; A FIXA&#199;&#195;O QUE N&#195;O APARECE","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ======== LISTA ========
LX=1640.0; yy=200.0
def head(t,col="var(--ink)"):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+340,yy,"var(--mute)","1"); yy+=24
def row(k,v,col=None):
    global yy
    txt(LX,yy,k,"s-dimt","start"); txt(LX+340,yy,v,"s-dimt","end",col); yy+=19
head("O QUE COMPRAR PARA A FASE 2")
row("Tecido ac&#250;stico escuro","3,5 m")
row("(rolo de 1,40 &#183; 0,80 por m&#243;dulo)","4,48 m&#178;")
row("Grampos 6 a 8 mm","180")
row("Fita perfurada","2 m")
row("Parafuso + bucha n&#186; 6","16")
yy+=8
for l in ["Se n&#227;o achar fita perfurada: qualquer chapinha","met&#225;lica furada que dobre com a m&#227;o serve."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=22
head("O CORTE DA MANTA","var(--ray)")
for a,b in [("Camada 1 &#183; corte em","0,615 m"),("Camada 2 &#183; corte em","0,465 m"),
            ("Camada 3 &#183; corte em","0,315 m"),("Camada 4 &#183; corte em","0,165 m")]:
    row(a,b)
yy+=6
for l in ["Cada camada = 2 placas de 1,20 empilhadas.","Estilete de l&#226;mina nova e uma r&#233;gua longa.","O 1,5 cm a mais &#233; de prop&#243;sito: a manta","comprime e n&#227;o deixa fresta na parede."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=22
head("A ORDEM DA FASE 2","var(--flag)")
for i,l in enumerate(["Cortar as 4 larguras de manta","Empilhar no canto, da mais estreita para fora",
                      "Estofar as escadas na bancada","Encaixar cada m&#243;dulo, de baixo para cima",
                      "Prender as fitas dentro da cavidade","Tala C nas costas, alinhando os dois m&#243;dulos"],1):
    tg(str(i),LX+11,yy-4,"var(--flag)",10.5); txt(LX+30,yy,l,"s-dimt","start"); yy+=23

txt(60,54,"MANUAL DE MONTAGEM &#183; FASE 2 &#183; MANTA, TECIDO E FIXA&#199;&#195;O","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(60,74,"Sem nenhuma madeira &#224; vista: o tecido envelopa o quadro e a fixa&#231;&#227;o mora dentro da cavidade.","s-dimt","start")
open("_fase2.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 2060 1000">\n<defs>'+"".join(CLIPS)+'</defs>\n'+"\n".join(o)+'\n</svg>')
print("ok")
