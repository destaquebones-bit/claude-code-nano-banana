# -*- coding: utf-8 -*-
"""Passo a passo de montagem da membrana + opcoes de acabamento."""
INK="#1a1a18"; MUTE="#8a8579"; RULE="#ddd9cd"; ORANGE="#c2703a"; BLUE="#1670b0"
WOOD="#8a6a3f"; MDF="#7a6a52"; SOFT="#EADCC4"; PET="#3f7d5c"; SUNK="#f0eee7"
o=[]
def a_(s): o.append("  "+s)
def txt(x,y,s,cls="s-dimt",an="start",fill=None,ex=""):
    fl=' fill="%s"'%fill if fill else ""
    a_(f'<text class="{cls}" x="{x:.1f}" y="{y:.1f}" text-anchor="{an}"{fl}{ex}>{s}</text>')
def rc(x,y,w,h,f,st="none",sw="1",rx=0):
    a_(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{rx}" fill="{f}" stroke="{st}" stroke-width="{sw}"/>')
def li(x1,y1,x2,y2,c,w="1",ex=""):
    a_(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" stroke="{c}" stroke-width="{w}"{ex}/>')
def tg(n,x,y,c=ORANGE,r=13):
    a_(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r}" fill="{c}"/>')
    a_(f'<text class="s-tag" x="{x:.1f}" y="{y+4:.1f}" text-anchor="middle" fill="#faf9f5">{n}</text>')

txt(70,58,"MEMBRANA &#183; MONTAGEM PASSO A PASSO","s-dimt","start",INK,' letter-spacing="1.6"')
txt(70,80,"Quatro caixas iguais, todas sintonizadas em 63 Hz. C&#226;mara de 15 cm, face de MDF 8 mm.","s-dimt","start")

# ================= OITO PASSOS =================
SX,SY,SW,SH=70,120,392,236
passos=[
 ("Corte tudo antes de montar",
  ["Face MDF 8 mm e fundo compensado 10 mm no","tamanho da caixa. Moldura em t&#225;bua 2,5 &#215; 15 cm,","cortada em 90&#176; &#8212; sem ingletes."]),
 ("Monte a moldura no ch&#227;o plano",
  ["Ret&#226;ngulo de t&#225;bua em p&#233;. Fure, cole e","parafuse 2 parafusos por canto. Confira o","esquadro medindo as duas diagonais."]),
 ("Parafuse o fundo r&#237;gido",
  ["Compensado 10 mm por cima da moldura, parafuso","a cada 12 cm. &#201; ele que d&#225; rigidez &#8212; a caixa","toda depende disso."]),
 ("SELE POR DENTRO",
  ["Cord&#227;o de silicone neutro em TODAS as juntas","internas, por dentro da caixa. Passe o dedo","molhado. Uma fresta e a mola de ar vaza."]),
 ("Cole a l&#227; s&#243; no fundo",
  ["50 mm, colada apenas no compensado, ocupando","dois ter&#231;os. Sobram 10 cm de ar livre at&#233; a","face. Ela n&#227;o pode encostar na face."]),
 ("Feche a face",
  ["MDF 8 mm colado e parafusado a cada 10 cm em","toda a volta. Silicone na junta antes de","fechar. Depois disso a caixa n&#227;o abre mais."]),
 ("Acabamento",
  ["Pintura fosca ou tecido fino esticado. Nada","de espuma ou l&#227; NA FRENTE da face &#8212; isso","trava a chapa e mata a membrana."]),
 ("Instala&#231;&#227;o",
  ["Encostada na parede, apoiada no ch&#227;o ou","pendurada. Nunca no meio do v&#227;o: ela precisa","de press&#227;o, e press&#227;o &#233; m&#225;xima na parede."]),
]
def dib(i,x,y,w,h):
    """desenho esquematico de cada passo, em corte"""
    cx,cy=x+w/2,y+h/2; W,H=150,86
    X0,Y0=cx-W/2,cy-H/2
    if i==0:
        rc(X0,Y0+8,W,10,MDF); rc(X0,Y0+H-16,W,12,WOOD)
        for k in range(3): rc(X0+k*54,Y0+34,46,10,WOOD)
        txt(cx,Y0-4,"pe&#231;as soltas","s-dimt","middle",MUTE)
    elif i==1:
        rc(X0,Y0,10,H,WOOD); rc(X0+W-10,Y0,10,H,WOOD)
        rc(X0,Y0,W,10,WOOD); rc(X0,Y0+H-10,W,10,WOOD)
        for px,py in [(X0+5,Y0+5),(X0+W-5,Y0+5),(X0+5,Y0+H-5),(X0+W-5,Y0+H-5)]:
            a_(f'<circle cx="{px}" cy="{py}" r="3.4" fill="none" stroke="{ORANGE}" stroke-width="1.6"/>')
        li(X0+10,Y0+10,X0+W-10,Y0+H-10,MUTE,"1",' stroke-dasharray="4 4"')
        li(X0+W-10,Y0+10,X0+10,Y0+H-10,MUTE,"1",' stroke-dasharray="4 4"')
    elif i==2:
        rc(X0,Y0,10,H,WOOD); rc(X0+W-10,Y0,10,H,WOOD)
        rc(X0,Y0+H-12,W,12,SOFT,INK,"1.4")
        for k in range(5): a_(f'<circle cx="{X0+14+k*30}" cy="{Y0+H-6}" r="3.2" fill="none" stroke="{ORANGE}" stroke-width="1.6"/>')
    elif i==3:
        rc(X0,Y0,10,H,WOOD); rc(X0+W-10,Y0,10,H,WOOD); rc(X0,Y0+H-12,W,12,SOFT,INK,"1")
        for px,py in [(X0+10,Y0+H-12),(X0+W-10,Y0+H-12)]:
            a_(f'<circle cx="{px}" cy="{py}" r="7" fill="none" stroke="{ORANGE}" stroke-width="2.6"/>')
        txt(cx,Y0+H/2,"silicone","s-leg","middle",ORANGE)
    elif i==4:
        rc(X0,Y0,10,H,WOOD); rc(X0+W-10,Y0,10,H,WOOD); rc(X0,Y0+H-12,W,12,SOFT,INK,"1")
        rc(X0+10,Y0+H-40,W-20,28,PET)
        li(X0+10,Y0+6,X0+W-10,Y0+6,MUTE,"1",' stroke-dasharray="4 4"')
        txt(cx,Y0+30,"10 cm de ar","s-dimt","middle",MUTE)
    elif i==5:
        rc(X0,Y0,10,H,WOOD); rc(X0+W-10,Y0,10,H,WOOD); rc(X0,Y0+H-12,W,12,SOFT,INK,"1")
        rc(X0+10,Y0+H-40,W-20,28,PET); rc(X0,Y0,W,12,MDF,INK,"1.6")
        for k in range(6): a_(f'<circle cx="{X0+12+k*25}" cy="{Y0+6}" r="3.2" fill="none" stroke="{ORANGE}" stroke-width="1.6"/>')
    elif i==6:
        rc(X0,Y0,W,H,SUNK,RULE,"1")
        rc(X0+12,Y0+12,W-24,H-24,INK)
        txt(cx,Y0+H/2+5,"face acabada","s-dimt","middle","#faf9f5")
    else:
        rc(X0,Y0,8,H,MUTE)  # parede
        rc(X0+8,Y0+10,26,H-20,SUNK,INK,"1.4")
        li(X0+40,Y0+H/2,X0+W-6,Y0+H/2,ORANGE,"1.6")
        txt(X0+46,Y0+H/2-8,"press&#227;o m&#225;xima","s-dimt","start",ORANGE)
        txt(X0+46,Y0+H/2+16,"na parede","s-dimt","start",MUTE)
for i,(t1,ls) in enumerate(passos):
    col,row=i%4,i//4
    x=SX+col*SW; y=SY+row*SH
    tg(str(i+1),x+14,y+14)
    txt(x+36,y+19,t1,"s-leg","start",INK)
    dib(i,x+18,y+34,SW-56,116)
    for k,l in enumerate(ls): txt(x+36,y+174+k*17,l,"s-dimt","start")
    if col<3: li(x+SW-26,y+8,x+SW-26,y+SH-30,RULE,"1")

# ================= ACABAMENTO =================
AY=610
li(70,AY-26,1630,AY-26,RULE,"1")
txt(70,AY,"O ACABAMENTO MUDA A SINTONIA &#8212; ESCOLHA ANTES DE FECHAR A CAIXA","s-dimt","start",ORANGE,' letter-spacing="1.5"')
opts=[("Pintura fosca",   "6,0 kg/m&#178;","63 Hz","nenhuma","some na parede; combina com o gesso",INK),
      ("Tecido esticado", "6,2 kg/m&#178;","62 Hz","nenhuma","fecha a fam&#237;lia com os traps pretos",INK),
      ("Laminado / veneer","6,5 kg/m&#178;","61 Hz","c&#226;mara 14 cm","conversa com a bancada e o difusor",WOOD),
      ("Ripas de madeira","8,0 kg/m&#178;","55 Hz","c&#226;mara 11 cm","ecoa o difusor; exige recalcular",ORANGE)]
cx=70
txt(cx,AY+30,"acabamento","s-dimt","start",MUTE); txt(cx+250,AY+30,"massa","s-dimt","end",MUTE)
txt(cx+390,AY+30,"cai em","s-dimt","end",MUTE); txt(cx+560,AY+30,"corre&#231;&#227;o","s-dimt","end",MUTE)
txt(cx+600,AY+30,"por que","s-dimt","start",MUTE)
li(cx,AY+38,1630,AY+38,RULE,"1")
for k,(nm,mm,ff,corr,pq,col) in enumerate(opts):
    y=AY+62+k*26
    txt(cx,y,nm,"s-leg","start",col); txt(cx+250,y,mm,"s-dimt","end")
    txt(cx+390,y,ff,"s-dimt","end",ORANGE if k>1 else None)
    txt(cx+560,y,corr,"s-dimt","end",MUTE); txt(cx+600,y,pq,"s-dimt","start")
txt(cx,AY+186,"Qualquer coisa que voc&#234; cole na face vira massa e derruba a frequ&#234;ncia. Pintura e tecido fino s&#227;o desprez&#237;veis;","s-dimt","start")
txt(cx,AY+204,"laminado e ripa n&#227;o s&#227;o &#8212; nesses dois a c&#226;mara tem que encolher para compensar. Decida agora, n&#227;o depois.","s-dimt","start")

open("_montagem-membrana.svg","w").write('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1700 850">\n'+"\n".join(o)+'\n</svg>')
print("ok")
