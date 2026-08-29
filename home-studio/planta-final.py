# -*- coding: utf-8 -*-
import math
W,L,H = 2.75, 3.70, 2.65
DW=0.82; FX=W-DW; DZ=2.10; TRAP=0.424
SEP=0.770; EAR=1.587; BAF=0.92; ML=W/2-SEP/2; MR=W/2+SEP/2
TVW=1.08
MIRS=1.199          # espelho lateral
MIRC=1.254          # espelho do teto
BLK0,BLK1=0.577,1.777
S=205.0; OX=175.0; OY=120.0
def X(x): return OX+S*x
def Y(y): return OY+S*y
o=[]
def a(s): o.append("  "+s)
def rect(x0,x1,y0,y1,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'
      %(X(x0),Y(y0),X(x1)-X(x0),Y(y1)-Y(y0),fill,stroke,sw,extra))
def poly(pts,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<polygon points="%s" fill="%s" stroke="%s" stroke-width="%s"%s/>'
      %(" ".join("%.1f,%.1f"%(X(p[0]),Y(p[1])) for p in pts),fill,stroke,sw,extra))
def line(p,q,col,sw="1",extra=""):
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="%s"%s/>'%(X(p[0]),Y(p[1]),X(q[0]),Y(q[1]),col,sw,extra))
def txt(x,y,s,cls="s-lbl",anch="middle",fill=None,extra="",px=False):
    f=' fill="%s"'%fill if fill else ""
    xx,yy=(x,y) if px else (X(x),Y(y))
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,xx,yy,anch,f,extra,s))
def arcpl(cx,cy,r,a0,a1,col,sw="1.6",dash=' stroke-dasharray="9 6"'):
    pts=[("%.1f,%.1f"%(X(cx+r*math.cos(a0+(a1-a0)*i/40.)),Y(cy+r*math.sin(a0+(a1-a0)*i/40.)))) for i in range(41)]
    a('<polyline points="%s" fill="none" stroke="%s" stroke-width="%s"%s/>'%(" ".join(pts),col,sw,dash))
def tag(n,x,y,col="var(--ink)"):
    a('<circle cx="%.1f" cy="%.1f" r="11.5" fill="%s" stroke="var(--paper)" stroke-width="2"/>'%(X(x),Y(y),col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(X(x),Y(y)+4,n))
def dimH(x0,x1,ypx,label,col=None):
    c=' stroke="%s"'%col if col else ""
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(X(x0),ypx,X(x1),ypx,c))
    for xx in (x0,x1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(X(xx),ypx-5,X(xx),ypx+5,c))
    txt((X(x0)+X(x1))/2, ypx-8, label,"s-dimt","middle",col,px=True)
def dimV(y0,y1,xpx,label,col=None):
    c=' stroke="%s"'%col if col else ""
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(xpx,Y(y0),xpx,Y(y1),c))
    for yy in (y0,y1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(xpx-5,Y(yy),xpx+5,Y(yy),c))
    f=' fill="%s"'%col if col else ""
    a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle"%s transform="rotate(-90 %.1f %.1f)">%s</text>'
      %(xpx-6,(Y(y0)+Y(y1))/2,f,xpx-6,(Y(y0)+Y(y1))/2,label))

a('<defs>')
a('<pattern id="wl" width="9" height="9" patternUnits="userSpaceOnUse"><circle cx="4.5" cy="4.5" r="2.3" fill="none" stroke="var(--ray)" stroke-width=".95"/></pattern>')
a('<pattern id="hw" width="8" height="8" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="8" stroke="var(--mute)" stroke-width="1"/></pattern>')
a('<pattern id="rug" width="7" height="7" patternUnits="userSpaceOnUse"><line x1="0" y1="7" x2="7" y2="0" stroke="var(--mute)" stroke-width=".6"/></pattern>')
a('</defs>')

# alvenaria
for x0,x1,y0,y1 in ((-0.15,W+0.15,-0.15,0),(-0.15,0,0,L),(W,W+0.15,0,L),(-0.15,W+0.15,L,L+0.15)):
    rect(x0,x1,y0,y1,"url(#hw)","none","0",' opacity=".5"')
# tapete
rect(0.375,W-0.375,0.50,3.50,"url(#rug)","var(--mute)","1",' opacity=".42"')
# paredes
line((0,0),(FX,0),"var(--ink)","3"); line((0,0),(0,L),"var(--ink)","3"); line((W,0),(W,L),"var(--ink)","3")
line((FX,0),(W,0),"var(--wood)","2.4",' stroke-dasharray="7 5"')
arcpl(W,0,DW,math.pi,math.pi/2,"var(--flag)"); line((W,0),(W-0.03,DW),"var(--flag)","4")
# fundo de vidro
line((0,L),(W,L),"var(--glass)","5")
txt(W/2,L+0.115,"PORTA DE VIDRO &#183; parede do fundo inteira","s-dimt",fill="var(--glass)")

