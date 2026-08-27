import math, json, sys
W,H=700,440
def sub(a,b): return (a[0]-b[0],a[1]-b[1],a[2]-b[2])
def add(a,b): return (a[0]+b[0],a[1]+b[1],a[2]+b[2])
def dot(a,b): return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def norm(a):
    l=math.sqrt(dot(a,a)); return (a[0]/l,a[1]/l,a[2]/l)
def scal(a,s): return (a[0]*s,a[1]*s,a[2]*s)

class Cam:
    def __init__(self,eye,target,fov):
        self.E=eye
        self.f=norm(sub(target,eye))
        self.r=norm(cross((0,0,1),self.f))
        self.u=cross(self.f,self.r)
        self.focal=(W/2)/math.tan(math.radians(fov)/2)
    def cc(self,P):
        d=sub(P,self.E)
        return (dot(d,self.r),dot(d,self.u),dot(d,self.f))
    def p(self,P):
        x,y,z=self.cc(P)
        if z<0.18: return None
        return (W/2+self.focal*x/z, H/2-self.focal*y/z, z)
    def clipproj(self,pts):
        NEAR=0.20
        cs=[self.cc(p) for p in pts]
        out=[]
        n=len(cs)
        for i in range(n):
            a=cs[i]; b=cs[(i+1)%n]
            ain=a[2]>=NEAR; bin_=b[2]>=NEAR
            if ain: out.append(a)
            if ain!=bin_:
                t=(NEAR-a[2])/(b[2]-a[2])
                out.append((a[0]+t*(b[0]-a[0]), a[1]+t*(b[1]-a[1]), NEAR))
        if len(out)<3: return None
        return [(W/2+self.focal*q[0]/q[2], H/2-self.focal*q[1]/q[2]) for q in out]

L=norm((-0.40,-0.55,0.73))
def shade(n): return round(0.52+0.48*max(0.0,dot(n,L)),3)

def boxfaces(x0,x1,y0,y1,z0,z1):
    v=lambda x,y,z:(x,y,z)
    return [
      ([v(x0,y0,z1),v(x1,y0,z1),v(x1,y1,z1),v(x0,y1,z1)],(0,0,1)),
      ([v(x0,y0,z0),v(x0,y1,z0),v(x1,y1,z0),v(x1,y0,z0)],(0,0,-1)),
      ([v(x0,y0,z0),v(x1,y0,z0),v(x1,y0,z1),v(x0,y0,z1)],(0,-1,0)),
      ([v(x1,y1,z0),v(x0,y1,z0),v(x0,y1,z1),v(x1,y1,z1)],(0,1,0)),
      ([v(x0,y1,z0),v(x0,y0,z0),v(x0,y0,z1),v(x0,y1,z1)],(-1,0,0)),
      ([v(x1,y0,z0),v(x1,y1,z0),v(x1,y1,z1),v(x1,y0,z1)],(1,0,0)),
    ]

