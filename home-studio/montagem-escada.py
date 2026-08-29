# -*- coding: utf-8 -*-
import math
BW=0.05                      # largura da face da ripa (exemplo)
o=[]
def a(s): o.append("  "+s)
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def li(x1,y1,x2,y2,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(x1,y1,x2,y2,col,sw,extra))
def rc(x,y,w,h,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(x,y,w,h,fill,stroke,sw,extra))
def tg(n,x,y,col="var(--ink)",r=13):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s" stroke="var(--paper)" stroke-width="2.2"/>'%(x,y,r,col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(x,y+4,n))
def screw(x,y,r=5.2):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="none" stroke="var(--flag)" stroke-width="1.8"/>'%(x,y,r))
    li(x-r*.66,y-r*.66,x+r*.66,y+r*.66,"var(--flag)","1.6"); li(x-r*.66,y+r*.66,x+r*.66,y-r*.66,"var(--flag)","1.6")
def dimH(x0,x1,y,lab,col="var(--mute)"):
    li(x0,y,x1,y,col,"1")
    for x in (x0,x1): li(x,y-5,x,y+5,col,"1")
    txt((x0+x1)/2,y-8,lab,"s-dimt","middle",col)
def dimV(y0,y1,x,lab,col="var(--mute)"):
    li(x,y0,x,y1,col,"1")
    for y in (y0,y1): li(x-5,y,x+5,y,col,"1")
    a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" fill="%s" transform="rotate(-90 %.1f %.1f)">%s</text>'%(x-6,(y0+y1)/2,col,x-6,(y0+y1)/2,lab))

a('<defs><pattern id="gr" width="7" height="7" patternTransform="rotate(35)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="7" stroke="var(--wood)" stroke-width=".7" opacity=".55"/></pattern></defs>')

