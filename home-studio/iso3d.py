# -*- coding: utf-8 -*-
import math
R2=math.sqrt(2)
def sub(a,b): return (a[0]-b[0],a[1]-b[1],a[2]-b[2])
def add(a,b): return (a[0]+b[0],a[1]+b[1],a[2]+b[2])
def mul(a,s): return (a[0]*s,a[1]*s,a[2]*s)
def cross(a,b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
def dot(a,b): return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
def norm(a):
    m=math.sqrt(dot(a,a)); return (a[0]/m,a[1]/m,a[2]/m) if m else a

class Cam:
    def __init__(s,eye,target,fov,W,H):
        s.E=eye; s.W=W; s.H=H
        s.f=norm(sub(target,eye)); s.r=norm(cross((0,0,1),s.f)); s.u=cross(s.f,s.r)
        s.focal=(W/2)/math.tan(math.radians(fov)/2)
    def cc(s,p):
        d=sub(p,s.E); return (dot(d,s.r),dot(d,s.u),dot(d,s.f))
    def proj(s,pts):
        NEAR=0.12; cs=[s.cc(p) for p in pts]; out=[]; n=len(cs)
        for i in range(n):
            a=cs[i]; b=cs[(i+1)%n]
            ain=a[2]>=NEAR; bin_=b[2]>=NEAR
            if ain: out.append(a)
            if ain!=bin_:
                t=(NEAR-a[2])/(b[2]-a[2])
                out.append((a[0]+t*(b[0]-a[0]), a[1]+t*(b[1]-a[1]), NEAR))
        if len(out)<3: return None
        return [(s.W/2+s.focal*q[0]/q[2], s.H/2-s.focal*q[1]/q[2]) for q in out]
    def depth(s,pts):
        return sum(s.cc(p)[2] for p in pts)/len(pts)

LIGHT=norm((0.45,0.65,1.0))
class Scene:
    def __init__(s,cam,ox=0,oy=0): s.cam=cam; s.f=[]; s.ox=ox; s.oy=oy
    def quad(s,pts,color,stroke=None,sw="1",op=1.0,shade=True,prio=1):
        n=norm(cross(sub(pts[1],pts[0]),sub(pts[2],pts[0])))
        if dot(n,sub(s.cam.E,pts[0]))<=0: return
        sh=0.45+0.55*max(0.0,dot(n,LIGHT)) if shade else 1.0
        s.f.append((prio,s.cam.depth(pts),pts,color,stroke,sw,op,sh))
    def box(s,o,e1,e2,e3,color,stroke="var(--ink)",sw=".8",op=1.0,prio=1):
        P=[o,add(o,e1),add(add(o,e1),e2),add(o,e2)]
        Q=[add(p,e3) for p in P]
        for f in ([P[0],P[3],P[2],P[1]],Q,
                  [P[0],P[1],Q[1],Q[0]],[P[1],P[2],Q[2],Q[1]],
                  [P[2],P[3],Q[3],Q[2]],[P[3],P[0],Q[0],Q[3]]):
            s.quad(f,color,stroke,sw,op,True,prio)
    def out(s):
        o=[]
        for pr,d,pts,color,stroke,sw,op,sh in sorted(s.f,key=lambda t:(t[0],-t[1])):
            pr=s.cam.proj(pts)
            if not pr: continue
            d_=" ".join("%.1f,%.1f"%(p[0]+s.ox,p[1]+s.oy) for p in pr)
            st=' stroke="%s" stroke-width="%s" stroke-linejoin="round"'%(stroke,sw) if stroke else ''
            o.append('  <polygon points="%s" fill="%s" fill-opacity="%.3f"%s/>'%(d_,color,op,st))
            if sh<0.999:
                o.append('  <polygon points="%s" fill="#000" fill-opacity="%.3f"/>'%(d_,(1-sh)*0.58))
        return o
    def pt(s,p):
        c=s.cam.cc(p)
        if c[2]<0.12: return None
        return (s.cam.W/2+s.cam.focal*c[0]/c[2]+s.ox, s.cam.H/2-s.cam.focal*c[1]/c[2]+s.oy)
