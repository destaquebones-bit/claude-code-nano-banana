# -*- coding: utf-8 -*-
W,H = 2.75, 2.60
DW,DZ = 0.88, 2.10
DX0 = W-DW
TR  = 0.424
TVW,TVH,TVZ = 1.08,0.61,0.80
TVC = min(W/2, DX0-TVW/2)
BLKC= (TR+DX0)/2
SEP,EAR = 0.770,1.587
ML,MR = W/2-SEP/2, W/2+SEP/2
MIRX = ML + (0.92/(0.92+EAR))*(W/2-ML)

S=232.0; OX=150.0; OZ=740.0
def X(m): return OX+S*m
def Y(z): return OZ-S*z
o=[]
def a(s): o.append("  "+s)
def rect(x0,x1,z0,z1,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'
      %(X(x0),Y(z1),X(x1)-X(x0),Y(z0)-Y(z1),fill,stroke,sw,extra))
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def vtxt(x,z,s,cls="s-lbl",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" text-anchor="middle"%s%s transform="rotate(-90 %.1f %.1f)" x="%.1f" y="%.1f">%s</text>'
      %(cls,f,extra,X(x),Y(z),X(x),Y(z),s))
def dimH(x0,x1,y,label,col=None):
    c=' stroke="%s"'%col if col else ""
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(X(x0),y,X(x1),y,c))
    for xx in (x0,x1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(X(xx),y-5,X(xx),y+5,c))
    txt((X(x0)+X(x1))/2, y-8, label, "s-dimt","middle",col)
def dimV(z0,z1,x,label,col=None):
    c=' stroke="%s"'%col if col else ""
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(x,Y(z0),x,Y(z1),c))
    for zz in (z0,z1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"%s/>'%(x-5,Y(zz),x+5,Y(zz),c))
    f=' fill="%s"'%col if col else ""
    a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle"%s transform="rotate(-90 %.1f %.1f)">%s</text>'
      %(x-6,(Y(z0)+Y(z1))/2,f,x-6,(Y(z0)+Y(z1))/2,label))
def tag(n,x,z,col="var(--ink)"):
    a('<circle cx="%.1f" cy="%.1f" r="11" fill="%s" stroke="var(--paper)" stroke-width="2"/>'%(X(x),Y(z),col))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%d</text>'%(X(x),Y(z)+4,n))

a('<defs>')
a('<pattern id="tr" width="10" height="10" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="10" stroke="var(--ray)" stroke-width="1.7"/></pattern>')
a('<pattern id="wl" width="9" height="9" patternUnits="userSpaceOnUse"><circle cx="4.5" cy="4.5" r="2.3" fill="none" stroke="var(--ray)" stroke-width=".95"/></pattern>')
a('<pattern id="wo" width="9" height="9" patternUnits="userSpaceOnUse"><circle cx="4.5" cy="4.5" r="2.3" fill="none" stroke="var(--ray)" stroke-width=".7" opacity=".45"/></pattern>')
a('</defs>')

rect(0,W,0,H,"var(--sunk)","var(--ink)","2.5")
rect(0,TR,0,H,"url(#tr)","var(--ray)","1.8")                    # 1 trap esquerdo
rect(W-TR,W,DZ,H,"url(#tr)","var(--ray)","1.8")                 # 2 trap direito, so acima
rect(W-TR,W,0,DZ,"none","var(--flag)","1.6",' stroke-dasharray="8 5"')   # 3 trecho perdido
rect(DX0,W,0,DZ,"none","var(--wood)","2.6")                     # porta
for i in (0,1):
    for j in (0,1):
        rect(BLKC-0.60+0.60*i, BLKC+0.60*i, 1.40+0.60*j, 2.00+0.60*j, "url(#wl)","var(--ray)","1.6")
rect(TR, TVC-TVW/2, TVZ, TVZ+TVH, "url(#wo)","var(--ray)","1.2",' stroke-dasharray="5 4"')
rect(DX0, W-TR, DZ, H, "url(#wo)","var(--ray)","1.2",' stroke-dasharray="5 4"')
rect(TVC-TVW/2,TVC+TVW/2,TVZ,TVZ+TVH,"var(--felt)","var(--ink)","1.8")
rect(TVC-TVW/2+0.03,TVC+TVW/2-0.03,TVZ+0.03,TVZ+TVH-0.03,"var(--glass)","none","0",' fill-opacity=".26"')
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="var(--mute)" opacity=".12"/>'%(X(0.30),Y(0.75),X(DX0)-X(0.30),Y(0)-Y(0.75)))
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--mute)" stroke-width="2.5" stroke-dasharray="8 5"/>'%(X(0.30),Y(0.75),X(DX0),Y(0.75)))
for cx in (ML,MR):
    rect(cx-0.15,cx+0.15,0.75,0.87,"var(--mute)","var(--ink)","1.2",' fill-opacity=".5"')
    rect(cx-0.085,cx+0.085,0.87,1.155,"var(--felt)","var(--ink)","1.5")
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="none" stroke="var(--paper)" stroke-width="2"/>'%(X(cx),Y(0.975),0.05*S))
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="var(--paper)"/>'%(X(cx),Y(1.10),0.019*S))
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--flag)" stroke-width="1.8" stroke-dasharray="18 5 3 5"/>'%(X(-0.10),Y(1.10),X(W+0.04),Y(1.10)))
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--flag)" stroke-width="1.4" stroke-dasharray="7 6"/>'%(X(W/2),Y(-0.10),X(W/2),Y(H+0.16)))
for mx in (MIRX, W-MIRX):
    a('<circle cx="%.1f" cy="%.1f" r="8" fill="none" stroke="var(--flag)" stroke-width="2.4"/>'%(X(mx),Y(1.10)))
    a('<circle cx="%.1f" cy="%.1f" r="3" fill="var(--flag)"/>'%(X(mx),Y(1.10)))