def render(cam, prims, labels, title, sub_):
    out=[]
    shell=[]; objs=[]
    for (pts,n,fill,stroke,sw,inside) in prims:
        c=(sum(p[0] for p in pts)/len(pts),sum(p[1] for p in pts)/len(pts),sum(p[2] for p in pts)/len(pts))
        toward=dot(n,sub(cam.E,c))
        if inside:
            if toward>0: continue
            n=scal(n,-1)
        else:
            if toward<=0: continue
        pr=cam.clipproj(pts)
        if pr is None: continue
        rec=(math.dist(cam.E,c),pr,fill,stroke,sw,shade(n))
        (shell if inside else objs).append(rec)
    shell.sort(key=lambda t:-t[0]); objs.sort(key=lambda t:-t[0])
    for d,pr,fill,stroke,sw,sh in shell:
        pts=" ".join("%.1f,%.1f"%(q[0],q[1]) for q in pr)
        out.append('  <polygon points="%s" fill="%s"/>'%(pts,fill))
        out.append('  <polygon points="%s" fill="#000" opacity="%.3f"/>'%(pts,(1-sh)*0.60))
    E=[]
    for a in [(0,0),(3,0),(3,4.8),(0,4.8)]:
        E.append(((a[0],a[1],0),(a[0],a[1],2.6)))
    for z in (0,2.6):
        E += [((0,0,z),(3,0,z)),((3,0,z),(3,4.8,z)),((3,4.8,z),(0,4.8,z)),((0,4.8,z),(0,0,z))]
    for p,q in E:
        pr=cam.clipproj([p,q,q])
        if pr and len(pr)>=2:
            out.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="var(--ink)" stroke-width="1" opacity=".28"/>'%(pr[0][0],pr[0][1],pr[1][0],pr[1][1]))
    for d,pr,fill,stroke,sw,sh in objs:
        pts=" ".join("%.1f,%.1f"%(q[0],q[1]) for q in pr)
        out.append('  <polygon points="%s" fill="%s" stroke="%s" stroke-width="%s"/>'%(pts,fill,stroke,sw))
        out.append('  <polygon points="%s" fill="#000" opacity="%.3f"/>'%(pts,(1-sh)*0.60))
    for (anchor,dx,dy,text,col,anch) in labels:
        q=cam.p(anchor)
        if q is None: continue
        tx=min(max(q[0]+dx,14),686); ty=min(max(q[1]+dy,50),396)
        out.append('  <line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="1" opacity=".8"/>'%(q[0],q[1],tx,ty,col))
        out.append('  <circle cx="%.1f" cy="%.1f" r="3.4" fill="%s" stroke="var(--surface)" stroke-width="1"/>'%(q[0],q[1],col))
        out.append('  <text class="s-lbl" x="%.1f" y="%.1f" fill="%s" text-anchor="%s" paint-order="stroke" stroke="var(--surface)" stroke-width="3.4" stroke-linejoin="round">%s</text>'%(tx,ty-6,col,anch,text))
    out.append('  <rect x="0" y="0" width="700" height="38" fill="var(--surface)" opacity=".88"/>')
    out.append('  <rect x="0" y="%d" width="700" height="30" fill="var(--surface)" opacity=".88"/>'%(H-30))
    out.append('  <text class="s-dimt" x="16" y="25" letter-spacing="1.4" fill="var(--ink)">%s</text>'%title)
    out.append('  <text class="s-dimt" x="16" y="%d">%s</text>'%(H-11,sub_))
    return "\n".join(out)

RAY="var(--ray)"; MDF="var(--mdf)"; DK="var(--felt)"; OBJ="var(--sunk)"; WOOD="var(--wood)"
SURF="var(--sunk)"; FLOOR="var(--rule)"; GLASS="var(--glass-fill)"; MUTE="var(--mute)"; FLAG="var(--flag)"
INK="var(--ink)"

