# -*- coding: utf-8 -*-
import math
from iso3d import *
R2=math.sqrt(2)
CAT=0.424; HIP=0.60; HM=1.20; BW=0.05; BT=0.025; CH=0.008
U=(-1/R2,1/R2,0); N=(1/R2,1/R2,0)
WOOD="var(--wood)"; PET="var(--ray)"; WALL="var(--sunk)"; FLOOR="var(--rule)"
FAB="var(--felt)"; FLAG="var(--flag)"; SHEET="var(--mdf)"
o=[]; CLIPS=[]
def clipped(f,cid,x,y,w,h):
    CLIPS.append('<clipPath id="%s"><rect x="%.0f" y="%.0f" width="%.0f" height="%.0f"/></clipPath>'%(cid,x,y,w,h))
    return ['  <g clip-path="url(#%s)">'%cid]+f+['  </g>']
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    o.append('  <text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    o.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def rc(x,y,w,h,fill,stroke="var(--ink)",sw="1",extra=""):
    o.append('  <rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(x,y,w,h,fill,stroke,sw,extra))
def poly(pts,fill,stroke,sw="1",extra=""):
    o.append('  <polygon points="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(" ".join("%.1f,%.1f"%p for p in pts),fill,stroke,sw,extra))
def tg(n,x,y,col="var(--ink)",r=12):
    o.append('  <circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.3"/>'%(x,y,r,col))
    o.append('  <text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def callout(p,dx,dy,lines,col="var(--ink)",anch="start"):
    if not p: return
    x2,y2=p[0]+dx,p[1]+dy
    li(p[0],p[1],x2,y2,col,"1.1")
    o.append('  <circle cx="%.1f" cy="%.1f" r="3.2" fill="%s"/>'%(p[0],p[1],col))
    for i,l in enumerate(lines):
        txt(x2+(6 if anch=="start" else -6),y2+4+i*14,l,"s-lbl" if i==0 else "s-dimt",anch,col if i==0 else None)

def triang(sc,z,shift=0.0):
    a=(0.0,0.0,z+shift); b=(CAT,0.0,z+shift); c=(0.0,CAT,z+shift)
    for dz in (0,CH):
        pts=[(p[0],p[1],p[2]+dz) for p in (a,b,c)]
        sc.quad(pts if dz else pts[::-1], SHEET,"var(--ink)",".8")
    for p,q in ((a,b),(b,c),(c,a)):
        sc.quad([p,q,(q[0],q[1],q[2]+CH),(p[0],p[1],p[2]+CH)], SHEET,"var(--ink)",".8")
def longarinas(sc,z0=0.0,h=HM,shift=0.0):
    base=(N[0]*shift,N[1]*shift,z0)
    for u0 in (0.0,HIP-BW):
        org=add(add((CAT,0,z0),mul(U,u0)),mul(N,shift))
        sc.box(org,mul(U,BW),mul(N,-BT),(0,0,h),WOOD,"var(--ink)",".8")
    sc.box((0.012+N[0]*shift,0.012+N[1]*shift,z0),(BW,0,0),(0,BW,0),(0,0,h),WOOD,"var(--ink)",".8")
def travessas(sc,z0=0.0,h=HM,shift=0.0):
    for zz in (0.0,h/2-BW/2,h-BW):
        org=add(add(add((CAT,0,z0),mul(N,-BT)),mul(N,shift)),(0,0,zz))
        sc.box(org,mul(U,HIP),mul(N,-BT),(0,0,BW),WOOD,"var(--ink)",".8")
def petfill(sc,z0=0.0,h=HM):
    K0=CAT; PASSO=0.05*R2
    for i,w in enumerate([0.60,0.45,0.30,0.15]):
        k0=K0-i*PASSO
        org=add((k0/2,k0/2,z0),mul(U,-w/2))
        sc.box(org,mul(U,w),mul(N,-0.0707),(0,0,h),PET,"var(--ray)",".7")
def upholstered(sc,z0=0.0,h=HM):
    a=(-0.004,-0.004,z0); b=(CAT+0.02,-0.004,z0); c=(-0.004,CAT+0.02,z0)
    sc.quad([(a[0],a[1],z0+h),(b[0],b[1],z0+h),(c[0],c[1],z0+h)],FAB,"var(--ink)",".9")
    for p,q in ((b,c),(c,a),(a,b)):
        sc.quad([p,q,(q[0],q[1],z0+h),(p[0],p[1],z0+h)],FAB,"var(--ink)",".9")

# ================= 1 : O PRISMA POR DENTRO =================
W1,H1=560,620
c1=Cam((2.15,1.55,2.05),(0.22,0.22,0.56),40,W1,H1); s1=Scene(c1,ox=40,oy=200)
triang(s1,0.0); triang(s1,HM-CH)
longarinas(s1); travessas(s1)
o+=clipped(s1.out(),'b1',40,206,560,630)
callout(s1.pt((0.14,0.14,HM)),86,-96,["1  Tri&#226;ngulo de chapa 6 mm","0,424 &#215; 0,424 &#215; 0,60","um em cima, um embaixo"],SHEET)
callout(s1.pt((0.035,0.035,0.72)),-96,-30,["2  Longarina do v&#233;rtice","1,20 m"],WOOD,"end")
callout(s1.pt(add((CAT,0,0.30),mul(U,0.02))),56,84,["3  Duas longarinas da frente","+ 4  tr&#234;s travessas de 0,60"],WOOD)
txt(60,178,"1 &#183; O ESQUELETO DE UM M&#211;DULO","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(60,196,"&#201; a escada da Op&#231;&#227;o A mais tr&#234;s pe&#231;as: dois tri&#226;ngulos e a longarina do v&#233;rtice.","s-dimt","start")

# ================= 2 : FECHADO, NO CANTO =================
W2,H2=450,690
c2=Cam((2.75,1.95,2.75),(0.26,0.26,1.02),35,W2,H2); s2=Scene(c2,ox=630,oy=180)
s2.quad([(0,0,0),(0,0,2.5),(0.9,0,2.5),(0.9,0,0)],WALL,"var(--rule)",".8",1.0,True,0)
s2.quad([(0,0,0),(0,0.9,0),(0,0.9,2.5),(0,0,2.5)],WALL,"var(--rule)",".8",1.0,True,0)
s2.quad([(0,0,0),(0.9,0,0),(0.9,0.9,0),(0,0.9,0)],FLOOR,"var(--rule)",".8",1.0,True,0)
for z0 in (0.0,HM): upholstered(s2,z0)
o+=clipped(s2.out(),'b2',628,176,456,700)
callout(s2.pt((0.14,0.14,2.28)),120,-64,["Fechado dos seis lados","frente, os dois lados e as","duas tampas &#8212; tudo tecido"],FAB)
txt(650,158,"2 &#183; FECHADO &#183; nada aparece e nada cai","s-dimt","start","var(--ink)",' letter-spacing="1.5"')

# ================= 3 : CORTE DOS TRIANGULOS =================
BX,BY,S3=1150,230,390
txt(BX,BY-58,"3 &#183; OS 8 TRI&#194;NGULOS SAEM DE 2 RETALHOS","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(BX,BY-40,"Retalho de 1,20 &#215; 0,60 de compensado 6 mm. Dois quadrados, um corte na diagonal em cada.","s-dimt","start")
for k in range(2):
    ox=BX+k*(S3*0.62)
    rc(ox,BY,S3*0.55,S3*0.275,"none","var(--mute)","1.6",' stroke-dasharray="6 4"')
    for j in range(2):
        qx=ox+j*S3*0.196
        rc(qx,BY,S3*0.196,S3*0.196,"var(--mdf-soft)","var(--mdf)","1.6")
        li(qx,BY+S3*0.196,qx+S3*0.196,BY,"var(--mdf)","2.4")
    txt(ox+S3*0.275,BY+S3*0.31,"1,20 &#215; 0,60","s-dimt","middle")
    txt(ox+S3*0.098,BY+S3*0.10,"0,424","s-dimt","middle")
txt(BX,BY+S3*0.40,"Sobra a faixa de 1,20 &#215; 0,17 em cada retalho &#8212; guarde para calços.","s-dimt","start")

# ================= 4 : ESTABILIDADE =================
EX,EY=1150,540
txt(EX,EY-16,"4 &#183; O PONTO FRACO: ELE TOMBA F&#193;CIL","s-dimt","start",FLAG,' letter-spacing="1.5"')
poly([(EX,EY+150),(EX+150,EY+150),(EX,EY+40)],"var(--sunk)","var(--mute)","1.6")
li(EX+52,EY+150,EX+52,EY+108,FLAG,"1.4",' stroke-dasharray="4 3"')
o.append('  <circle cx="%.1f" cy="%.1f" r="5" fill="%s"/>'%(EX+52,EY+113,FLAG))
txt(EX+62,EY+118,"centro de massa","s-dimt","start",FLAG)
li(EX,EY+150,EX+150,EY+150,"var(--ink)","2.5")
txt(EX+150,EY+168,"aresta de tombo","s-dimt","end")
txt(EX+190,EY+30,"Com o prisma vazio, o centro de massa fica a","s-dimt","start")
txt(EX+190,EY+46,"apenas 10 cm atr&#225;s da aresta da frente. Um","s-dimt","start")
txt(EX+190,EY+62,"empurr&#227;o de 5&#176; j&#225; derruba.","s-dimt","start")
txt(EX+190,EY+88,"Solu&#231;&#227;o: 8 a 12 kg de areia ensacada dentro do","s-dimt","start",FLAG)
txt(EX+190,EY+104,"m&#243;dulo de baixo. Vai para 10&#8211;12&#176;, custa R$ 10 e","s-dimt","start",FLAG)
txt(EX+190,EY+120,"n&#227;o muda nada na ac&#250;stica.","s-dimt","start",FLAG)
txt(EX+190,EY+146,"Se tiver crian&#231;a ou animal, some UM parafuso no","s-dimt","start")
txt(EX+190,EY+162,"topo &#8212; a&#237; volta a furar parede, mas s&#243; um furo.","s-dimt","start")

# ================= LISTA =================
LX=1700.0; yy=200.0
def head(t,col="var(--ink)"):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+330,yy,"var(--mute)","1"); yy+=24
def row(k,a,b,col=None):
    global yy
    txt(LX,yy,k,"s-dimt","start"); txt(LX+240,yy,a,"s-dimt","end")
    txt(LX+330,yy,b,"s-dimt","end",col); yy+=19
head("A CONTRA B")
txt(LX+240,yy-46,"A","s-dimt","end","var(--mute)"); txt(LX+330,yy-46,"B","s-dimt","end",FLAG)
row("Ripa","18,0 m","21,6 m")
row("Chapa 6 mm","&#8212;","1,44 m&#178;",FLAG)
row("Tecido ac&#250;stico","3,5 m","3,5 m")
row("Tecido barato (lados)","&#8212;","3,5 m",FLAG)
row("Bucha e parafuso na parede","16","0",FLAG)
row("Lastro de areia","&#8212;","10 kg")
yy+=4; li(LX,yy,LX+330,yy,"var(--rule)","1"); yy+=20
row("Custo a mais da B","","R$ 90 a 170",FLAG)
yy+=8
for l in ["Ordem de grandeza para conferir na sua regi&#227;o.","O que pesa &#233; a chapa e os 3,6 m de ripa a mais."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=22
head("O QUE A B COMPRA","var(--ray)")
for l in ["Nenhum furo de parede.","","Move inteiro: levanta e leva, a manta vai junto","dentro da caixa. Se a sala mudar, ou se voc&#234;","mudar de casa, ele vai junto pronto.","",
          "Mais r&#237;gido: os tri&#226;ngulos travam o esquadro","para sempre, sem depender do canto.","",
          "Serve em qualquer canto, inclusive nos dois que","hoje est&#227;o bloqueados, se um dia liberarem."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=18
head("O QUE N&#195;O MUDA","var(--ray)")
for l in ["O corte da manta: 0,615 / 0,465 / 0,315 / 0,165.","A ordem de entrada, da mais estreita para fora.","O envelope do tecido na face da frente.","As 4 escadas que voc&#234; j&#225; ia fazer &#8212; elas viram","a frente do prisma, sem uma serrada a mais."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16

txt(60,54,"OP&#199;&#195;O B &#183; PRISMA AUTOPORTANTE, FECHADO EM TECIDO","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(60,74,"Sem furar parede. Cada m&#243;dulo &#233; uma caixa fechada de 0,424 &#215; 0,424 &#215; 1,20 &#8212; dois por trap.","s-dimt","start")
open("_opcaoB.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 2090 980">\n<defs>'+"".join(CLIPS)+'</defs>\n'+"\n".join(o)+'\n</svg>')
print("ok")
