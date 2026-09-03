# -*- coding: utf-8 -*-
import math
INK="var(--ink)"; MUTE="var(--mute)"; RAY="var(--ray)"; FLAG="var(--flag)"
WOOD="var(--wood)"; MDF="var(--mdf)"; SOFT="var(--mdf-soft)"; FELT="var(--felt)"
o=[]
def a(s): o.append("  "+s)
def txt(x,y,s,cls="s-dimt",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def rc(x,y,w,h,fill,stroke=INK,sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(x,y,w,h,fill,stroke,sw,extra))
def poly(pts,fill,stroke=INK,sw="1",extra=""):
    a('<polygon points="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(" ".join("%.1f,%.1f"%p for p in pts),fill,stroke,sw,extra))
def tg(n,x,y,col=INK,r=12.5):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.2"/>'%(x,y,r,col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def dimH(x0,x1,y,lab,col=MUTE):
    li(x0,y,x1,y,col,"1")
    for x in (x0,x1): li(x,y-5,x,y+5,col,"1")
    txt((x0+x1)/2,y-8,lab,"s-dimt","middle",col)
def dimV(y0,y1,x,lab,col=MUTE):
    li(x,y0,x,y1,col,"1")
    for y in (y0,y1): li(x-5,y,x+5,y,col,"1")
    a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" fill="%s" transform="rotate(-90 %.1f %.1f)">%s</text>'%(x-7,(y0+y1)/2,col,x-7,(y0+y1)/2,lab))
def ang(cx,cy,r,a0,a1,col,lab,lr=None):
    pts=[("%.1f,%.1f"%(cx+r*math.cos(a0+(a1-a0)*i/24.),cy+r*math.sin(a0+(a1-a0)*i/24.))) for i in range(25)]
    a('<polyline points="%s" fill="none" stroke="%s" stroke-width="1.3"/>'%(" ".join(pts),col))
    am=(a0+a1)/2; rr=lr or r+15
    txt(cx+rr*math.cos(am),cy+rr*math.sin(am)+4,lab,"s-dimt","middle",col)

txt(92,58,"O TRI&#194;NGULO &#183; COMO CORTAR E COMO PARAFUSAR","s-dimt","start",INK,' letter-spacing="1.6"')
txt(92,78,"Tri&#226;ngulo ret&#226;ngulo is&#243;sceles: catetos 42,4 cm, hipotenusa 60 cm. Nenhum transferidor entra nessa obra.","s-dimt","start")

# ================= 1 · O TRUQUE DO QUADRADO =================
txt(92,140,"1 &#183; O TRUQUE &#183; marque um QUADRADO e corte a diagonal","s-dimt","start",INK,' letter-spacing="1.5"')
SQ=272.0; QX,QY=110.0,190.0
rc(QX,QY,SQ,SQ,SOFT,INK,"2")
li(QX,QY+SQ,QX+SQ,QY,FLAG,"2.6")
txt(QX+SQ*0.30,QY+SQ*0.72,"A","s-big","middle",INK)
txt(QX+SQ*0.72,QY+SQ*0.30,"B","s-big","middle",INK)
# esquadro do canto
li(QX,QY+SQ-22,QX+22,QY+SQ-22,MUTE,"1.2"); li(QX+22,QY+SQ-22,QX+22,QY+SQ,MUTE,"1.2")
txt(QX+34,QY+SQ-30,"90&#176;","s-dimt","start",MUTE)
ang(QX+SQ,QY+SQ,34,math.pi,math.pi*1.25,FLAG,"45&#176;")
ang(QX,QY,34,0,math.pi*0.25,FLAG,"45&#176;")
dimH(QX,QX+SQ,QY-16,"42,4 cm")
dimV(QY,QY+SQ,QX-16,"42,4 cm")
txt(QX+SQ*0.52,QY+SQ*0.52,"60 cm","s-dimt","start",FLAG,' transform="rotate(-45 %.1f %.1f)"'%(QX+SQ*0.52,QY+SQ*0.52))
yy=QY+SQ+44
for l in ["Um quadrado de 42,4 &#215; 42,4 cortado na diagonal d&#225;",
          "DOIS tri&#226;ngulos perfeitos, com os 45&#176; de gra&#231;a.",
          "",
          "Marque o quadrado com esquadro de pedreiro. Confira","medindo as duas diagonais: t&#234;m que dar igual.",
          "",
          "42,4 cm &#233; 42 cm e 4 mm. N&#227;o arredonde para 42 &#8212; o","erro se acumula e a hipotenusa n&#227;o fecha em 60."]:
    txt(QX-18,yy,l,"s-dimt","start"); yy+=17

# ================= 2 · APROVEITAMENTO =================
CX=560.0
txt(CX,140,"2 &#183; 4 TRI&#194;NGULOS POR CHAPA","s-dimt","start",INK,' letter-spacing="1.5"')
# chapa 1,20 x 0,60 -> escala
SC=310.0/1.20
PX,PY=CX,180.0
rc(PX,PY,1.20*SC,0.60*SC,"var(--surface)",INK,"2")
q=0.424*SC
for i in range(2):
    x0=PX+i*2*q*0.5+i*0  # posicionar dois pares
for i,(ox,flip) in enumerate([(0,0),(q,1),(2*q,0),(0,0)][:3]):
    pass
# desenhar 4 triangulos nesteados na faixa de 0,424
base=PY
pairs=[(0,False),(q,True),(2*q,False)]
for ox,flip in pairs:
    x0=PX+ox
    if x0+q>PX+1.20*SC+1: continue
    if not flip: pts=[(x0,base+q),(x0,base),(x0+q,base+q)]
    else:        pts=[(x0,base),(x0+q,base),(x0+q,base+q)]
    poly(pts,SOFT,FLAG,"1.6")
# quarto triangulo aproveitando a ponta
x0=PX+3*q*0  # nao cabe: usar segunda chapa
txt(PX+1.20*SC/2,PY+0.60*SC+22,"chapa 1,20 &#215; 0,60","s-dimt","middle",MUTE)
li(PX,PY+q,PX+1.20*SC,PY+q,MUTE,"1",' stroke-dasharray="5 5"')
txt(PX+1.20*SC-4,PY+q+q*0.30,"sobra 17 cm &#8212; gua&#231;os e cal&#231;os","s-dimt","end",MUTE)
dimV(PY,PY+q,PX-14,"42,4")
yy=PY+0.60*SC+48
for l in ["Tr&#234;s por chapa, alternando ponta para cima e para","baixo. O quarto sai da sobra da chapa seguinte.",
          "","Voc&#234; precisa de 8 tri&#226;ngulos no total:","2 traps &#215; 2 m&#243;dulos &#215; 2 tampas."]:
    txt(PX,yy,l,"s-dimt","start"); yy+=17

# ================= 3 · O MODULO: PLANTA E FRENTE =================
txt(CX,556,"3 &#183; O M&#211;DULO &#183; onde cada ripa fica","s-dimt","start",INK,' letter-spacing="1.5"')
K=2.15   # px por cm
# --- planta ---
TX,TY=CX+16,600.0
c=42.4*K
poly([(TX,TY+c),(TX,TY),(TX+c,TY+c)],SOFT,INK,"1.8")
txt(TX,TY-34,"PLANTA","s-dimt","start",MUTE)
li(TX,TY+c-16,TX+16,TY+c-16,MUTE,"1"); li(TX+16,TY+c-16,TX+16,TY+c,MUTE,"1")
# longarinas: quadradinhos nos 3 cantos
for (px,py,n,col) in [(TX,TY+c,"2",WOOD),(TX,TY,"3",WOOD),(TX+c,TY+c,"3",WOOD)]:
    rc(px-9 if px>TX else px, py-18 if py==TY+c else py, 18,18, WOOD, INK,"1.4")
tg("2",TX+9,TY+c-9,WOOD,10)
tg("3",TX+9,TY+9,WOOD,10)
tg("3",TX+c-9,TY+c-9,WOOD,10)
dimH(TX,TX+c,TY-14,"42,4")
li(TX+c*0.56,TY+c*0.44,TX+c+40,TY+c*0.10,FLAG,"1.1")
txt(TX+c+45,TY+c*0.10+4,"face da frente","s-dimt","start",FLAG)
txt(TX+c+45,TY+c*0.10+20,"hipotenusa 60 cm","s-dimt","start",MUTE)
txt(TX,TY+c+34,"Duas ripas encostam nas paredes.","s-dimt","start")
txt(TX,TY+c+51,"A terceira fica no v&#233;rtice do canto.","s-dimt","start")

# --- elevacao da frente ---
FXX,FYY=CX+250,600.0
fw,fh=60*K,120*K*0.86
rc(FXX,FYY,fw,fh,"var(--surface)",MUTE,"1")
for ox in (0,fw-16):
    rc(FXX+ox,FYY,16,fh,WOOD,INK,"1.4")
for t in (0.06,0.48,0.90):
    rc(FXX+16,FYY+fh*t,fw-32,14,WOOD,INK,"1.4")
tg("3",FXX+8,FYY+fh*0.28,WOOD,10)
tg("4",FXX+fw/2,FYY+fh*0.48+7,FLAG,10)
dimH(FXX,FXX+fw,FYY-14,"60 cm")
dimV(FYY,FYY+fh,FXX-14,"1,20 m")
txt(FXX,FYY+fh+26,"FRENTE","s-dimt","start",MUTE)
txt(FXX,FYY+fh+48,"As 3 travessas seguram o tecido","s-dimt","start")
txt(FXX,FYY+fh+65,"e travam o esquadro da face.","s-dimt","start")

# ================= 4 · COMO PARAFUSAR =================
RX=980.0; yy=140.0
txt(RX,yy,"4 &#183; COMO PARAFUSAR NA PONTA DA RIPA","s-dimt","start",FLAG,' letter-spacing="1.5"'); yy+=12
li(RX,yy,RX+330,yy,MUTE,"1"); yy+=30
# corte: chapa + ripa em pe
BXX,BYY=RX+10,yy
rc(BXX,BYY,190,16,SOFT,INK,"1.6")                      # chapa 6mm (exagerada)
rc(BXX+58,BYY+16,74,150,WOOD,INK,"1.6")                # ripa de topo
txt(BXX+200,BYY+11,"tri&#226;ngulo","s-dimt","start",MUTE)
txt(BXX+142,BYY+92,"ripa em p&#233;","s-dimt","start",MUTE)
# furos
for ox in (78,112):
    li(BXX+ox,BYY-6,BXX+ox,BYY+16,FLAG,"1.4",' stroke-dasharray="3 3"')
    li(BXX+ox,BYY+16,BXX+ox,BYY+16+52,FLAG,"1.4",' stroke-dasharray="3 3"')
    poly([(BXX+ox-7,BYY),(BXX+ox+7,BYY),(BXX+ox+3,BYY+7),(BXX+ox-3,BYY+7)],FLAG,FLAG,"1")
txt(BXX+205,BYY+38,"furo-guia 2,5 mm","s-dimt","start",FLAG)
txt(BXX+205,BYY+54,"na ponta da ripa","s-dimt","start",MUTE)
txt(BXX+205,BYY-8,"escareado &#8212; cabe&#231;a afundada","s-dimt","start",FLAG)
yy=BYY+186
for l in ["Sempre DOIS parafusos por encontro. Um s&#243; deixa a","ripa girar e o esquadro se perde.",
          "","Fura antes, sempre. Ripa de 2,5 cm racha na ponta se","voc&#234; enfiar parafuso seco.",
          "","Broca 2,5 mm na ripa (furo-guia) e 3,7 mm na chapa","(furo passante) &#8212; assim o parafuso PUXA as duas.",
          "","Escareie a chapa. A cabe&#231;a tem que afundar: o tecido","passa por cima e cabe&#231;a saliente marca e rasga.",
          "","Parafuso 3,5 &#215; 40 mm. Menor que isso n&#227;o pega ponta."]:
    txt(RX,yy,l,"s-dimt","start"); yy+=17
yy+=14
txt(RX,yy,"COLA &#201; QUEM SEGURA","s-leg","start",FLAG); yy+=20
for l in ["Ponta de ripa &#233; fibra cortada: parafuso ali agarra","pouco. Passe cola branca de madeira em toda junta.",
          "","O parafuso &#233; o grampo que segura enquanto a cola","cura. Depois de seca, ela &#233; que faz a for&#231;a."]:
    txt(RX,yy,l,"s-dimt","start"); yy+=17

# ================= 5 · LISTA E ORDEM =================
LX=1370.0; yy=140.0
txt(LX,yy,"LISTA POR M&#211;DULO","s-dimt","start",INK,' letter-spacing="1.5"'); yy+=12
li(LX,yy,LX+250,yy,MUTE,"1"); yy+=26
items=[("1","Tri&#226;ngulo de chapa","2 pe&#231;as &#183; 42,4/42,4/60",INK),
       ("2","Longarina do v&#233;rtice","1 &#183; ripa de 1,20 m",WOOD),
       ("3","Longarinas da frente","2 &#183; ripa de 1,20 m",WOOD),
       ("4","Travessas da frente","3 &#183; ripa de 0,60 m",FLAG)]
for n,t1,t2,c in items:
    tg(n,LX+12,yy-4,c,11)
    txt(LX+34,yy-7,t1,"s-leg","start",INK); txt(LX+34,yy+8,t2,"s-dimt","start")
    yy+=40
yy+=8; li(LX,yy,LX+250,yy,MUTE,"1"); yy+=26
txt(LX,yy,"TOTAL DA OBRA","s-dimt","start",INK,' letter-spacing="1.5"'); yy+=24
for a_,b_ in [("Tri&#226;ngulos de chapa","8"),("Ripa (longarinas)","14,4 m"),
              ("Ripa (travessas)","7,2 m"),("Ripa total","21,6 m"),
              ("Parafuso 3,5 &#215; 40","~120"),("Cola branca","1 frasco")]:
    txt(LX,yy,a_,"s-dimt","start"); txt(LX+250,yy,b_,"s-dimt","end"); yy+=19
yy+=20; li(LX,yy,LX+250,yy,MUTE,"1"); yy+=26
txt(LX,yy,"ORDEM DE MONTAGEM","s-dimt","start",FLAG,' letter-spacing="1.5"'); yy+=24
for i,l in enumerate(["Corte os 8 tri&#226;ngulos.","Corte as ripas no esquadro (90&#176;).",
                      "Deite UM tri&#226;ngulo no ch&#227;o plano.","Fure, cole e parafuse as 3 longarinas","em p&#233; sobre ele.",
                      "Ponha o segundo tri&#226;ngulo em cima e","repita. O esquadro sai sozinho.",
                      "Por &#250;ltimo as 3 travessas da frente."],1):
    txt(LX,yy,l,"s-dimt","start"); yy+=17
yy+=14
for l in ["Nenhuma ripa leva corte em 45&#176;. Todas as pontas","s&#227;o cortadas em 90&#176;. Quem faz o &#226;ngulo &#233; a chapa."]:
    txt(LX,yy,l,"s-dimt","start",FLAG); yy+=17

open("_corte.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1700 940">\n'+"\n".join(o)+'\n</svg>')
print("ok")