def scene():
    P=[]
    def box(x0,x1,y0,y1,z0,z1,fill,stroke=INK,sw="0.7"):
        for pts,n in boxfaces(x0,x1,y0,y1,z0,z1): P.append((pts,n,fill,stroke,sw,False))
    def quad(pts,n,fill,stroke=INK,sw="0.7"):
        P.append((pts,n,fill,stroke,sw,False))
    # room shell (inside)
    for pts,n in boxfaces(0,3.0,0,4.8,0,2.6):
        f = FLOOR if n==(0,0,-1) else ("var(--surface)" if n==(0,0,1) else SURF)
        P.append((pts,n,f,"var(--mute)","1",True))
    # rug
    quad([(0.5,0.6,0.004),(2.5,0.6,0.004),(2.5,3.6,0.004),(0.5,3.6,0.004)],(0,0,1),MDF,MDF,"1.2")
    # bass traps
    L2=0.424
    for (a,b,c) in [((0,0),(L2,0),(0,L2)),((3,0),(3-L2,0),(3,L2)),((0,4.8),(L2,4.8),(0,4.8-L2)),((3,4.8),(3-L2,4.8),(3,4.8-L2))]:
        top=[(a[0],a[1],2.6),(b[0],b[1],2.6),(c[0],c[1],2.6)]
        P.append((top,(0,0,1),RAY,INK,"0.7",False))
        for (p,q) in [(a,b),(b,c),(c,a)]:
            e=norm((q[1]-p[1],-(q[0]-p[0]),0))
            mid=((p[0]+q[0])/2,(p[1]+q[1])/2)
            if dot(e,(mid[0]-1.5,mid[1]-2.4,0))<0: e=scal(e,-1)
            P.append(([(p[0],p[1],0),(q[0],q[1],0),(q[0],q[1],2.6),(p[0],p[1],2.6)],e,RAY,INK,"0.7",False))
    # front wall modules + TV + door
    for x0 in (0.30,0.90,1.50): box(x0,x0+0.60,0,0.10,1.00,2.20,RAY)
    box(0.96,2.04,0,0.08,0.34,0.95,DK)
    quad([(2.12,0.01,0),(3.0,0.01,0),(3.0,0.01,2.1),(2.12,0.01,2.1)],(0,1,0),WOOD,WOOD,"1.6")
    box(2.20,2.80,0.02,0.07,0.70,1.90,DK)          # painel preto existente na porta
    # side mirror modules (10 cm gap) + existing 5 cm panels + skylines
    box(0.10,0.20,1.02,1.62,0.60,1.80,RAY)
    box(2.80,2.90,1.02,1.62,0.60,1.80,RAY)
    box(0.10,0.15,1.70,2.30,0.65,1.85,DK)
    box(2.85,2.90,1.70,2.30,0.65,1.85,DK)
    box(0.00,0.15,3.00,3.60,1.00,1.60,MDF)
    box(2.85,3.00,3.00,3.60,1.00,1.60,MDF)
    # cloud (tilted)
    A=(0.60,0.95,2.31);B=(2.40,0.95,2.31);C=(2.40,1.55,2.45);D=(0.60,1.55,2.45)
    Ab=(0.60,0.95,2.21);Bb=(2.40,0.95,2.21);Cb=(2.40,1.55,2.35);Db=(0.60,1.55,2.35)
    quad([A,B,C,D],(0,-0.226,0.974),RAY); quad([Ab,Bb,Cb,Db],(0,0.226,-0.974),RAY)
    quad([D,C,Cb,Db],(0,1,0),RAY); quad([A,B,Bb,Ab],(0,-1,0),RAY)
    quad([B,C,Cb,Bb],(1,0,0),RAY); quad([A,D,Db,Ab],(-1,0,0),RAY)
    # fan
    fc=(1.5,2.20)
    for ang in (90,210,330):
        a=math.radians(ang); ca,sa=math.cos(a),math.sin(a)
        P.append(([(fc[0],fc[1],2.33),(fc[0]+0.58*ca-0.08*sa,fc[1]+0.58*sa+0.08*ca,2.33),
                   (fc[0]+0.58*ca+0.08*sa,fc[1]+0.58*sa-0.08*ca,2.33)],(0,0,-1),FLAG,FLAG,"0.8",False))
    box(1.47,1.53,2.17,2.23,2.33,2.6,MUTE)
    # desk + monitors
    box(0.60,2.40,1.00,1.60,0.72,0.75,WOOD)
    for lx in (0.66,2.34):
        box(lx-0.03,lx+0.03,1.06,1.12,0,0.72,WOOD); box(lx-0.03,lx+0.03,1.48,1.54,0,0.72,WOOD)
    for cx in (0.95,2.05):
        box(cx-0.20,cx+0.20,0.62,1.02,0,0.04,DK)
        box(cx-0.035,cx+0.035,0.78,0.85,0.04,1.05,DK)
        box(cx-0.10,cx+0.10,0.70,0.95,1.05,1.35,DK)
    # chair
    box(1.34,1.66,1.95,2.30,0.42,0.47,MUTE)
    box(1.36,1.64,2.24,2.30,0.47,1.00,MUTE)
    box(1.48,1.52,2.10,2.14,0,0.42,MUTE)
    # rear: curtain, glass, gobo, shelf, painting
    quad([(0.45,4.79,0),(2.55,4.79,0),(2.55,4.79,2.1),(0.45,4.79,2.1)],(0,-1,0),GLASS,"var(--glass)","1.2")
    for i in range(13):
        a=0.20+i*0.2; b=a+0.2
        yy=4.70 if i%2==0 else 4.62
        quad([(a,yy,0),(b,yy,0),(b,yy,2.55),(a,yy,2.55)],(0,-1,0),MUTE,MUTE,"0.9")
    box(0.90,2.10,4.45,4.55,0.35,1.55,RAY)
    box(2.65,3.00,3.70,4.60,0,1.10,WOOD)
    quad([(2.99,3.55,1.30),(2.99,4.55,1.30),(2.99,4.55,2.05),(2.99,3.55,2.05)],(-1,0,0),WOOD,WOOD,"1.4")
    return P