# ============ PAINEL 1 : A ESCADA MONTADA ============
S=390.0; OX=200.0; OY=215.0
def L(x,z): return (OX+S*x, OY+S*(1.20-z))
txt(96,138,"1 &#183; A ESCADA MONTADA &#183; fa&#231;a quatro iguais","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(96,158,"Vista da face que vai olhar para a sala. As travessas ficam POR CIMA dos montantes.","s-dimt","start")
# montantes
for x0 in (0.0,0.60-BW):
    p=L(x0,1.20); rc(p[0],p[1],S*BW,S*1.20,"url(#gr)","var(--wood)","2")
# travessas por cima
for z0,lab in ((1.20-BW,"B1"),(0.60-BW/2,"B2"),(0.0,"B3")):
    p=L(0,z0+BW); rc(p[0],p[1],S*0.60,S*BW,"var(--wood)","var(--ink)","2",' fill-opacity=".9"')
for z0 in (1.20-BW/2, 0.60, BW/2):
    for xc in (BW/2, 0.60-BW/2):
        p=L(xc,z0)
        screw(p[0]-7,p[1]-6); screw(p[0]+7,p[1]+6)
tg("A",L(BW/2,0.92)[0],L(0,0.92)[1],"var(--wood)")
tg("A",L(0.60-BW/2,0.28)[0],L(0,0.28)[1],"var(--wood)")
for z,lb in ((1.20-BW/2,"B"),(0.60,"B"),(BW/2,"B")):
    tg(lb,L(0.30,z)[0],L(0,z)[1],"var(--ink)")
dimH(L(0,0)[0],L(0.60,0)[0],L(0,0)[1]+64,"0,60")
dimV(L(0,1.20)[1],L(0,0)[1],L(0,0)[0]-46,"1,20")
# diagonal de conferencia
li(L(0,0)[0],L(0,0)[1],L(0.60,1.20)[0],L(0,1.20)[1],"var(--flag)","1.4",' stroke-dasharray="7 5"')
txt(L(0.60,0.86)[0]+16,L(0,0.86)[1],"as duas diagonais","s-dimt","start","var(--flag)")
txt(L(0.60,0.86)[0]+16,L(0,0.86)[1]+14,"t&#234;m que dar 1,342 m","s-dimt","start","var(--flag)")
txt(L(0,1.20)[0],L(0,1.20)[1]-14,"B1 rente ao topo","s-dimt","start")
txt(L(0,0)[0],L(0,0)[1]+30,"B3 rente ao p&#233;","s-dimt","start")
txt(L(0,0)[0],L(0,0)[1]+44,"B2 no meio, centro em 0,60","s-dimt","start")

# ============ PAINEL 2 : O ENCAIXE ============
DX=700.0; DY=250.0; DS=1400.0
txt(DX-40,138,"2 &#183; O ENCAIXE &#183; corte pelo montante","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(DX-40,158,"Travessa por cima, dois parafusos em diagonal. Sem corte de esquadria.","s-dimt","start")
rc(DX,DY+DS*BW,DS*0.16,DS*BW,"url(#gr)","var(--wood)","2.2")            # montante (de topo)
rc(DX+DS*0.02,DY,DS*0.12,DS*BW,"var(--wood)","var(--ink)","2.2",' fill-opacity=".9"')   # travessa
for dx in (0.045,0.095):
    x=DX+DS*dx; screw(x,DY+DS*BW/2,6.5)
    li(x,DY+DS*BW/2,x,DY+DS*BW*1.8,"var(--flag)","1.4",' stroke-dasharray="4 3"')
txt(DX+DS*0.135,DY+DS*BW/2+4,"travessa B","s-lbl","start","var(--ink)")
txt(DX+DS*0.175,DY+DS*BW*1.5+4,"montante A","s-lbl","start","var(--wood)")
li(DX+DS*0.17,DY+DS*BW*1.5,DX+DS*0.168,DY+DS*BW*1.5,"var(--wood)","1")
for i,l in enumerate(["DOIS di&#226;metros de broca, e a ordem importa:","",
  "&#183; na TRAVESSA, furo passante de 4 mm","  (o parafuso tem que passar solto)","",
  "&#183; no MONTANTE, furo-guia de 2,5 mm","  entrando uns 15 mm","",
  "&#201; isso que puxa as duas pe&#231;as uma contra a","outra. Furo fino nas duas deixa a junta aberta.","",
  "DOIS parafusos por junta, na diagonal.","Um s&#243; deixa a travessa girar."]):
    txt(DX,DY+DS*BW*2.15+i*16,l,"s-dimt","start")
tg("B",DX+DS*0.02+DS*0.06,DY+DS*BW/2,"var(--ink)",11)
tg("A",DX+DS*0.145,DY+DS*BW*1.5,"var(--wood)",11)

# ============ PAINEL 3 : AS DUAS EMPILHADAS ============
EX=790.0; EY=600.0; ES=128.0
txt(EX-130,EY-48,"3 &#183; DUAS ESCADAS POR TRAP","s-dimt","start","var(--ink)",' letter-spacing="1.5"')
txt(EX-130,EY-28,"Empilhadas d&#227;o 2,40 m. A tala C emenda as duas na hora de instalar.","s-dimt","start")
for k in (0,1):
    y0=EY+ES*1.20*k
    for x0 in (0.0,0.60-BW):
        rc(EX+ES*x0,y0,ES*BW,ES*1.20,"url(#gr)","var(--wood)","1.6")
    for z0 in (0.0,0.60-BW/2,1.20-BW):
        rc(EX,y0+ES*z0,ES*0.60,ES*BW,"var(--wood)","var(--ink)","1.4",' fill-opacity=".9"')
for x0 in (0.0,0.60-BW):
    rc(EX+ES*x0-3,EY+ES*1.20-ES*0.15,ES*BW+6,ES*0.30,"none","var(--flag)","2",' stroke-dasharray="6 4"')
tg("C",EX+ES*0.30,EY+ES*1.20,"var(--flag)",11)
dimV(EY,EY+ES*2.40,EX-34,"2,40")
txt(EX+ES*0.60+18,EY+ES*1.20+4,"tala C dos dois lados","s-dimt","start","var(--flag)")

# ============ PAINEL 4 : LISTA ============
LX=1180.0; yy=150.0
def head(t,col="var(--ink)"):
    global yy
    txt(LX,yy,t,"s-dimt","start",col,' letter-spacing="1.5"'); yy+=10
    li(LX,yy,LX+330,yy,"var(--mute)","1"); yy+=26
def row(k,v,col=None,tag=None,tcol="var(--wood)"):
    global yy
    if tag: tg(tag,LX+11,yy-4,tcol,11); txt(LX+30,yy,k,"s-dimt","start")
    else: txt(LX,yy,k,"s-dimt","start")
    txt(LX+330,yy,v,"s-dimt","end",col); yy+=21
head("AS PE&#199;AS")
row("Montante &#183; 1,20 m","8 pe&#231;as",None,"A","var(--wood)")
row("Travessa &#183; 0,60 m","12 pe&#231;as",None,"B","var(--ink)")
row("Tala de emenda &#183; 0,30 m","4 pe&#231;as",None,"C","var(--flag)")
yy+=4; li(LX,yy,LX+330,yy,"var(--rule)","1"); yy+=20
row("Ripa total","18,00 m","var(--ray)")
row("Comprar / separar","6 barras de 3 m","var(--ray)")
yy+=6
for l in ["S&#243; existem dois comprimentos de corte: 1,20 e 0,60.","A medida da ripa n&#227;o muda nada &#8212; como a travessa","vai POR CIMA, a largura da ripa n&#227;o entra na conta."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=22
head("O QUE USAR","var(--flag)")
for k,v in [("Parafuso &#183; bitola","3,5 mm"),("Parafuso &#183; comprimento","travessa + 20 mm"),
            ("Quantidade","60"),("Brocas","4 mm e 2,5 mm"),
            ("Serrote","o de 550 mm que voc&#234; comprou"),("Esquadro ou gabarito","obrigat&#243;rio")]:
    row(k,v)
yy+=8
for l in ["Antes de cortar: tire TODO prego, parafuso e","cantoneira das ripas. Um s&#243; escondido acaba com","os dentes do serrote em uma passada."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16
yy+=22
head("A ORDEM","var(--ray)")
for i,l in enumerate(["Limpar as ripas e escolher as retas para os A",
                      "Marcar todos os cortes antes de serrar",
                      "Cortar 8 &#215; 1,20 e 12 &#215; 0,60",
                      "Furar as travessas fora do lugar",
                      "Montar B1 e B3 nas pontas, conferir a diagonal",
                      "S&#243; ent&#227;o parafusar B2 no meio",
                      "Repetir mais tr&#234;s vezes"],1):
    tg(str(i),LX+11,yy-4,"var(--ray)",10.5); txt(LX+30,yy,l,"s-dimt","start"); yy+=23

txt(96,58,"MANUAL DE MONTAGEM &#183; ESTRUTURA DOS BASS TRAPS &#183; FASE 1 DE 2","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(96,78,"Quatro escadas de 0,60 &#215; 1,20 &#183; duas por trap &#183; a l&#227; e o tecido entram na fase 2","s-dimt","start")
open("_montagem.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1560 940">\n'+"\n".join(o)+'\n</svg>')
print("ok")
