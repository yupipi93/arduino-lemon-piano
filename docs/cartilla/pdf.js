const puppeteer = require('puppeteer');
(async () => {
  const b = await puppeteer.launch({ args:['--no-sandbox','--disable-dev-shm-usage','--font-render-hinting=none'] });
  const p = await b.newPage();
  await p.goto('file:///work/cartilla.built.html', { waitUntil:'networkidle0', timeout:60000 });
  await p.evaluate(() => document.fonts.ready);
  await new Promise(r => setTimeout(r, 600));
  await p.pdf({ path:'/work/cartilla.pdf', format:'A4', landscape:true,
                printBackground:true, margin:{top:0,right:0,bottom:0,left:0},
                preferCSSPageSize:true });
  // a PNG proof of each side, so the layout can be checked without a printer
  await p.setViewport({ width:1123, height:794, deviceScaleFactor:2 });
  await p.screenshot({ path:'/work/side1.png', clip:{x:0,y:0,width:1123,height:794} });
  await b.close();
})();
