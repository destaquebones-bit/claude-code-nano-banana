# -*- coding: utf-8 -*-
# Cartelas 1080x1920 com fundo transparente, identidade das pranchas do projeto.
import os,subprocess,sys
W,H=1080,1920
SAFE_Y=980          # centro vertical util (fora da UI do Instagram)
def esc(s): return s.replace("&","&amp;").replace("<","&lt;")
def card(name,lines,kind="linha"):
    o=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}">']
    # sombra suave para legibilidade sobre imagem
    o.append('<defs><filter id="sh" x="-30%" y="-30%" width="160%" height="160%">'
             '<feDropShadow dx="0" dy="3" stdDeviation="14" flood-color="#000" flood-opacity="0.72"/></filter></defs>')
    if kind=="hero":
        big,small=lines[0],lines[1:]
        y=SAFE_Y-60
        o.append(f'<text class="h1 flag" x="{W//2}" y="{y}" font-size="210" text-anchor="middle" filter="url(#sh)" letter-spacing="-4">{esc(big)}</text>')
        for i,l in enumerate(small):
            o.append(f'<text class="h2" x="{W//2}" y="{y+92+i*62}" font-size="46" text-anchor="middle" filter="url(#sh)">{esc(l)}</text>')
    elif kind=="dado":
        y=SAFE_Y-100
        o.append(f'<rect x="90" y="{y-72}" width="{W-180}" height="{92+62*len(lines)}" rx="6" fill="#1a1a18" fill-opacity="0.80"/>')
        o.append(f'<line x1="130" y1="{y-24}" x2="{W-130}" y2="{y-24}" stroke="#c2703a" stroke-width="3"/>')
        for i,l in enumerate(lines):
            fs=58 if i==0 else 40
            cls="h2" if i==0 else "sm"
            o.append(f'<text class="{cls}" x="130" y="{y+34+i*62}" font-size="{fs}">{esc(l)}</text>')
    else:
        y=SAFE_Y-(len(lines)-1)*40
        for i,l in enumerate(lines):
            o.append(f'<text class="h2" x="{W//2}" y="{y+i*80}" font-size="62" text-anchor="middle" filter="url(#sh)">{esc(l)}</text>')
    o.append('</svg>')
    open(f"_{name}.svg","w").write("\n".join(o))
    subprocess.run([sys.executable,"rendercard.py",f"_{name}.svg",f"{name}.png"],check=True)
    print("  ->",name+".png")

CARDS=[("c01_gancho",["\u221215 dB","Foi o que a medi\u00e7\u00e3o achou","na minha sala."],"hero"),
       ("c02_soum",["E s\u00f3 de um lado."],"linha"),
       ("c03_faixa",["75 a 275 Hz","O problema n\u00e3o era o agudo."],"dado"),
       ("c04_porta",["Culpa de um v\u00e3o de porta.","82 cm."],"linha"),
       ("c05_quadrado",["Quadrado de 42,4","Corta na diagonal."],"dado"),
       ("c06_angulo",["Nenhuma ripa cortada em \u00e2ngulo."],"linha"),
       ("c07_la",["6 placas de l\u00e3 de PET","em cada um."],"dado"),
       ("c08_antes",["ANTES"],"linha"),
       ("c09_depois",["DEPOIS"],"linha"),
       ("c10_sala",["A sala onde o EP foi feito."],"linha")]
for n,l,k in CARDS: card(n,l,k)
print("ok - %d cartelas"%len(CARDS))
