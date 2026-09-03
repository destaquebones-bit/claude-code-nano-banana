# -*- coding: utf-8 -*-
import math
W,L = 2.75, 3.70
DW=0.82; DX0=W-DW                      # vao de porta na parede frontal, encostado a direita
ML,MR = 0.99, 1.76                     # monitores
BAF=0.92                               # plano do driver
EAR=1.587; CEN=W/2
DESK0,DESK1=0.70,1.30
S=168.0; OX=92.0; OY=178.0
INK="var(--ink)"; MUTE="var(--mute)"; RAY="var(--ray)"; FLAG="var(--flag)"
WOOD="var(--wood)"; FELT="var(--felt)"; MDF="var(--mdf)"
o=[]
def a(s): o.append("  "+s)
def X(x): return OX+S*x
def Y(y): return OY+S*y
def rect(x0,x1,y0,y1,fill,stroke=INK,sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'
      %(X(x0),Y(y0),X(x1)-X(x0),Y(y1)-Y(y0),fill,stroke,sw,extra))
def line(p,q,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(X(p[0]),Y(p[1]),X(q[0]),Y(q[1]),col,sw,extra))
def lpx(x1,y1,x2,y2,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def txt(x,y,s,cls="s-lbl",anch="middle",fill=None,extra="",px=False):
    f=' fill="%s"'%fill if fill else ""
    xx,yy=(x,y) if px else (X(x),Y(y))
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,xx,yy,anch,f,extra,s))
def tag(n,xp,yp,col=INK,r=12.5):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.2"/>'%(xp,yp,r,col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(xp,yp+4,n))

a('<defs>')
a('<pattern id="solid" width="7" height="7" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="7" stroke="%s" stroke-width="1.1"/></pattern>'%MUTE)
a('<pattern id="hole" width="7" height="7" patternUnits="userSpaceOnUse"><circle cx="3.5" cy="3.5" r="1" fill="%s"/></pattern>'%FLAG)
a('</defs>')

# ---------- titulo ----------
txt(92,58,"POR QUE S&#211; A ESQUERDA TEM O BURACO","s-dimt","start",INK,' letter-spacing="1.6"',px=True)
txt(92,78,"Interfer&#234;ncia de fronteira (SBIR) &#183; e as quatro medidas que faltam para fechar o diagn&#243;stico","s-dimt","start",None,"",px=True)

# ---------- 1 PLANTA ----------
txt(92,140,"1 &#183; PLANTA &#183; onde cada parede cancela","s-dimt","start",INK,' letter-spacing="1.5"',px=True)
rect(0,W,0,L,"var(--surface)",INK,"2")
# parede frontal solida (esquerda) e vao (direita)
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="url(#solid)" stroke="none"/>'%(X(0),Y(0)-13,X(DX0)-X(0),13))
lpx(X(0),Y(0),X(DX0),Y(0),INK,"3.4")
lpx(X(DX0),Y(0),X(W),Y(0),FLAG,"3.4",' stroke-dasharray="7 6"')
txt((0+DX0)/2,-0.115,"PAREDE S&#211;LIDA &#183; reflete e cancela","s-dimt","middle",MUTE)
txt((DX0+W)/2,-0.115,"V&#195;O 0,82","s-dimt","middle",FLAG)
# paredes laterais
lpx(X(0),Y(0),X(0),Y(L),INK,"3.4"); lpx(X(W),Y(0),X(W),Y(L),INK,"3.4")
# bancada
rect(0.10,W-0.10,DESK0,DESK1,"var(--sunk)",MUTE,"1")
txt(CEN,(DESK0+DESK1)/2+0.04,"BANCADA","s-dimt","middle",MUTE)
# monitores
for mx in (ML,MR):
    rect(mx-0.11,mx+0.11,BAF-0.28,BAF,"var(--felt)",INK,"1")
    a('<circle cx="%.1f" cy="%.1f" r="4" fill="var(--paper)"/>'%(X(mx),Y(BAF)-4))
txt(ML,BAF-0.40,"L","s-leg","middle",INK); txt(MR,BAF-0.40,"R","s-leg","middle",INK)
# ouvinte
a('<circle cx="%.1f" cy="%.1f" r="13" fill="none" stroke="%s" stroke-width="1.6"/>'%(X(CEN),Y(EAR),INK))
txt(CEN,EAR+0.20,"escuta","s-dimt","middle",MUTE)