# armario, antes dos cantos
rect(0,1.20,L-0.42,L,"var(--sunk)","var(--wood)","2")
txt(0.60,L-0.26,"ARM&#193;RIO","s-dimt",fill="var(--wood)")
txt(0.60,L-0.14,"s&#243; at&#233; 1,35 de altura","s-dimt",fill="var(--wood)")
# bass traps
poly([(0,TRAP),(0,0),(TRAP,0)],"url(#wl)","var(--ray)","2.6"); tag("A",0.145,0.145,"var(--ray)")
poly([(W,TRAP),(W,0),(W-TRAP,0)],"none","var(--flag)","2",' stroke-dasharray="7 5"'); tag(2,W-0.135,0.135,"var(--flag)")
poly([(0,L-TRAP),(0,L),(TRAP,L)],"none","var(--flag)","2",' stroke-dasharray="7 5"'); tag(3,0.135,L-0.135,"var(--flag)")
poly([(W,L-TRAP),(W,L),(W-TRAP,L)],"url(#wl)","var(--ray)","2.6"); tag("B",W-0.145,L-0.145,"var(--ray)")
# bloco frontal
rect(BLK0,BLK1,0,0.10,"url(#wl)","var(--ray)","2"); tag(4,BLK0+0.16,0.05,"var(--ray)")
# TV
rect(W/2-TVW/2,W/2+TVW/2,0,0.055,"var(--felt)","var(--ink)","1.4"); tag(5,W/2+0.42,0.028,"var(--felt)")
# modulos de ponto-espelho
rect(0,0.10,MIRS-0.30,MIRS+0.30,"url(#wl)","var(--ray)","2"); tag(6,0.05,MIRS,"var(--ray)")
rect(W-0.10,W,MIRS-0.30,MIRS+0.30,"url(#wl)","var(--ray)","2"); tag(6,W-0.05,MIRS,"var(--ray)")
# bancada
rect(0.30,FX,0.70,1.30,"var(--surface)","var(--mute)","1.6",' stroke-dasharray="7 5"')
txt(0.42,1.21,"bancada","s-dimt",anch="start")
for cx in (ML,MR):
    rect(cx-0.111,cx+0.111,BAF-0.222,BAF,"var(--felt)","var(--ink)","1.6")
    a('<circle cx="%.1f" cy="%.1f" r="4" fill="var(--paper)"/>'%(X(cx),Y(BAF-0.03)))
# nuvem, por cima da bancada
rect(W/2-0.90,W/2+0.90,MIRC-0.30,MIRC+0.30,"none","var(--ray)","2",' stroke-dasharray="9 5"')
txt(W/2-0.90+0.30,MIRC+0.235,"NUVEM ACIMA","s-dimt",fill="var(--ray)")
tag(7,W/2-0.90+0.10,MIRC-0.18,"var(--ray)")
# gobo
rect(W/2-0.60,W/2+0.60,L-0.16,L-0.06,"url(#wl)","var(--ray)","1.6",' opacity=".8"'); tag(8,W/2+0.72,L-0.11,"var(--ray)")
# armario
line((TRAP*0.55,TRAP*0.55),(W-TRAP*0.55,L-TRAP*0.55),"var(--ray)","1.2",' stroke-dasharray="3 8" opacity=".7"')
# ouvinte + raios
for cx,mx in ((ML,1.131),(MR,W-1.131)):
    line((cx,BAF),(W/2,EAR),"var(--flag)","1.9")
    line((cx,BAF),(mx,0),"var(--ray)","1.3",' stroke-dasharray="6 4"'); line((mx,0),(W/2,EAR),"var(--ray)","1.3",' stroke-dasharray="6 4"')
    a('<circle cx="%.1f" cy="%.1f" r="5.5" fill="var(--flag)"/>'%(X(mx),Y(0)))
for wx in (0,W):
    line((ML if wx==0 else MR,BAF),(wx,MIRS),"var(--ray)","1.3",' stroke-dasharray="6 4"')
    line((wx,MIRS),(W/2,EAR),"var(--ray)","1.3",' stroke-dasharray="6 4"')
a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="var(--surface)" stroke="var(--ink)" stroke-width="2.4"/>'%(X(W/2),Y(EAR),0.085*S))
line((W/2,-0.12),(W/2,L+0.10),"var(--flag)","1.3",' stroke-dasharray="7 6"')
txt(W/2,-0.185,"eixo &#183; 1,375","s-dimt",fill="var(--flag)")
txt(W/2,EAR+0.20,"orelha &#183; 1,59 m","s-dimt")
txt(ML,BAF+0.135,"0,99","s-dimt"); txt(MR,BAF+0.135,"1,76","s-dimt")

