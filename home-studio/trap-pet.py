# -*- coding: utf-8 -*-
import math
R2=math.sqrt(2); PASSO=0.05*R2          # recuo de k a cada camada de 50 mm
K0=0.424; NC=4; ALT=2.40; PD=2.65
CUT=[0.60,0.45,0.30,0.15]
o=[]
def a(s): o.append("  "+s)
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def pg(pts,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<polygon points="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(" ".join("%.1f,%.1f"%p for p in pts),fill,stroke,sw,extra))
def tg(n,x,y,col="var(--ink)",r=11.5):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2"/>'%(x,y,r,col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
a('<defs>')
a('<pattern id="wl" width="10" height="10" patternUnits="userSpaceOnUse"><circle cx="5" cy="5" r="2.6" fill="none" stroke="var(--ray)" stroke-width="1"/></pattern>')
a('<pattern id="hw" width="8" height="8" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="8" stroke="var(--mute)" stroke-width="1"/></pattern>')
a('</defs>')

# ---------- 1. SECAO HORIZONTAL ----------
S=1000.0; CX=175.0; CY=690.0
def P(x,y): return (CX+S*x, CY-S*y)
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="url(#hw)" opacity=".5"/>'%(CX,CY,S*0.50,28))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="url(#hw)" opacity=".5"/>'%(CX-28,CY-S*0.50,28,S*0.50))
li(CX,CY,CX+S*0.50,CY,"var(--ink)","3.5"); li(CX,CY,CX,CY-S*0.50,"var(--ink)","3.5")
for i in range(NC):
    k0=K0-i*PASSO; k1=k0-PASSO
    pg([P(k0,0),P(0,k0),P(0,k1),P(k1,0)],"url(#wl)","var(--ray)","1.8")
kf=K0-NC*PASSO
pg([P(0,0),P(kf,0),P(0,kf)],"none","var(--mute)","1.5",' stroke-dasharray="6 4"')
e1=P(K0,0); e2=P(0,K0)
li(e1[0],e1[1],e2[0],e2[1],"var(--felt)","4")
# chamadas das camadas
for i in range(NC):
    k=K0-(i+0.5)*PASSO
    p=P(k*0.72,k*0.28)
    tg(str(i+1),p[0],p[1],"var(--ray)",10)
mm=P(kf*0.30,kf*0.30); txt(mm[0],mm[1]+4,"ar","s-dimt","middle")
# cotas
li(CX,CY+56,e1[0],CY+56,"var(--mute)","1")
for xx in (CX,e1[0]): li(xx,CY+50,xx,CY+62,"var(--mute)","1")
txt((CX+e1[0])/2,CY+48,"0,424","s-dimt","middle")
li(CX-62,CY,CX-62,e2[1],"var(--mute)","1")
for yy in (CY,e2[1]): li(CX-68,yy,CX-56,yy,"var(--mute)","1")
a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" transform="rotate(-90 %.1f %.1f)">0,424</text>'%(CX-68,(CY+e2[1])/2,CX-68,(CY+e2[1])/2))
f=P(K0/2,K0/2); li(CX,CY,f[0],f[1],"var(--flag)","1.8",' stroke-dasharray="8 5"')
a('<circle cx="%.1f" cy="%.1f" r="4" fill="var(--flag)"/>'%(f[0],f[1]))
li(f[0],f[1],f[0]+120,f[1]-96,"var(--flag)","1")
txt(f[0]+126,f[1]-98,"0,300 do canto at&#233; a face","s-lbl","start","var(--flag)")
txt(f[0]+126,f[1]-84,"quarto de onda em 286 Hz","s-dimt","start")
g=P(K0*0.26,K0*0.74); li(g[0],g[1],g[0]+150,g[1]-58,"var(--felt)","1")
txt(g[0]+156,g[1]-60,"tecido ac&#250;stico na face","s-lbl","start","var(--felt)")
txt(g[0]+156,g[1]-46,"sopre nele: o ar tem que passar","s-dimt","start")
txt(120,150,"1 &#183; SE&#199;&#195;O HORIZONTAL DO CANTO","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(120,170,"Quatro camadas de 50 mm em degrau. A l&#227; de 7 kg/m&#179; &#233; mole: corte","s-dimt","start")
txt(120,186,"ret&#226;ngulos e empurre &#8212; ela se acomoda sozinha contra as paredes.","s-dimt","start")

# ---------- 2. ELEVACAO ----------
EX=900.0; EY=250.0; ES=175.0
def E(x,z): return (EX+ES*x, EY+ES*(PD-z))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="url(#hw)" opacity=".45"/>'%(EX-32,EY,32,ES*PD))
li(EX,EY,EX,EY+ES*PD,"var(--ink)","3")
li(EX-45,EY,EX+ES*0.95,EY,"var(--ink)","3.5"); li(EX-45,EY+ES*PD,EX+ES*0.95,EY+ES*PD,"var(--ink)","3.5")
txt(EX+ES*0.95+8,EY+5,"teto","s-dimt","start"); txt(EX+ES*0.95+8,EY+ES*PD+5,"piso","s-dimt","start")
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="url(#wl)" stroke="var(--ray)" stroke-width="2"/>'
  %(E(0,ALT)[0],E(0,ALT)[1],ES*0.60,ES*ALT))