# caminhos de reflexao do monitor L (parede frontal e parede esquerda)
line((ML,BAF),(ML,0.02),RAY,"1.5",' stroke-dasharray="6 5"')
line((ML,0.02),(CEN,EAR),RAY,"1.5",' stroke-dasharray="6 5"')
line((ML,BAF),(0.02,BAF-0.16),RAY,"1.5",' stroke-dasharray="6 5"')
line((0.02,BAF-0.16),(CEN,EAR),RAY,"1.5",' stroke-dasharray="6 5"')
# monitor R: so a frontal, e ela acaba perto
line((MR,BAF),(MR,0.02),RAY,"1.5",' stroke-dasharray="6 5"')
line((MR,0.02),(CEN,EAR),RAY,"1.5",' stroke-dasharray="6 5"')
a('<path d="M %.1f %.1f Q %.1f %.1f %.1f %.1f" fill="none" stroke="%s" stroke-width="1.5" stroke-dasharray="4 5"/>'
  %(X(MR),Y(BAF-0.10),X(MR+0.55),Y(0.30),X(W-0.10),Y(-0.10),FLAG))
txt(MR+0.62,0.30,"escapa","s-dimt","start",FLAG)

# armario no canto traseiro esquerdo
rect(0,0.92,L-0.58,L,"var(--sunk)",MUTE,"1")
txt(0.46,L-0.26,"ARM&#193;RIO","s-dimt","middle",MUTE)
# os dois traps na diagonal
TR=0.424
a('<polygon points="%.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="%s" fill-opacity="0.30" stroke="%s" stroke-width="1.4"/>'
  %(X(0),Y(0),X(TR),Y(0),X(0),Y(TR),RAY,RAY))
a('<polygon points="%.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="%s" fill-opacity="0.30" stroke="%s" stroke-width="1.4"/>'
  %(X(W),Y(L),X(W-TR),Y(L),X(W),Y(L-TR),RAY,RAY))
tag("1",X(0.17),Y(0.17),RAY,11); tag("2",X(W-0.17),Y(L-0.17),RAY,11)
txt(0.62,0.30,"trap 1","s-dimt","start",RAY)
txt(W-0.62,L-0.30,"trap 2","s-dimt","end",RAY)

# cotas A B C D
def dimV(x,y0,y1,lab,col=FLAG):
    xp=X(x)
    lpx(xp,Y(y0),xp,Y(y1),col,"1.4")
    for yy in (y0,y1): lpx(xp-5,Y(yy),xp+5,Y(yy),col,"1.4")
def dimH(y,x0,x1,col=FLAG):
    yp=Y(y)
    lpx(X(x0),yp,X(x1),yp,col,"1.4")
    for xx in (x0,x1): lpx(X(xx),yp-5,X(xx),yp+5,col,"1.4")
dimV(ML-0.20,0,BAF,"A"); tag("A",X(ML-0.20),Y(BAF/2),FLAG)
dimV(MR+0.20,0,BAF,"B"); tag("B",X(MR+0.20),Y(BAF/2),FLAG)
dimH(BAF+0.16,0,ML);      tag("C",X(ML/2),Y(BAF+0.16),FLAG)
dimH(BAF+0.16,MR,W);      tag("D",X((MR+W)/2),Y(BAF+0.16),FLAG)
# cotas da sala
txt(CEN,L+0.16,"2,75 m","s-dimt","middle",MUTE)
a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" fill="%s" transform="rotate(-90 %.1f %.1f)">3,70 m</text>'
  %(X(W)+34,(Y(0)+Y(L))/2,MUTE,X(W)+34,(Y(0)+Y(L))/2))

# ---------- 2 A FISICA ----------
CX=740.0; yy=140.0
txt(CX,yy,"2 &#183; A CONTA","s-dimt","start",INK,' letter-spacing="1.5"',px=True); yy+=12
lpx(CX,yy,CX+400,yy,MUTE,"1"); yy+=40
txt(CX,yy,"f = 343 / (4 &#215; d)","s-big","start",INK,px=True); yy+=24
for l in ["d = dist&#226;ncia do alto-falante at&#233; a superf&#237;cie",
          "f = a frequ&#234;ncia que ela CANCELA"]:
    txt(CX,yy,l,"s-dimt","start",None,"",px=True); yy+=16
yy+=18
txt(CX,yy,"O som vai at&#233; a parede e volta. Se voltar meia onda","s-dimt","start",None,"",px=True); yy+=16
txt(CX,yy,"atrasado, chega invertido e some com o direto.","s-dimt","start",None,"",px=True); yy+=30

# tabela d -> f com faixa marcada
BAND=(0.31,1.14)
rows=[(0.15,572),(0.20,429),(0.30,286),(0.40,214),(0.50,172),(0.60,143),
      (0.70,122),(0.80,107),(0.90,95),(1.00,86),(1.20,71),(1.50,57)]