dimH(0,W,Y(L)+112,"2,75")
dimH(0,TRAP,Y(L)+84,"0,42"); dimH(TRAP,FX,Y(L)+84,"1,51 de parede livre"); dimH(FX,W,Y(L)+84,"0,82")
dimV(0,L,X(W)+150,"3,70")
dimV(0,BAF,X(W)+66,"0,92"); dimV(BAF,EAR,X(W)+66,"0,67"); dimV(EAR,L,X(W)+66,"2,11")
dimV(MIRS-0.30,MIRS+0.30,X(0)-58,"0,60")
txt(56,44,"PLANTA &#183; 2,75 &#215; 3,70 m &#183; P&#201;-DIREITO 2,65 &#183; GEOMETRIA FECHADA","s-dimt","start","var(--ink)",' letter-spacing="1.6"',px=True)
txt(56,64,"Cotas em metros &#183; laranja = som direto e espelhos &#183; verde tracejado = caminho das reflex&#245;es precoces","s-dimt","start",None,"",px=True)

LX=X(W)+215; yy=150
items=[("A","var(--ray)","Bass trap &#183; CONSTRUIR AGORA","frontal esquerdo &#183; 0,42 &#215; 0,42 &#215; 2,40"),
 ("B","var(--ray)","Bass trap &#183; CONSTRUIR AGORA","traseiro direito &#183; na diagonal do A"),
 ("2","var(--flag)","Canto frontal direito &#183; bloqueado","a porta &#183; s&#243; 0,42 &#215; 0,55 acima de 2,10"),
 ("3","var(--flag)","Canto traseiro esquerdo &#183; meio livre","o arm&#225;rio s&#243; vai at&#233; ~1,35 &#183; vaga do 3&#186; trap"),
 ("4","var(--ray)","Bloco da parede frontal","4 m&#243;dulos &#183; x 0,58 a 1,78 &#183; z 1,45 a 2,65"),
 ("5","var(--felt)","TV 48\"","x 0,835 a 1,915 &#8212; centralizada, com 1,5 cm de folga"),
 ("6","var(--ray)","Ponto-espelho lateral","centro em y = 1,20 &#183; um de cada lado, mesma cota"),
 ("7","var(--ray)","Nuvem de teto","1,80 &#215; 0,60 &#183; centro em y = 1,25, se o ventilador deixar"),
 ("8","var(--ray)","Gobo m&#243;vel","1,20 &#215; 1,20 &#183; s&#243; na frente do vidro na hora de mixar")]
txt(LX,yy,"OS DOIS TRAPS NA DIAGONAL","s-dimt","start","var(--ink)",' letter-spacing="1.5"',px=True); yy+=10
a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+350,yy)); yy+=26
for n,c_,t1,t2 in items:
    a('<circle cx="%.1f" cy="%.1f" r="11.5" fill="%s"/>'%(LX+11,yy-4,c_))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(LX+11,yy,n))
    txt(LX+32,yy-6,t1,"s-leg","start","var(--ink)",px=True); txt(LX+32,yy+9,t2,"s-dimt","start",None,"",px=True)
    yy+=42
yy+=10; a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+350,yy)); yy+=26
txt(LX,yy,"AS COTAS QUE N&#195;O SE MEXEM","s-dimt","start","var(--flag)",' letter-spacing="1.5"',px=True); yy+=22
for l in ["Dois cantos inteiros: frontal esquerdo e traseiro","direito. &#201; onde v&#227;o os dois traps de agora.","",
          "Os outros dois est&#227;o ocupados s&#243; embaixo &#8212; a porta","at&#233; 2,10 e o arm&#225;rio at&#233; ~1,35. Sobram 0,55 e 1,30 m","de canto alto, guardados para os pr&#243;ximos traps.","",
          "Para o modo, tanto faz qual canto: todos s&#227;o ventre","de press&#227;o. Na diagonal os dois ainda se atrapalham","menos, porque n&#227;o dividem a mesma regi&#227;o de campo.","",
          "Baffle a 0,92 m: a 5 mm do n&#243; de 92,7 Hz.","Tweeter e orelha a 1,10 m. Separa&#231;&#227;o 0,77 m."]:
    txt(LX,yy,l,"s-dimt","start",None,"",px=True); yy+=17
open("_planta-final.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1420 1010">\n'+"\n".join(o)+'\n</svg>')
print("ok")
