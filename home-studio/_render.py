import asyncio,sys
from playwright.async_api import async_playwright
CSS="""<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;600&display=swap');
:root{--paper:#faf9f5;--surface:#fff;--sunk:#f0eee7;--ink:#1a1a18;--mute:#8a8579;--rule:#ddd9cd;
--ray:#3f7d5c;--flag:#c2703a;--wood:#8a6a3f;--felt:#2f2f2c;--glass:#8fb3c4;--mdf:#7a6a52;--mdf-soft:#EADCC4}
body{margin:0;background:var(--paper)}
svg{display:block;background:var(--paper)}
.s-lbl{font-family:'IBM Plex Mono',monospace;font-size:11px;fill:var(--ink)}
.s-big{font-family:'IBM Plex Mono',monospace;font-size:14px;font-weight:600;fill:var(--ink)}
.s-tag{font-family:'IBM Plex Mono',monospace;font-size:11px;font-weight:600}
.s-leg{font-family:'IBM Plex Mono',monospace;font-size:12px;font-weight:600;fill:var(--ink)}
.s-dimt{font-family:'IBM Plex Mono',monospace;font-size:10.5px;fill:var(--mute)}
.s-dim{stroke:var(--mute);stroke-width:1}
</style>"""
async def main():
    src,out,w,h=sys.argv[1],sys.argv[2],int(sys.argv[3]),int(sys.argv[4])
    svg=open(src).read()
    html=f"<!doctype html><meta charset=utf8>{CSS}<body>{svg}</body>"
    open("/tmp/_r.html","w").write(html)
    async with async_playwright() as p:
        b=await p.chromium.launch(executable_path="/opt/pw-browsers/chromium-1194/chrome-linux/chrome")
        pg=await b.new_page(viewport={"width":w,"height":h},device_scale_factor=2)
        await pg.goto("file:///tmp/_r.html"); await pg.wait_for_timeout(1200)
        await pg.locator("svg").screenshot(path=out)
        await b.close()
asyncio.run(main())
