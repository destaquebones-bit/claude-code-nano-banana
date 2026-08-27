S=240.0
def X(m): return 140+S*m
def Y(z): return 700-S*z
o=[]
def a(x): o.append("  "+x)
def rect(x0,x1,z0,z1,fill,stroke="var(--ink)",sw="1",extra=""):
    a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s" stroke="%s" stroke-width="%s"%s/>'%(X(x0),Y(z1),X(x1)-X(x0),Y(z0)-Y(z1),fill,stroke,sw,extra))
def txt(x,y,s,cls="s-lbl",anch="start",fill=None,extra=""):
    f=' fill="%s"'%fill if fill else ""
    a('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s"%s%s>%s</text>'%(cls,x,y,anch,f,extra,s))
def dimH(x0,x1,y,label):
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(X(x0),y,X(x1),y))
    for xx in (x0,x1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(X(xx),y-6,X(xx),y+6))
    txt((X(x0)+X(x1))/2, y-9, label, "s-dimt","middle")
def dimV(z0,z1,x,label):
    a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(x,Y(z0),x,Y(z1)))
    for zz in (z0,z1): a('<line class="s-dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>'%(x-6,Y(zz),x+6,Y(zz)))
    a('<text class="s-dimt" x="%.1f" y="%.1f" text-anchor="middle" transform="rotate(-90 %.1f %.1f)">%s</text>'%(x-7,(Y(z0)+Y(z1))/2,x-7,(Y(z0)+Y(z1))/2,label))

a('<defs>')
a('<pattern id="tr" width="10" height="10" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="10" stroke="var(--ray)" stroke-width="1.5"/></pattern>')
a('<pattern id="tf" width="10" height="10" patternTransform="rotate(45)" patternUnits="userSpaceOnUse"><line x1="0" y1="0" x2="0" y2="10" stroke="var(--flag)" stroke-width="1.5"/></pattern>')
a('</defs>')
rect(0,3.0,0,2.60,"var(--sunk)","var(--ink)","2.5")
rect(0,0.424,0,2.60,"url(#tr)","var(--ray)","1.6")
rect(2.576,3.0,0,2.60,"url(#tf)","var(--flag)","1.8",' stroke-dasharray="9 5"')
rect(2.12,3.0,0,2.10,"none","var(--wood)","2.5")
for x0 in (0.90,1.50):
    rect(x0,x0+0.60,1.40,2.00,"var(--ray)","var(--ink)","1.3",' fill-opacity=".8"')
    rect(x0,x0+0.60,2.00,2.60,"var(--ray)","var(--ink)","1.3",' fill-opacity=".8"')
