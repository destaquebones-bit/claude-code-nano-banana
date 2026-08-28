# -*- coding: utf-8 -*-
import math
W=2.75; DW=0.88; DX0=W-DW; TR=0.424
SEP,EAR,BAF=0.770,1.587,0.92
ML,MR=W/2-SEP/2,W/2+SEP/2
MIRX=ML+(BAF/(BAF+EAR))*(W/2-ML)
DEEP=1.88
S=270.0; OX=150.0; OY=150.0
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
def arcpl(cx,cy,r,a0,a1,col,sw="1.7",dash=' stroke-dasharray="9 6"'):
    pts=[]
    for i in range(41):
        t=a0+(a1-a0)*i/40.0
        pts.append("%.1f,%.1f"%(X(cx+r*math.cos(t)),Y(cy+r*math.sin(t))))
    a('<polyline points="%s" fill="none" stroke="%s" stroke-width="%s"%s/>'%(" ".join(pts),col,sw,dash))
def tag(n,x,y,col="var(--ink)"):
    a('<circle cx="%.1f" cy="%.1f" r="11" fill="%s" stroke="var(--paper)" stroke-width="2"/>'%(X(x),Y(y),col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(X(x),Y(y)+4,n))

a('<defs>')
a('<pattern id="wl2" width="9" height="9" patternUnits="userSpaceOnUse"><circle cx="4.5" cy="4.5" r="2.3" fill="none" stroke="var(--ray)" stroke-width=".95"/></pattern>')
a('<pattern id="hw" width="8" height="8" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="8" stroke="var(--mute)" stroke-width="1"/></pattern>')
a('</defs>')
rect(-0.15,W+0.15,-0.15,0,"url(#hw)","none","0",' opacity=".5"')
rect(-0.15,0,0,DEEP,"url(#hw)","none","0",' opacity=".5"')
rect(W,W+0.15,0,DEEP,"url(#hw)","none","0",' opacity=".5"')
line((0,0),(DX0,0),"var(--ink)","3"); line((0,0),(0,DEEP),"var(--ink)","3"); line((W,0),(W,DEEP),"var(--ink)","3")
line((DX0,0),(W,0),"var(--wood)","2.4",' stroke-dasharray="7 5"')
# arcos: A dobradica na parede direita ; B dobradica no batente
arcpl(W,0,DW,math.pi,math.pi/2,"var(--flag)")
line((W,0),(W-0.03,DW),"var(--flag)","4")
arcpl(DX0,0,DW,0,math.pi/2,"var(--mute)","1.5",' stroke-dasharray="4 6"')
line((DX0,0),(DX0+0.03,DW),"var(--mute)","3",' stroke-dasharray="5 4"')
tag("A",W-0.30,0.78,"var(--flag)"); tag("B",DX0+0.68,0.40,"var(--mute)")
# bass traps
poly([(0,TR),(0,0),(TR,0)],"url(#wl2)","var(--ray)","2")
poly([(W,TR),(W,0),(W-TR,0)],"none","var(--flag)","2",' stroke-dasharray="8 5"')
tag(1,0.14,0.14,"var(--ray)"); tag(3,W-0.14,0.14,"var(--flag)")
# painel frontal + TV
rect(0.547,1.747,0,0.10,"none","var(--ray)","1.8",' stroke-dasharray="7 5"')
txt(1.147,-0.115,"bloco de 4 m&#243;dulos &#8212; ACIMA da TV","s-dimt",fill="var(--ray)")
tag(4,0.62,0.05,"var(--ray)")
rect(0.790,1.870,0,0.055,"var(--felt)","var(--ink)","1.4"); tag(5,1.60,0.028,"var(--felt)")
# bancada
rect(0.30,DX0,0.70,1.30,"var(--surface)","var(--mute)","1.6",' stroke-dasharray="7 5"')
txt(0.42,1.22,"bancada","s-dimt",anch="start")
for cx in (ML,MR):
    rect(cx-0.111,cx+0.111,BAF-0.222,BAF,"var(--felt)","var(--ink)","1.6")
    a('<circle cx="%.1f" cy="%.1f" r="4" fill="var(--paper)"/>'%(X(cx),Y(BAF-0.03)))
a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="var(--surface)" stroke="var(--ink)" stroke-width="2.2"/>'%(X(W/2),Y(EAR),0.085*S))
for cx,mx in ((ML,MIRX),(MR,W-MIRX)):
    line((cx,BAF),(mx,0),"var(--ray)","1.5",' stroke-dasharray="6 4"')
    line((mx,0),(W/2,EAR),"var(--ray)","1.5",' stroke-dasharray="6 4"')
    line((cx,BAF),(W/2,EAR),"var(--flag)","1.8")
    a('<circle cx="%.1f" cy="%.1f" r="6.5" fill="var(--flag)"/>'%(X(mx),Y(0)))
line((W/2,-0.12),(W/2,DEEP),"var(--flag)","1.3",' stroke-dasharray="7 6"')
txt(W/2,-0.235,"centro real 1,375","s-dimt",fill="var(--flag)")
txt(MIRX-0.10,-0.045,"espelho 1,13","s-dimt",fill="var(--ray)",anch="end")
txt(W-MIRX+0.10,-0.045,"1,62","s-dimt",fill="var(--ray)",anch="start")
txt(ML,BAF+0.14,"0,99","s-dimt"); txt(MR,BAF+0.14,"1,76","s-dimt")
txt(W/2,EAR+0.22,"orelha &#183; 1,59 m","s-dimt")
txt(52,44,"PLANTA DO TER&#199;O FRONTAL &#183; LARGURA 2,75 m &#183; O CONFLITO DA PORTA","s-dimt","start","var(--ink)",' letter-spacing="1.6"',px=True)

LX=X(W)+120
yy=170
txt(LX,yy,"OS DOIS LADOS DE DOBRADI&#199;A","s-dimt","start","var(--ink)",' letter-spacing="1.5"',px=True); yy+=10
a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+360,yy)); yy+=30
for L,c,t1,t2,t3 in [("A","var(--flag)","Dobradi&#231;a na parede direita","A folha abre encostando na lateral.","Livra o monitor direito por 47 cm. &#201; a melhor."),
                     ("B","var(--mute)","Dobradi&#231;a no batente esquerdo","A folha varre para dentro da sala.","Passa a 4,7 cm do monitor direito. Aperta.")]:
    a('<circle cx="%.1f" cy="%.1f" r="12" fill="%s"/>'%(LX+12,yy-4,c))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%s</text>'%(LX+12,yy,L))
    txt(LX+36,yy-6,t1,"s-leg","start","var(--ink)",px=True)
    txt(LX+36,yy+10,t2,"s-dimt","start",None,"",px=True)
    txt(LX+36,yy+26,t3,"s-dimt","start",None,"",px=True)
    yy+=68
