import math
S=78.0; C=math.cos(math.radians(30)); OX=372.0; OY=252.0
def P(x,y,z): return (OX+(x-y)*C*S, OY+(x+y)*0.5*S - z*S)
def pts(l): return " ".join("%.1f,%.1f"%P(*p) for p in l)
out=[]
def add(s): out.append("  "+s)

def poly(l,cls="",fill=None,op=None,stroke=None,sw=None,extra=""):
    a='points="%s"'%pts(l)
    if cls: a+=' class="%s"'%cls
    if fill: a+=' fill="%s"'%fill
    if op is not None: a+=' fill-opacity="%s"'%op
    if stroke: a+=' stroke="%s"'%stroke
    if sw: a+=' stroke-width="%s"'%sw
    add("<polygon %s%s/>"%(a,extra))

def line(p1,p2,cls="s-hair",extra=""):
    a,b=P(*p1),P(*p2)
    add('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" class="%s"%s/>'%(a[0],a[1],b[0],b[1],cls,extra))

def box(x0,x1,y0,y1,z0,z1,fill,op=(1.0,.78,.58),stroke="var(--ink)",sw="0.8"):
    # top, +Y face, +X face
    poly([(x0,y0,z1),(x1,y0,z1),(x1,y1,z1),(x0,y1,z1)],fill=fill,op=op[0],stroke=stroke,sw=sw)
    poly([(x0,y1,z1),(x1,y1,z1),(x1,y1,z0),(x0,y1,z0)],fill=fill,op=op[1],stroke=stroke,sw=sw)
    poly([(x1,y0,z1),(x1,y1,z1),(x1,y1,z0),(x1,y0,z0)],fill=fill,op=op[2],stroke=stroke,sw=sw)

def prism(tri,z0,z1,fill,op=(1.0,.72,.55)):
    a,b,c=tri
    poly([(a[0],a[1],z1),(b[0],b[1],z1),(c[0],c[1],z1)],fill=fill,op=op[0],stroke="var(--ink)",sw="0.8")
    for (p,q),o in zip([(a,b),(b,c),(c,a)],[op[1],op[2],op[1]]):
        poly([(p[0],p[1],z1),(q[0],q[1],z1),(q[0],q[1],z0),(p[0],p[1],z0)],fill=fill,op=o,stroke="var(--ink)",sw="0.8")

RAY="var(--ray)"; MDF="var(--mdf)"; DK="var(--felt)"; OBJ="var(--sunk)"; MUTE="var(--mute)"

# ---- floor + rug ----
poly([(0,0,0),(3,0,0),(3,4.8,0),(0,4.8,0)],fill="var(--sunk)",op=.55,stroke="var(--rule)",sw="1")
poly([(.5,.6,0),(2.5,.6,0),(2.5,3.6,0),(.5,3.6,0)],fill=MDF,op=.28,stroke=MDF,sw="1.2")
# ---- walls (front + left solid) ----
poly([(0,0,0),(3,0,0),(3,0,2.6),(0,0,2.6)],fill="var(--surface)",op=.92,stroke="var(--ink)",sw="1.4")
poly([(0,0,0),(0,4.8,0),(0,4.8,2.6),(0,0,2.6)],fill="var(--surface)",op=.8,stroke="var(--ink)",sw="1.4")
# ---- wireframes: ceiling, right wall, rear wall ----
for a,b in [((0,0,2.6),(3,0,2.6)),((3,0,2.6),(3,4.8,2.6)),((3,4.8,2.6),(0,4.8,2.6)),((0,4.8,2.6),(0,0,2.6)),
            ((3,0,0),(3,4.8,0)),((3,4.8,0),(3,4.8,2.6)),((3,0,0),(3,0,2.6)),((0,4.8,0),(0,4.8,2.6)),((0,4.8,0),(3,4.8,0))]:
    line(a,b,"s-ghost")

# ---- bass traps ----
L=0.424
for tri in [[(0,0),(L,0),(0,L)],[(3,0),(3-L,0),(3,L)],[(0,4.8),(L,4.8),(0,4.8-L)],[(3,4.8),(3-L,4.8),(3,4.8-L)]]:
    prism(tri,0,2.6,RAY)

# ---- front wall: 3 modules, TV, door ----
for x0 in (0.30,0.90,1.50):
    box(x0,x0+0.60,0,0.10,1.40,2.00,RAY); box(x0,x0+0.60,0,0.10,2.00,2.60,RAY)
box(0.96,2.04,0,0.08,0.80,1.40,MUTE,op=(1,.9,.7))
poly([(2.12,0,0),(3,0,0),(3,0,2.1),(2.12,0,2.1)],fill="var(--wood)",op=.3,stroke="var(--wood)",sw="1.6")

# ---- side wall panels (10 cm air gap) + skylines ----
box(0.10,0.20,0.90,1.50,0.55,1.75,RAY)
box(2.80,2.90,0.90,1.50,0.55,1.75,RAY)
box(0.00,0.15,3.00,3.60,1.00,1.60,MDF)
box(2.85,3.00,3.00,3.60,1.00,1.60,MDF)