rect(0.96,2.04,0.80,1.40,"var(--felt)","var(--ink)","1.6")
rect(0.99,2.01,0.83,1.37,"var(--glass)","none","0",' fill-opacity=".30"')
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--mute)" stroke-width="2.5" stroke-dasharray="8 5"/>'%(X(0.55),Y(0.75),X(2.45),Y(0.75)))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="var(--mute)" opacity=".10"/>'%(X(0.55),Y(0.75),X(2.45)-X(0.55),Y(0)-Y(0.75)))
rect(1.35,1.65,0.75,0.94,"var(--mute)","var(--mute)","1.5",' fill-opacity=".25" stroke-dasharray="5 4"')
for cx in (1.12,1.88):
    rect(cx-0.15,cx+0.15,0.75,0.875,"var(--mute)","var(--ink)","1.2",' fill-opacity=".55"')
    rect(cx-0.085,cx+0.085,0.875,1.16,"var(--felt)","var(--ink)","1.5")
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="none" stroke="var(--paper)" stroke-width="2.2"/>'%(X(cx),Y(0.975),0.052*S))
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="var(--paper)"/>'%(X(cx),Y(1.10),0.020*S))
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--flag)" stroke-width="1.6" stroke-dasharray="16 5 3 5"/>'%(X(-0.20),Y(1.10),X(3.14),Y(1.10)))
# rotulos sobre as pecas
txt(X(1.50),Y(2.44),"4 M&#211;DULOS 0,60 &#215; 0,60 &#215; 0,10","s-big","middle","var(--paper)",' letter-spacing="1.2"')
txt(X(1.50),Y(2.22),"z = 1,40 a 2,60","s-lbl","middle","var(--paper)")
txt(X(1.50),Y(1.60),"cobrem os espelhos da parede frontal","s-lbl","middle","var(--paper)")
txt(X(1.50),Y(1.52),"x = 1,26 e 1,74","s-lbl","middle","var(--paper)")
txt(X(1.50),Y(1.24),"TV &#183; 0,80 a 1,40","s-big","middle","var(--paper)")
txt(X(1.50),Y(1.15),"centro em 1,10, na linha do olho","s-lbl","middle","var(--paper)")
a('<text class="s-lbl" fill="var(--ray)" text-anchor="middle" transform="rotate(-90 %.1f %.1f)" x="%.1f" y="%.1f" letter-spacing="1.4">BASS TRAP</text>'%(X(0.212),Y(1.95),X(0.212),Y(1.95)))
a('<text class="s-lbl" fill="var(--flag)" text-anchor="middle" transform="rotate(-90 %.1f %.1f)" x="%.1f" y="%.1f" letter-spacing="1.4">BASS TRAP &#8212; CONFLITA COM A PORTA</text>'%(X(2.79),Y(1.55),X(2.79),Y(1.55)))
a('<text class="s-lbl" fill="var(--wood)" text-anchor="middle" transform="rotate(-90 %.1f %.1f)" x="%.1f" y="%.1f" letter-spacing="1.4">PORTA</text>'%(X(2.30),Y(0.90),X(2.30),Y(0.90)))
txt(X(1.50),Y(0.55),"bancada &#183; borda de tr&#225;s a 0,70 m da parede","s-dimt","middle")
txt(X(1.50),Y(0.79),"MacBook","s-dimt","middle")
txt(X(1.12),Y(0.845),"x = 1,12","s-dimt","middle"); txt(X(1.88),Y(0.845),"x = 1,88","s-dimt","middle")
txt(X(3.02),Y(1.10)+16,"eixo tweeter","s-dimt","start","var(--flag)")
txt(X(3.02),Y(1.10)+30,"+ orelha &#183; 1,10","s-dimt","start","var(--flag)")
txt(X(0.50),Y(1.06),"suporte","s-dimt","middle"); txt(X(0.50),Y(0.98),"cheio de areia","s-dimt","middle")
a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--mute)" stroke-width="1"/>'%(X(0.72),Y(1.02),X(0.965),Y(0.82)))
# cotas
dimH(0,3.0,790,"3,00")
dimH(0,0.424,760,"0,42"); dimH(0.424,0.90,760,"0,48"); dimH(0.90,1.50,760,"0,60"); dimH(1.50,2.10,760,"0,60"); dimH(2.10,3.0,760,"0,90")
dimV(0,0.75,106,"0,75"); dimV(0.75,1.10,80,"0,35"); dimV(1.40,2.00,106,"0,60"); dimV(2.00,2.60,106,"0,60")
dimV(0.80,1.40,876,"TV 0,60"); dimV(0,2.60,946,"2,60")
txt(46,40,"ELEVA&#199;&#195;O DA PAREDE FRONTAL &#183; 3,00 &#215; 2,60 m &#183; VISTA DE DENTRO DA SALA","s-dimt","start","var(--ink)",' letter-spacing="1.4"')
txt(46,818,"Cotas em metros &#183; separa&#231;&#227;o entre tweeters 0,76 &#183; baffle a 0,92 da parede &#183; escuta a 1,58 &#183; hachura verde = bass trap atravessando o canto","s-dimt","start")
print("\n".join(o))
