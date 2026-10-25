$ErrorActionPreference = 'Stop'
Push-Location (Join-Path $PSScriptRoot '..')
try {
    foreach ($size in @(5, 7, 8, 10, 11, 12, 13, 14, 15, 16, 17, 20, 21, 22, 24, 28, 32, 48)) {
        & npx.cmd --yes lv_font_conv@1.5.3 `
            --font ui/fonts/InstrumentSans-Regular.ttf `
            --size $size --bpp 4 --no-compress --format lvgl `
            --lv-include lvgl.h `
            --output "ui/fonts/instrument_sans_regular_$size.c" `
            --lv-font-name "havit_instrument_sans_$size" `
            -r 0x20-0x7E -r 0xA1-0xFF
        if ($LASTEXITCODE -ne 0) { throw "Font conversion failed at size $size" }
    }
}
finally { Pop-Location }