tag(1,0.212,1.55,"var(--ray)")
tag(2,W-0.212,2.35,"var(--ray)")
tag(3,W-0.212,1.05,"var(--flag)")
tag(4,BLKC,2.30,"var(--ray)")
tag(5,TVC,1.10,"var(--felt)")
tag(6,DX0+0.23,2.35,"var(--ray)")
tag(7,0.60,1.10,"var(--ray)")
txt(X(W/2),Y(H)+ -16,"centro real 1,375","s-dimt","middle","var(--flag)")
txt(X(W)+26,Y(1.10)+4,"eixo tweeter + orelha &#183; 1,10","s-dimt","start","var(--flag)")
vtxt(DX0+0.44,1.05,"PORTA &#183; 0,88 &#215; 2,10","s-lbl","var(--wood)",' letter-spacing="1.4"')

dimH(0,W,838,"2,75 &#8212; MEDIDO HOJE")
dimH(0,TR,810,"0,42"); dimH(TR,DX0,810,"1,45 de parede livre"); dimH(DX0,W,810,"0,88")
dimH(BLKC-0.60,BLKC+0.60,784,"1,20 &#183; bloco"); 
dimV(0,TVZ,124,"0,80"); dimV(TVZ,TVZ+TVH,124,"0,61"); dimV(1.40,H,98,"1,20")
dimV(0,DZ,X(W)+236,"2,10"); dimV(DZ,H,X(W)+236,"0,50"); dimV(0,H,X(W)+282,"2,60 &#183; conferir")

# LEGENDA
LX=X(W)+320
items=[(1,"var(--ray)","Bass trap frontal esquerdo","inteiro, do piso ao teto &#183; 0,42 &#215; 2,60"),
 (2,"var(--ray)","Bass trap frontal direito","s&#243; o trecho acima da porta &#183; 0,42 &#215; 0,50"),
 (3,"var(--flag)","O que a porta rouba","2,10 m de canto que n&#227;o d&#225; para tratar"),
 (4,"var(--ray)","4 m&#243;dulos 0,60 &#215; 0,60 &#215; 0,10","x = 0,55 a 1,75 &#183; z = 1,40 a 2,60"),
 (5,"var(--felt)","A TV cobre os dois espelhos","x = 1,13 e 1,62, na altura do tweeter"),
 (6,"var(--ray)","Filete acima da porta","0,46 &#215; 0,50 &#183; opcional, fase 2"),
 (7,"var(--ray)","Faixa &#224; esquerda da TV","0,37 &#215; 0,61 &#183; opcional, fase 2")]
yy=150
txt(LX,yy,"O QUE VAI NA PAREDE","s-dimt","start","var(--ink)",' letter-spacing="1.5"'); yy+=10
a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+330,yy)); yy+=26
for n,c,t1,t2 in items:
    a('<circle cx="%.1f" cy="%.1f" r="11" fill="%s"/>'%(LX+11,yy-4,c))
    a('<text class="s-tag" x="%.1f" y="%.1f" text-anchor="middle" fill="var(--paper)">%d</text>'%(LX+11,yy,n))
    txt(LX+32,yy-6,t1,"s-leg","start","var(--ink)")
    txt(LX+32,yy+9,t2,"s-dimt","start")
    yy+=44
yy+=8
a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(LX,yy,LX+330,yy)); yy+=24
txt(LX,yy,"A MEDIDA QUE FALTA","s-dimt","start","var(--flag)",' letter-spacing="1.5"'); yy+=22
for l in ["Da parede DIREITA at&#233; o batente da porta.","&#201; ela que fixa os 1,45 m de parede livre,","o centro poss&#237;vel da TV e a separa&#231;&#227;o","m&#225;xima entre os monitores. Assumi 0,88 m."]:
    txt(LX,yy,l,"s-dimt","start"); yy+=16

txt(52,44,"PAREDE FRONTAL &#183; 2,75 &#215; 2,60 m &#183; VISTA DE DENTRO DA SALA","s-dimt","start","var(--ink)",' letter-spacing="1.6"')
txt(52,64,"Cotas em metros","s-dimt","start")
txt(52,872,"Hachura verde cheia = l&#227; de rocha 100 mm &#183; c&#237;rculos claros tracejados = filetes opcionais &#183; tracejado laranja = o que a porta impede","s-dimt","start")
open("_frente275.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1440 900">\n'+"\n".join(o)+'\n</svg>')
print("ok")