# ---- monitors on stands ----
for cx in (1.12,1.88):
    box(cx-0.15,cx+0.15,0.70,0.98,0.75,0.875,DK,op=(1,.92,.72))
    box(cx-0.085,cx+0.085,0.70,0.922,0.875,1.16,DK,op=(1,.92,.72))
# ---- desk ----
box(0.55,2.45,0.70,1.30,0.72,0.75,OBJ)
line((0.61,0.76,0.72),(0.61,0.76,0),"s-hair"); line((2.39,0.76,0.72),(2.39,0.76,0),"s-hair")
line((0.61,1.24,0.72),(0.61,1.24,0),"s-hair"); line((2.39,1.24,0.72),(2.39,1.24,0),"s-hair")
# ---- chair + listener ----
box(1.34,1.66,1.65,2.00,0,0.45,OBJ)
box(1.36,1.64,1.94,2.00,0.45,1.00,OBJ)
h=P(1.5,1.58,1.25)
add('<circle cx="%.1f" cy="%.1f" r="13" fill="var(--ink)"/>'%h)
add('<circle cx="%.1f" cy="%.1f" r="19" fill="none" stroke="var(--ink)" stroke-width="1" opacity=".3"/>'%h)

# ---- rear: curtain + gobo + glass ----
poly([(0.20,4.72,0),(2.80,4.72,0),(2.80,4.72,2.55),(0.20,4.72,2.55)],fill="var(--mute)",op=.28,stroke="var(--mute)",sw="1")
poly([(0.45,4.8,0),(2.55,4.8,0),(2.55,4.8,2.1),(0.45,4.8,2.1)],fill="var(--glass-fill)",op=.5,stroke="var(--glass)",sw="1.2")
box(0.90,2.10,4.45,4.55,0.35,1.55,RAY)

# ---- ceiling fan ----
fc=(1.5,2.20)
ring=[(fc[0]+0.6*math.cos(t*math.pi/16), fc[1]+0.6*math.sin(t*math.pi/16), 2.32) for t in range(32)]
poly(ring,fill="var(--flag)",op=.10,stroke="var(--flag)",sw="1.2",extra=' stroke-dasharray="5 4"')
for ang in (90,210,330):
    a=math.radians(ang)
    poly([(fc[0],fc[1],2.34),(fc[0]+0.58*math.cos(a)-0.07*math.sin(a),fc[1]+0.58*math.sin(a)+0.07*math.cos(a),2.34),
          (fc[0]+0.58*math.cos(a)+0.07*math.sin(a),fc[1]+0.58*math.sin(a)-0.07*math.cos(a),2.34)],
         fill="var(--flag)",op=.4,stroke="var(--flag)",sw="0.8")
line((1.5,2.20,2.6),(1.5,2.20,2.34),"s-ghost")

# ---- tilted cloud ----
A=(0.60,0.95,2.31); B=(2.40,0.95,2.31); Cp=(2.40,1.55,2.45); D=(0.60,1.55,2.45)
Ab=(0.60,0.95,2.21); Bb=(2.40,0.95,2.21); Cb=(2.40,1.55,2.35); Db=(0.60,1.55,2.35)
poly([A,B,Cp,D],fill=RAY,op=1,stroke="var(--ink)",sw="0.8")
poly([D,Cp,Cb,Db],fill=RAY,op=.75,stroke="var(--ink)",sw="0.8")
poly([B,Cp,Cb,Bb],fill=RAY,op=.55,stroke="var(--ink)",sw="0.8")
for p,q in [((0.65,1.00,2.6),(0.65,1.00,2.30)),((2.35,1.00,2.6),(2.35,1.00,2.30)),
            ((0.65,1.50,2.6),(0.65,1.50,2.44)),((2.35,1.50,2.6),(2.35,1.50,2.44))]:
    line(p,q,"s-hair")

# ---- keys ----
keys=[(1,(1.20,0.05,2.60)),(2,(0.10,0.10,2.60)),(3,(0.15,1.20,1.80)),(4,(1.50,1.25,2.50)),
      (5,(1.50,2.20,2.55)),(6,(0.08,3.30,1.68)),(7,(1.50,4.50,1.70)),(8,(1.50,2.10,0.02)),
      (9,(1.12,0.80,1.20)),(10,(1.50,0.08,1.40))]
kg=['<g class="s-key">']
for n,(x,y,z) in keys:
    sx,sy=P(x,y,z)
    kg.append('    <circle cx="%.1f" cy="%.1f" r="9"/><text x="%.1f" y="%.1f">%d</text>'%(sx,sy-14,sx,sy-10.5,n))
    kg.append('    <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--ink)" stroke-width="0.9" opacity=".5" fill="none"/>'%(sx,sy-5,sx,sy))
kg.append('  </g>')
out.extend(kg)
print("\n".join(out))
