# -*- coding: utf-8 -*-
import math
from iso3d import *
R2=math.sqrt(2)
K0=0.424; PASSO=0.05*R2; NC=4; BW=0.05; BT=0.025
CUT=[0.60,0.45,0.30,0.15]
U=(-1/R2,1/R2,0); N=(1/R2,1/R2,0)
WOOD="var(--wood)"; PET="var(--ray)"; WALL="var(--sunk)"; FLOOR="var(--rule)"; FAB="var(--felt)"; FLAG="var(--flag)"
o=[]
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    o.append('  <text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    o.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def tg(n,x,y,col="var(--ink)",r=13):
    o.append('  <circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.4"/>'%(x,y,r,col))
    o.append('  <text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def callout(p,dx,dy,lines,col="var(--ink)",anch="start"):
    if not p: return
    x2,y2=p[0]+dx,p[1]+dy
    li(p[0],p[1],x2,y2,col,"1.1")
    o.append('  <circle cx="%.1f" cy="%.1f" r="3.2" fill="%s"/>'%(p[0],p[1],col))
    for i,l in enumerate(lines):
        txt(x2+(6 if anch=="start" else -6), y2+4+i*14, l, "s-lbl" if i==0 else "s-dimt", anch, col if i==0 else None)

def walls(sc,ext=0.92,hh=1.42):
    sc.quad([(0,0,0),(0,0,hh),(ext,0,hh),(ext,0,0)], WALL,"var(--rule)",".8",1.0,True,0)
    sc.quad([(0,0,0),(0,ext,0),(0,ext,hh),(0,0,hh)], WALL,"var(--rule)",".8",1.0,True,0)
    sc.quad([(0,0,0),(ext,0,0),(ext,ext,0),(0,ext,0)], FLOOR,"var(--rule)",".8",1.0,True,0)
def petlayer(sc,i,shift=0.0,h=1.20,z0=0.0):
    k0=K0-i*PASSO; w=CUT[i]
    org=add(add((k0/2,k0/2,z0), mul(U,-w/2)), mul(N,shift))
    sc.box(org, mul(U,w), mul(N,-0.0707), (0,0,h), PET,"var(--ray)",".7")
def ladder(sc,shift=0.0,z0=0.0,h=1.20):
    base=(K0+N[0]*shift, N[1]*shift, z0)
    for u0 in (0.0,0.60-BW):
        sc.box(add(base,mul(U,u0)), mul(U,BW), mul(N,BT), (0,0,h), WOOD,"var(--ink)",".8")
    for zz in (0.0, h/2-BW/2, h-BW):
        sc.box(add(add(base,mul(N,BT)),(0,0,zz)), mul(U,0.60), mul(N,BT), (0,0,BW), WOOD,"var(--ink)",".8")
def fabric(sc,shift,z0=0.0,h=1.20):
    base=(K0+N[0]*shift, N[1]*shift, z0)
    sc.quad([base, add(base,mul(U,0.60)), add(add(base,mul(U,0.60)),(0,0,h)), add(base,(0,0,h))],
            FAB,"var(--ink)",".9",0.62)
def block(sc,z0,side=0):
    if side==0: sc.box((K0+0.045,0,z0),(0.13,0,0),(0,BT,0),(0,0,0.06), FLAG,"var(--ink)",".8")
    else:       sc.box((0,K0+0.045,z0),(0,0.13,0),(BT,0,0),(0,0,0.06), FLAG,"var(--ink)",".8")

EYE=(1.75,1.00,2.05); TGT=(0.24,0.24,0.42)
# ================= 1 EXPLODIDA =================
W1,H1=580,660
c1=Cam((2.45,1.45,2.55),(0.34,0.34,0.42),40,W1,H1); s1=Scene(c1,ox=40,oy=190)
walls(s1,ext=0.92,hh=1.42)
SH=[0.62,0.46,0.30,0.14]
for i in range(NC): petlayer(s1,i,shift=SH[i])
ladder(s1,shift=0.92)
o+=s1.out()
callout(s1.pt(add((K0/2,K0/2,1.22), mul(N,0.50))), 96,-96,["1  As quatro camadas de manta","0,60 / 0,45 / 0,30 / 0,15","empurradas para o canto, soltas"],PET)
callout(s1.pt(add((K0,0.0,0.30), mul(N,0.94))), 40,86,["2  A escada","entra por &#250;ltimo e prende tudo"],WOOD)
txt(60,168,"1 &#183; EXPLODIDA &#183; a ordem de montagem","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ================= 2 MONTADA =================
W2,H2=560,760
c2=Cam((3.05,1.85,3.05),(0.30,0.30,1.05),36,W2,H2); s2=Scene(c2,ox=640,oy=140)
walls(s2,ext=0.95,hh=2.65)
for z0 in (0.0,1.20):
    for i in range(NC): petlayer(s2,i,z0=z0)
    ladder(s2,z0=z0)
for z in (0.30,1.00,1.70,2.20): block(s2,z,0); block(s2,z,1)
o+=s2.out()
callout(s2.pt((K0+0.10,0.0,1.00)), 62,20,["3  Tarugos de trava","parafusados na parede,","um de cada lado"],FLAG)
callout(s2.pt(add((K0,0.0,1.20),mul(N,0.05))), -132,-56,["4  Emenda dos dois m&#243;dulos","1,20 + 1,20 = 2,40"],WOOD,"end")
txt(660,120,"2 &#183; MONTADA &#183; os dois m&#243;dulos no canto","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ================= 3 EM PLANTA / CORTE =================
W3,H3=470,380
c3=Cam((0.86,0.78,1.95),(0.24,0.24,0.06),40,W3,H3); s3=Scene(c3,ox=1300,oy=200)
s3.quad([(0,0,0),(0.62,0,0),(0.62,0.62,0),(0,0.62,0)], FLOOR,"var(--rule)",".8",1.0,True,0)
s3.quad([(0,0,0),(0,0,0.20),(0.62,0,0.20),(0.62,0,0)], WALL,"var(--rule)",".8",1.0,True,0)
s3.quad([(0,0,0),(0,0.62,0),(0,0.62,0.20),(0,0,0.20)], WALL,"var(--rule)",".8",1.0,True,0)
for i in range(NC): petlayer(s3,i,h=0.16)
ladder(s3,h=0.16)
block(s3,0.02,0); block(s3,0.02,1)
o+=s3.out()
txt(1300,185,"3 &#183; VISTO DE CIMA &#183; o degrau das camadas","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ================= LEGENDA =================
LX=1258.0; yy=600.0
def head(t,col="var(--ink)"):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+430,yy,"var(--mute)","1"); yy+=24
head("O QUE SEGURA A MANTA")
for n,c_,t1,t2 in [("1",PET,"Nada segura &#8212; ela fica contida","duas paredes, o piso e a escada fecham uma caixa"),
                   ("2",WOOD,"A escada entra depois da manta","empurra o empilhamento contra o canto"),
                   ("3",FLAG,"Tarugos de trava na parede","impedem a escada de cair para dentro da sala"),
                   ("4",WOOD,"Tala C emenda os dois m&#243;dulos","parafusada nos dois montantes, por fora")]:
    tg(n,LX+12,yy-4,c_,11.5); txt(LX+32,yy,t1,"s-leg","start","var(--ink)")
    txt(LX+32,yy+15,t2,"s-dimt","start"); yy+=42
yy+=8
for l in ["A manta pesa 1,25 kg por trap. Ela n&#227;o cai, n&#227;o escorrega","e n&#227;o precisa de cola, tela nem grampo: ela senta no piso,","encosta nas duas paredes e a escada fecha a frente.","",
          "O tecido vem por &#250;ltimo, grampeado na escada j&#225; no lugar."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16

txt(60,54,"COMO A MANTA FICA PRESA &#183; TRAP DE CANTO EM 3D","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(60,74,"Sem cola e sem tela. O canto &#233; a caixa; a escada &#233; a tampa.","s-dimt","start")
open("_trap3d.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1790 980">\n'+"\n".join(o)+'\n</svg>')
print("ok")