VIEWS=[
 ("V1", (1.50,4.30,1.58),(1.50,0.0,0.92),58,
  "VISTA 1 &#183; PAREDE FRONTAL, DO PONTO DE ESCUTA",
  "3 m&#243;dulos de 1,00 a 2,20 m &#183; TV centrada, topo em 0,95 &#183; monitores em pedestais, baffle a 0,95 m da parede",
  [((1.20,0.10,1.90),0,44,"3 m&#243;dulos 0,60 &#215; 1,20 &#183; 100 mm","var(--ray)","middle"),
   ((1.50,0.08,0.72),0,56,"TV centrada, topo em 0,95","var(--felt)","middle"),
   ((0.21,0.21,1.10),26,0,"bass trap de canto","var(--ray)","start"),
   ((1.25,2.60,0.01),0,20,"tapete 2 &#215; 3","var(--mdf)","middle")]),
 ("V2", (0.26,2.20,1.58),(3.00,2.60,0.95),72,
  "VISTA 2 &#183; PAREDE DIREITA",
  "M&#243;dulo do ponto-espelho em y = 1,32 &#183; painel de 5 cm afastado 10 cm &#183; skyline em y = 3,30",
  [((2.80,1.32,1.20),0,62,"m&#243;dulo 100 mm &#183; y = 1,32","var(--ray)","middle"),
   ((2.87,2.00,1.25),0,62,"painel de 5 cm, 10 cm de ar","var(--felt)","middle"),
   ((2.92,3.30,1.30),0,66,"skyline &#183; y = 3,30","var(--mdf)","middle"),
   ((2.82,4.15,0.60),0,30,"estante","var(--wood)","middle")]),
 ("V3", (1.50,1.32,1.52),(1.50,4.80,0.88),72,
  "VISTA 3 &#183; FUNDO DE VIDRO, DA CADEIRA",
  "O que voc&#234; v&#234; ao virar a cadeira &#183; cortina blackout do teto ao ch&#227;o e o gobo m&#243;vel na frente do vidro",
  [((1.50,4.68,1.95),0,40,"cortina blackout, teto ao ch&#227;o","var(--mute)","middle"),
   ((1.50,4.46,1.20),0,46,"gobo m&#243;vel na frente do vidro","var(--ray)","middle"),
   ((0.21,4.59,1.30),22,0,"bass trap traseiro","var(--ray)","start")]),
 ("V4", (2.74,2.45,1.58),(0.0,2.55,1.02),74,
  "VISTA 4 &#183; PAREDE ESQUERDA",
  "A parede que n&#227;o aparece em nenhuma foto &#183; mesmo tratamento do lado direito, nas mesmas cotas",
  [((0.20,1.32,1.20),0,62,"m&#243;dulo 100 mm &#183; y = 1,32","var(--ray)","middle"),
   ((0.13,2.00,1.25),0,62,"painel de 5 cm, 10 cm de ar","var(--felt)","middle"),
   ((0.08,3.30,1.30),0,66,"skyline &#183; y = 3,30","var(--mdf)","middle"),
   ((0.21,0.21,1.10),-22,0,"bass trap de canto","var(--ray)","end")]),
]

P=scene()
res={}
for key,eye,tgt,fov,title,sub_,labels in VIEWS:
    cam=Cam(eye,tgt,fov)
    res[key]=render(cam,P,labels,title,sub_)
json.dump(res,open('/tmp/claude-0/-home-user-claude-code-nano-banana/b450115a-a6a6-5c6b-a90c-968d275bdc05/scratchpad/views.json','w'))
for k,v in res.items():
    n=v.count('<polygon')
    print(k,'polygons',n)
