import asyncio,sys,json
from playwright.async_api import async_playwright
CSS="""<style>
@import url('https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;600;700&display=swap');
:root{--paper:#faf9f5;--ink:#1a1a18;--mute:#8a8579;--ray:#3f7d5c;--flag:#c2703a}
html,body{margin:0;background:transparent}
svg{display:block;background:transparent}
.h1{font-family:'IBM Plex Mono',monospace;font-weight:700;fill:var(--paper)}
.h2{font-family:'IBM Plex Mono',monospace;font-weight:600;fill:var(--paper)}
.sm{font-family:'IBM Plex Mono',monospace;font-weight:400;fill:var(--paper)}
.flag{fill:#e08a4e}
</style>"""
async def main():
    src,out=sys.argv[1],sys.argv[2]
    svg=open(src).read()
    open("/tmp/_c.html","w").write(f"<!doctype html><meta charset=utf8>{CSS}<body>{svg}</body>")
    async with async_playwright() as p:
        b=await p.chromium.launch(executable_path="/opt/pw-browsers/chromium-1194/chrome-linux/chrome")
        pg=await b.new_page(viewport={"width":1080,"height":1920})
        await pg.goto("file:///tmp/_c.html"); await pg.wait_for_timeout(900)
        await pg.locator("svg").screenshot(path=out,omit_background=True)
        await b.close()
asyncio.run(main())
