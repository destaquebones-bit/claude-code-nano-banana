# -*- coding: utf-8 -*-
import math
from iso3d import *
WOOD="var(--wood)"; PET="var(--ray)"; MDF="var(--mdf)"; PLY="var(--wood)"
FLAG="var(--flag)"; AIR="var(--glass)"; INK="var(--ink)"
o=[]; CLIPS=[]
def clipped(f,cid,x,y,w,h):
    CLIPS.append('<clipPath id="%s"><rect x="%.0f" y="%.0f" width="%.0f" height="%.0f"/></clipPath>'%(cid,x,y,w,h))
    return ['  <g clip-path="url(#%s)">'%cid]+f+['  </g>']
def txt(x,y,s,cls="s-dimt",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    o.append('  <text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,c,w="1",e=""): o.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,c,w,e))
def rc(x,y,w,h,f,s="none",sw="1",e=""): o.append('  <rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(x,y,w,h,f,s,sw,e))
def tg(n,x,y,c=INK,r=12):
    o.append('  <circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.3"/>'%(x,y,r,c))
    o.append('  <text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def callto(p,x2,y2,lines,col=INK,anch="start"):
    if not p: return
    li(p[0],p[1],x2,y2,col,"1.1")
    o.append('  <circle cx="%.1f" cy="%.1f" r="3.2" fill="%s"/>'%(p[0],p[1],col))
    for i,l in enumerate(lines):
        txt(x2+(6 if anch=="start" else -6),y2+4+i*14,l,"s-lbl" if i==0 else "s-dimt",anch,col if i==0 else None)

# ============ 1 · EXPLODIDA 3D ============
Wd,Ht,Dp=0.72,1.20,0.15
FACE=0.008; BACK=0.010; SIDE=0.025
W1,H1=470,700
c1=Cam((2.30,1.70,2.00),(0.30,0.15,0.58),34,W1,H1); s1=Scene(c1,ox=14,oy=180)
def slab(sc,o_,w,h,t,col,shift=0.0):
    sc.box((o_[0],o_[1]-shift,o_[2]),(w,0,0),(0,-t,0),(0,0,h),col,"var(--ink)",".8")
# fundo (y=0), depois laterais, la, face
slab(s1,(0,0,0),Wd,Ht,BACK,PLY,0.0)
for x0 in (0,Wd-SIDE):
    s1.box((x0,-BACK-0.14,0),(SIDE,0,0),(0,-Dp,0),(0,0,Ht),WOOD,"var(--ink)",".8")
s1.box((0,-BACK-0.14,0),(Wd,0,0),(0,-Dp,0),(0,0,SIDE),WOOD,"var(--ink)",".8")
s1.box((0,-BACK-0.14,Ht-SIDE),(Wd,0,0),(0,-Dp,0),(0,0,SIDE),WOOD,"var(--ink)",".8")
s1.box((SIDE,-BACK-0.30,0.02),(Wd-2*SIDE,0,0),(0,-0.05,0),(0,0,Ht-0.04),PET,"var(--ray)",".7")
s1.box((0,-BACK-0.50,0),(Wd,0,0),(0,-FACE,0),(0,0,Ht),MDF,"var(--ink)",".9")
o+=clipped(s1.out(),'m1',34,196,392,676)
LGX=452
callto(s1.pt((Wd*0.45,-BACK-0.50-FACE,Ht*0.80)),LGX,268,["1  FACE &#183; MDF 8 mm","a pe&#231;a sintonizada.","S&#243; ela vibra &#8212; nada mais."],MDF)
callto(s1.pt((Wd-SIDE/2,-BACK-0.22,Ht*0.62)),LGX,398,["2  MOLDURA","t&#225;bua 2,5 &#215; 15 cm","define a c&#226;mara de 150 mm"],WOOD)
callto(s1.pt((Wd*0.5,-BACK-0.33,Ht*0.34)),LGX,528,["3  L&#195; solta no fundo","50 mm, colada s&#243; no fundo","folga de 100 mm at&#233; a face"],PET)
callto(s1.pt((Wd*0.32,0.002,Ht*0.16)),LGX,658,["4  FUNDO r&#237;gido","compensado 10 mm","ou a pr&#243;pria parede"],WOOD)
txt(60,158,"1 &#183; AS QUATRO CAMADAS","s-dimt","start",INK,' letter-spacing="1.5"')
txt(60,176,"Uma caixa hermética com uma tampa que vibra. Nada mais que isso.","s-dimt","start")

# ============ 2 · CORTE COTADO ============
BX,BY,S=700,250,1500
txt(BX-20,178,"2 &#183; CORTE &#183; a c&#226;mara de ar &#233; a mola, a chapa &#233; a massa","s-dimt","start",INK,' letter-spacing="1.5"')
txt(BX-20,196,"O ar preso entre a face e o fundo empurra de volta. A frequência sai da razão entre os dois.","s-dimt","start")
y0=BY
rc(BX,y0,S*0.008,S*0.20,MDF,"var(--ink)","1.6")                       # face 8mm
rc(BX+S*0.008,y0,S*0.142,S*0.20,"none","none","0")                     # ar
rc(BX+S*0.10,y0+S*0.012,S*0.05,S*0.176,PET,"var(--ray)","1.2")         # la
rc(BX+S*0.15,y0,S*0.010,S*0.20,PLY,"var(--ink)","1.6")                 # fundo
# hachura do ar
for k in range(9):
    yy=y0+S*0.02+k*S*0.02
    li(BX+S*0.012,yy,BX+S*0.098,yy,AIR,"1",' stroke-dasharray="3 5"')
txt(BX+S*0.055,y0+S*0.105,"AR","s-lbl","middle",AIR)
# setas de vibracao
for yy in (y0+S*0.05,y0+S*0.15):
    li(BX-16,yy,BX-2,yy,FLAG,"2"); li(BX-16,yy,BX-10,yy-5,FLAG,"2"); li(BX-16,yy,BX-10,yy+5,FLAG,"2")
tg("1",BX+S*0.004,y0-16,MDF,10); tg("3",BX+S*0.125,y0-16,PET,10); tg("4",BX+S*0.155,y0-16,PLY,10)
def dimH(x0,x1,yy,lab,col="var(--mute)"):
    li(x0,yy,x1,yy,col,"1")
    for x in (x0,x1): li(x,yy-5,x,yy+5,col,"1")
    txt((x0+x1)/2,yy-8,lab,"s-dimt","middle",col)
dimH(BX,BX+S*0.16,y0+S*0.235,"150 mm de c&#226;mara &#183; d")
dimH(BX,BX+S*0.008,y0-34,"8")
dimH(BX+S*0.15,BX+S*0.16,y0-34,"10")
txt(BX+S*0.19,y0+S*0.03,"f = 60 / &#8730;(m &#215; d)","s-big","start",INK)
txt(BX+S*0.19,y0+S*0.055,"m = 750 &#215; 0,008 = 6,0 kg/m&#178;","s-dimt","start")
txt(BX+S*0.19,y0+S*0.073,"d = 0,15 m","s-dimt","start")
txt(BX+S*0.19,y0+S*0.098,"f = 60 / &#8730;0,90 = 63,2 Hz","s-lbl","start",FLAG)
txt(BX+S*0.19,y0+S*0.135,"A l&#227; ocupa 50 mm no fundo e deixa","s-dimt","start")
txt(BX+S*0.19,y0+S*0.153,"100 mm de ar livre atr&#225;s da face.","s-dimt","start")
txt(BX+S*0.19,y0+S*0.171,"Ela amortece a resson&#226;ncia; n&#227;o absorve.","s-dimt","start")
txt(BX+S*0.19,y0+S*0.199,"A seta laranja &#233; por onde o som chega e faz a chapa trabalhar.","s-dimt","start",FLAG)

# ============ 3 · AS TRES REGRAS ============
RX,RY=700,672
txt(RX-20,RY-16,"3 &#183; AS TR&#202;S REGRAS QUE DECIDEM SE FUNCIONA","s-dimt","start",FLAG,' letter-spacing="1.5"')
rules=[("HERM&#201;TICA","Silicone por dentro em TODAS as juntas. Uma fresta e a mola de ar vaza:","vira pain&#233;l furado qualquer, sem sintonia nenhuma."),
       ("A L&#195; N&#195;O ENCOSTA","Ela vai no fundo, ocupando dois ter&#231;os. Encostada na face, ela trava","a chapa e a caixa deixa de ressoar."),
       ("O FUNDO &#201; R&#205;GIDO","Se o fundo vibrar junto, viram duas massas e a sintonia se perde.","Compensado 10 mm, ou parafuse a caixa direto na parede.")]
yy=RY+14
for i,(a_,b_,c_) in enumerate(rules,1):
    tg(str(i),RX-6,yy+2,FLAG,11)
    txt(RX+16,yy+6,a_,"s-lbl","start",INK)
    txt(RX+16,yy+23,b_,"s-dimt","start"); txt(RX+16,yy+39,c_,"s-dimt","start")
    yy+=62

# ============ 4 · TABELA DE SINTONIA ============
LX=1524.0; yy=200.0
def head(t,col=INK):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+412,yy,"var(--mute)","1"); yy+=24
def row(a_,b_,c_,col=None):
    global yy
    txt(LX,yy,a_,"s-dimt","start"); txt(LX+290,yy,b_,"s-dimt","end"); txt(LX+412,yy,c_,"s-dimt","end",col); yy+=19
head("AS DUAS CAIXAS")
row("M1 &#183; parede frontal","2 de 0,72 &#215; 1,20","63 Hz","var(--flag)")
row("   face MDF 8 mm &#183; c&#226;mara 150","","6,0 kg/m&#178;")
row("M2 &#183; laterais","2 de 0,60 &#215; 1,20","126 Hz","var(--flag)")
row("   face MDF 3 mm &#183; c&#226;mara 100","","2,25 kg/m&#178;")
yy+=8; row("Peso de cada","M1 18 kg","M2 11 kg")
yy+=24
head("SE A SINTONIA SAIR ERRADA","var(--mdf)")
for l in ["A f&#243;rmula acerta dentro de uns 15%. D&#225; para","corrigir depois de pronto, sem refazer nada:"]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=8
row("Colar +0,5 kg/m&#178; na face","","desce para 61 Hz")
row("Colar +1,0 kg/m&#178;","","desce para 59 Hz")
row("C&#226;mara de 12 cm","","sobe para 71 Hz")
row("C&#226;mara de 18 cm","","desce para 58 Hz")
yy+=8
for l in ["Massa &#233; mais f&#225;cil: uma segunda chapa fina","colada por cima resolve sem abrir a caixa."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=26
head("LISTA DE CORTE","var(--ray)")
for a_,b_ in [("MDF 8 mm","1,73 m&#178;"),("MDF 3 mm","1,44 m&#178;"),("Compensado 10 mm","3,17 m&#178;"),
              ("T&#225;bua 2,5 &#215; 15 cm","7,5 m"),("T&#225;bua 2,5 &#215; 10 cm","7,0 m"),
              ("L&#227; de amortecimento 50 mm","3,17 m&#178;"),("Silicone neutro","2 tubos"),("Parafuso 3,5 &#215; 30","60")]:
    txt(LX,yy,a_,"s-dimt","start"); txt(LX+412,yy,b_,"s-dimt","end"); yy+=19
yy+=10
for l in ["A l&#227; daqui n&#227;o precisa ser boa &#8212; ela s&#243; amortece.","Sobra de PET, manta barata ou cobertor velho serve.","Guarde a l&#227; boa para os cantos."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16

txt(60,54,"ABSORVEDOR DE MEMBRANA &#183; COMO CONSTRUIR","s-dimt","start",INK,' letter-spacing="1.6"')
txt(60,74,"Quatro membranas: duas de 63 Hz na parede frontal, duas de 126 Hz nas laterais","s-dimt","start")
open("_membrana.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1990 940">\n<defs>'+"".join(CLIPS)+'</defs>\n'+"\n".join(o)+'\n</svg>')
print("ok")