li(E(0,1.20)[0],E(0,1.20)[1],E(0.60,1.20)[0],E(0,1.20)[1],"var(--ray)","1.4",' stroke-dasharray="8 5"')
for z in (0,1.20,2.40):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="var(--wood)"/>'%(E(0,z)[0],E(0,z)[1]-5,ES*0.60,10))
for x in (0.03,0.57):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="var(--wood)"/>'%(E(x,0)[0]-4,E(0,ALT)[1],8,ES*ALT))
for z in (1.80,0.60):
    px,py=E(0.30,z)
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="var(--surface)" opacity=".93"/>'%(px-52,py-11,104,17))
    txt(px,py+1,"placa 1,20","s-dimt","middle","var(--ink)")
tg(3,E(0.30,2.40)[0],E(0,2.40)[1],"var(--wood)")
xd=EX+ES*0.60+40
li(xd,E(0,0)[1],xd,E(0,ALT)[1],"var(--mute)","1")
for z in (0,ALT): li(xd-5,E(0,z)[1],xd+5,E(0,z)[1],"var(--mute)","1")
a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" transform="rotate(-90 %.1f %.1f)">2,40</text>'%(xd-6,(E(0,0)[1]+E(0,ALT)[1])/2,xd-6,(E(0,0)[1]+E(0,ALT)[1])/2))
li(xd,E(0,ALT)[1],xd,EY,"var(--flag)","1.2")
for yy in (E(0,ALT)[1],EY): li(xd-5,yy,xd+5,yy,"var(--flag)","1.2")
txt(xd+12,(E(0,ALT)[1]+EY)/2+4,"0,25 sobra no alto","s-dimt","start","var(--flag)")
li(E(0,0)[0],E(0,0)[1]+34,E(0.60,0)[0],E(0,0)[1]+34,"var(--mute)","1")
for x in (0,0.60): li(E(x,0)[0],E(0,0)[1]+28,E(x,0)[0],E(0,0)[1]+40,"var(--mute)","1")
txt(E(0.30,0)[0],E(0,0)[1]+26,"0,60","s-dimt","middle")
txt(EX-45,EY-30,"2 &#183; ELEVA&#199;&#195;O","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(EX-45,EY-14,"cada coluna = 2 placas de 1,20 empilhadas","s-dimt","start")

# ---------- 3. LISTA ----------
LX=1220.0; yy=160.0
def head(t,col="var(--ink)"):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+320,yy,"var(--mute)","1"); yy+=24
def row(k,v,col=None,tag=None):
    global yy
    if tag: tg(tag,LX+9,yy-4,"var(--ray)",9.5); txt(LX+26,yy,k,"s-dimt","start")
    else: txt(LX,yy,k,"s-dimt","start")
    txt(LX+320,yy,v,"s-dimt","end",col); yy+=19
head("O CORTE &#183; 10 PLACAS, SOBRA ZERO")
row("Camada 0,60 &#215; 2,40","4 placas inteiras",None,"1")
row("Camada 0,45 &#215; 2,40","4 placas rasgadas",None,"2")
row("Camada 0,30 &#215; 2,40","2 placas rasgadas",None,"3")
row("Camada 0,15 &#215; 2,40","as sobras da 0,45",None,"4")
yy+=4; li(LX,yy,LX+320,yy,"var(--rule)","1"); yy+=20
row("Manta de 50 mm consumida","7,20 m&#178;","var(--ray)")
row("Peso total de PET nos dois","2,5 kg","var(--ray)")
yy+=26
head("O QUADRO DE RIP&#195;O","var(--wood)")
for k,v in [("2 montantes de 2,40 por trap","9,6 m"),("3 tri&#226;ngulos 0,42 / 0,42 / 0,60","8,7 m"),
            ("Sarrafo 2,5 &#215; 5 cm d&#225; e sobra","&#8776;19 m"),("Comprar","7 barras de 3 m")]:
    row(k,v)
yy+=6
for l in ["O recheio pesa 2,5 kg. O quadro vai pesar","mais que a l&#227; &#8212; n&#227;o precisa de madeira grossa."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=15
yy+=24
head("O QUE ESPERAR, SEM ROMANCE","var(--flag)")
for f,al,sab in (("125 Hz","0,30","0,9 sabins"),("250 Hz","0,55","1,6 sabins"),
                 ("500 Hz","0,80","2,3 sabins"),("1 kHz","0,85","2,4 sabins")):
    row(f+" &#183; alfa "+al,sab)
yy+=8
for l in ["A meta da sala inteira &#233; 16 sabins.","","A 7 kg/m&#179; a manta &#233; pouco resistiva: rende","muito mais no m&#233;dio que no grave. Os dois traps","s&#227;o um bom come&#231;o &#8212; mas o pico de 127 Hz vai","precisar de mais material ou de manta densa.","","Na diagonal porque os outros dois cantos est&#227;o","ocupados: a porta na frente, o arm&#225;rio no fundo."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=15

txt(110,54,"BASS TRAP DE CANTO &#183; L&#195; DE PET 7 kg/m&#179; &#183; FRONTAL ESQUERDO E TRASEIRO DIREITO","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(110,74,"Cotas em metros &#183; face a 0,424 de cada parede, igual &#224; planta &#183; 20 cm de manta e 10 cm de ar no v&#233;rtice","s-dimt","start")
open("_trap-pet.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1600 900">\n'+"\n".join(o)+'\n</svg>')
print("ok")