txt(CX,yy,"d","s-dimt","start",MUTE,px=True); txt(CX+150,yy,"cancela em","s-dimt","end",MUTE,px=True); yy+=8
lpx(CX,yy,CX+400,yy,MUTE,"1"); yy+=20
for d_,f_ in rows:
    dentro = BAND[0]<=d_<=BAND[1]
    if dentro:
        a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" fill-opacity="0.13"/>'%(CX-8,yy-13,416,19,FLAG))
    c = FLAG if dentro else None
    txt(CX,yy,"%.2f m"%d_,"s-dimt","start",c,px=True)
    txt(CX+150,yy,"%d Hz"%f_,"s-dimt","end",c,px=True)
    if dentro: txt(CX+408,yy,"dentro do seu buraco","s-dimt","end",FLAG,px=True)
    yy+=19
yy+=22
txt(CX,yy,"Seu buraco medido: 75 a 275 Hz","s-leg","start",FLAG,px=True); yy+=20
for l in ["Qualquer superf&#237;cie entre 0,31 e 1,14 m de um","alto-falante joga o cancelamento dela dentro dele.",
          "","Fora dessa janela, o cancelamento cai onde n&#227;o","atrapalha. &#201; s&#243; isso que a gente precisa fazer."]:
    txt(CX,yy,l,"s-dimt","start",None,"",px=True); yy+=17

# ---------- 3 O QUE FAZER ----------
RX=1230.0; yy=140.0
txt(RX,yy,"3 &#183; AS QUATRO MEDIDAS","s-dimt","start",FLAG,' letter-spacing="1.5"',px=True); yy+=12
lpx(RX,yy,RX+400,yy,MUTE,"1"); yy+=34
meds=[("A","Driver do monitor ESQUERDO","at&#233; a parede frontal"),
      ("B","Driver do monitor DIREITO","at&#233; a parede frontal"),
      ("C","Monitor ESQUERDO","at&#233; a parede esquerda"),
      ("D","Monitor DIREITO","at&#233; a parede direita")]
for n,t1,t2 in meds:
    tag(n,RX+13,yy-4,FLAG)
    txt(RX+36,yy-7,t1,"s-leg","start",INK,px=True)
    txt(RX+36,yy+8,t2,"s-dimt","start",None,"",px=True)
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="1" stroke-dasharray="3 4"/>'%(RX+300,yy+2,RX+400,yy+2,MUTE))
    txt(RX+400,yy-2,"____ m","s-dimt","end",MUTE,px=True)
    yy+=44
yy+=6
txt(RX,yy,"Mede at&#233; a frente da caixa, onde ficam os","s-dimt","start",None,"",px=True); yy+=16
txt(RX,yy,"alto-falantes &#8212; n&#227;o at&#233; a traseira.","s-dimt","start",None,"",px=True); yy+=34

lpx(RX,yy,RX+400,yy,MUTE,"1"); yy+=26
txt(RX,yy,"A REGRA","s-dimt","start",INK,' letter-spacing="1.5"',px=True); yy+=28
# barra de zonas
BW=400.0; BH=26.0
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" fill-opacity="0.18"/>'%(RX,yy,80,BH,RAY))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" fill-opacity="0.28"/>'%(RX+80,yy,230,BH,FLAG))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" fill-opacity="0.18"/>'%(RX+310,yy,90,BH,RAY))
txt(RX+40,yy+17,"BOM","s-dimt","middle",RAY,px=True)
txt(RX+195,yy+17,"ZONA RUIM","s-dimt","middle",FLAG,px=True)
txt(RX+355,yy+17,"BOM","s-dimt","middle",RAY,px=True)
yy+=BH+16
txt(RX,yy,"0","s-dimt","start",MUTE,px=True)
txt(RX+80,yy,"0,31","s-dimt","middle",MUTE,px=True)
txt(RX+310,yy,"1,14","s-dimt","middle",MUTE,px=True)
txt(RX+400,yy,"1,50 m","s-dimt","end",MUTE,px=True)
yy+=32
for l in ["Encoste em at&#233; 0,20 m, ou afaste al&#233;m de 1,20 m.",
          "O meio-termo &#233; o pior lugar poss&#237;vel.","",
          "As HS s&#227;o portadas atr&#225;s: 0,15 m &#233; o m&#237;nimo","para o port respirar. N&#227;o encoste mais que isso.","",
          "MOVER &#201; O REM&#201;DIO DE PRIMEIRA LINHA.","Se n&#227;o der para mover, o segundo rem&#233;dio &#233; l&#227; grossa","na parede frontal, atr&#225;s e ao lado do monitor L.","",
          "O trap de canto atende o MODO da sala. Cancelamento","de fronteira quer massa porosa no ponto onde a onda","bate &#8212; s&#227;o dois problemas e dois rem&#233;dios."]:
    txt(RX,yy,l,"s-dimt","start",None,"",px=True); yy+=17

open("_sbir.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1700 900">\n'+"\n".join(o)+'\n</svg>')
print("ok")