yy+=6
a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+360,yy)); yy+=28
txt(LX,yy,"MAS A CONCLUS&#195;O &#201; A MESMA","s-dimt","start","var(--flag)",' letter-spacing="1.5"',px=True); yy+=24
for l in ["Nos dois casos o arco de 0,88 m varre o canto","frontal direito inteiro at&#233; a altura da porta.","","O bass trap desse canto s&#243; existe ACIMA de 2,10 m","&#8212; e ali ele ainda serve, porque a press&#227;o modal","&#233; m&#225;xima justamente no encontro do canto","com o teto, que &#233; onde mora o modo de 132 Hz."]:
    txt(LX,yy,l,"s-dimt","start",None,"",px=True); yy+=17
yy+=14
txt(LX,yy,"3 = o trecho de trap que se perde","s-dimt","start","var(--flag)",px=True); yy+=17
txt(LX,yy,"1 = trap esquerdo, inteiro &#183; 4 = bloco de 4 m&#243;dulos","s-dimt","start",None,"",px=True); yy+=17
txt(LX,yy,"5 = TV &#183; verde tracejado = os dois raios de reflex&#227;o","s-dimt","start",None,"",px=True)
open("_planta275.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1420 780">\n'+"\n".join(o)+'\n</svg>')
print("ok")
