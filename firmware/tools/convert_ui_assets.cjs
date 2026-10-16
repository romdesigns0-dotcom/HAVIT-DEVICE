const sharp = require('sharp');
const fs = require('fs');
const path = require('path');
const source = path.resolve('ui/images');
const target = path.resolve('firmware/main/assets');
const assets = [
  ['gradient_ring.png', 'ring1.rgb565a8'],
  ['gradient_ring_f2.png', 'ring2.rgb565a8'],
  ['gradient_ring_f3.png', 'ring3.rgb565a8'],
  ['google_logo_1.png', 'google_logo.rgb565a8'],
  ['vec_9_466.png', 'separator.rgb565a8'],
  ['continue_pill.png', 'continue_pill.rgb565a8'],
  ['vec_9_540.png', 'arrow.rgb565a8'],
  ['vec_9_570.png', 'arrow_welcome.rgb565a8'],
];
(async () => {
  for (const [input, output] of assets) {
    const { data, info } = await sharp(path.join(source, input)).ensureAlpha().raw().toBuffer({ resolveWithObject: true });
    const count = info.width * info.height;
    const color = Buffer.alloc(count * 2);
    const alpha = Buffer.alloc(count);
    for (let i = 0; i < count; i++) {
      const r = data[i * 4], g = data[i * 4 + 1], b = data[i * 4 + 2];
      const c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      color[i * 2] = c & 255;
      color[i * 2 + 1] = c >> 8;
      alpha[i] = data[i * 4 + 3];
    }
    fs.writeFileSync(path.join(target, output), Buffer.concat([color, alpha]));
    console.log(`${input} ${info.width}x${info.height}: ${count * 3} bytes`);
  }
  fs.copyFileSync(path.join('ui/fonts', 'InstrumentSans-Regular.ttf'), path.join(target, 'instrument_sans.ttf'));
})();
