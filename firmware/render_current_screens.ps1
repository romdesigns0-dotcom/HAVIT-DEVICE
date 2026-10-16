Add-Type -AssemblyName System.Drawing

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$assets = Join-Path $root 'main/assets'
$output = Join-Path $root 'previews'
New-Item -ItemType Directory -Force -Path $output | Out-Null

$script:fonts = [System.Drawing.Text.PrivateFontCollection]::new()
$script:fonts.AddFontFile((Join-Path $assets 'instrument_sans.ttf'))
$script:fontFamily = $script:fonts.Families[0]
$script:background = [System.Drawing.Color]::FromArgb(0x16,0x16,0x21)
$script:white = [System.Drawing.Color]::White
$script:muted = [System.Drawing.Color]::FromArgb(184,255,255,255)
$script:accent = [System.Drawing.Color]::FromArgb(0x52,0x50,0xF3)
$script:button = [System.Drawing.Color]::FromArgb(0xD9,0xD9,0xD9)
$script:format = [System.Drawing.StringFormat]::new()
$script:format.Alignment = [System.Drawing.StringAlignment]::Center
$script:format.LineAlignment = [System.Drawing.StringAlignment]::Near
$script:graphicsScale = 1

function New-Canvas {
    $bitmap = [System.Drawing.Bitmap]::new(360,360,[System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $graphics.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $graphics.Clear($script:background)
    return @{ Bitmap=$bitmap; Graphics=$graphics }
}

function Draw-Outline($g) {
    $pen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(16,0,0,0),1)
    $g.DrawEllipse($pen,0,0,359,359)
    $pen.Dispose()
}

function Draw-RoundRect($g,[int]$x,[int]$y,[int]$w,[int]$h,[System.Drawing.Color]$color,[int]$radius=999) {
    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $d = [Math]::Min([Math]::Min($w,$h),[Math]::Max(1,$radius*2))
    if($d -ge $h) {
        $path.AddArc($x,$y,$h,$h,90,180)
        $path.AddArc($x+$w-$h,$y,$h,$h,270,180)
        $path.CloseFigure()
    } else {
        $path.AddArc($x,$y,$d,$d,180,90)
        $path.AddArc($x+$w-$d,$y,$d,$d,270,90)
        $path.AddArc($x+$w-$d,$y+$h-$d,$d,$d,0,90)
        $path.AddArc($x,$y+$h-$d,$d,$d,90,90)
        $path.CloseFigure()
    }
    $brush = [System.Drawing.SolidBrush]::new($color)
    $g.FillPath($brush,$path)
    $brush.Dispose(); $path.Dispose()
}

function Draw-Text($g,[string]$text,[int]$x,[int]$y,[int]$width,[float]$size,[System.Drawing.Color]$color,[bool]$center=$false) {
    $style = [System.Drawing.FontStyle]::Regular
    $font = [System.Drawing.Font]::new($script:fontFamily,$size,$style,[System.Drawing.GraphicsUnit]::Pixel)
    $brush = [System.Drawing.SolidBrush]::new($color)
    $fmt = [System.Drawing.StringFormat]::new()
    $fmt.Alignment = if($center) {[System.Drawing.StringAlignment]::Center} else {[System.Drawing.StringAlignment]::Near}
    $fmt.LineAlignment = [System.Drawing.StringAlignment]::Near
    $fmt.Trimming = [System.Drawing.StringTrimming]::EllipsisCharacter
    $fmt.FormatFlags = [System.Drawing.StringFormatFlags]::NoWrap
    $g.DrawString($text,$font,$brush,[System.Drawing.RectangleF]::new($x,$y,$width,90),$fmt)
    $font.Dispose(); $brush.Dispose(); $fmt.Dispose()
}

function Draw-Label($g,[string]$text,[int]$y,[float]$size,[System.Drawing.Color]$color,[int]$opacity=255) {
    $c = [System.Drawing.Color]::FromArgb($opacity,$color.R,$color.G,$color.B)
    Draw-Text $g $text 0 $y 360 $size $c $true
}

function Draw-Rgb565A8($g,[string]$name,[int]$x,[int]$y,[int]$w,[int]$h,[int]$sourceW=0,[int]$sourceH=0) {
    $path = Join-Path $assets $name
    $data = [System.IO.File]::ReadAllBytes($path)
    if($sourceW -le 0) {$sourceW=$w}
    if($sourceH -le 0) {$sourceH=$h}
    $count = $sourceW*$sourceH
    if($data.Length -lt $count*3) { throw "Asset $name has unexpected size $($data.Length)" }
    $bmp = [System.Drawing.Bitmap]::new($sourceW,$sourceH,[System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    for($i=0;$i -lt $count;$i++) {
        $v = [int]$data[$i*2] -bor ([int]$data[$i*2+1] -shl 8)
        $r = (($v -shr 11) -band 31) * 255 / 31
        $gg = (($v -shr 5) -band 63) * 255 / 63
        $b = ($v -band 31) * 255 / 31
        $a = [int]$data[$count*2+$i]
        $bmp.SetPixel($i % $sourceW,[int][Math]::Floor($i / $sourceW),[System.Drawing.Color]::FromArgb($a,$r,$gg,$b))
    }
    $g.DrawImage($bmp,$x,$y,$w,$h)
    $bmp.Dispose()
}

function Draw-Eye($g,[int]$x,[int]$y,[int]$w,[int]$h,[float]$degrees,[float]$blink=1) {
    $state=$g.Save()
    $g.TranslateTransform($x+$w/2,$y+$h/2)
    $g.RotateTransform($degrees)
    $eyeH=[Math]::Max(2,$h*$blink)
    $eyeColor=[System.Drawing.Color]::FromArgb(0x1A,0x1A,0x24)
    Draw-RoundRect $g ([int](-$w/2)) ([int](-$eyeH/2)) $w ([int]$eyeH) $eyeColor 999
    $g.Restore($state)
}

function Draw-BaseScreen($g) {
    Draw-Outline $g
    Draw-Text $g 'Wi-Fi' 0 17 360 15 ([System.Drawing.Color]::White) $true
}

function Draw-Onboarding($g,[bool]$blink=$false) {
    Draw-Outline $g
    Draw-Eye $g 211 71 37 110 5 $(if($blink){0.08}else{1})
    Draw-Eye $g 112 71 37 110 175 $(if($blink){0.08}else{1})
    Draw-Label $g 'Hola, Soy Havit.' 211 22 ([System.Drawing.Color]::White) 184
    Draw-Label $g 'Vamos a crecer juntos' 239 15 ([System.Drawing.Color]::White) 64
    Draw-Rgb565A8 $g 'ring1.rgb565a8' 10 10 340 340
    Draw-RoundRect $g 153 278 54 54 $script:button 999
    Draw-Rgb565A8 $g 'arrow_welcome.rgb565a8' 167 298 25 14 29 17
    Draw-Label $g 'Wi-Fi' 17 15 ([System.Drawing.Color]::White)
}

function Draw-SignIn($g) {
    Draw-Outline $g
    Draw-Label $g 'Inicia Sesión' 143 22 ([System.Drawing.Color]::White) 184
    Draw-Label $g 'Tu próximo paso empieza aquí' 171 15 ([System.Drawing.Color]::White) 64
    Draw-Rgb565A8 $g 'ring2.rgb565a8' 10 10 340 340
    Draw-Rgb565A8 $g 'separator.rgb565a8' 150 209 30 84
    Draw-Text $g 'Welcome' 185 224 170 20 ([System.Drawing.Color]::FromArgb(128,255,255,255))
    Draw-Text $g 'Back' 185 248 80 20 ([System.Drawing.Color]::FromArgb(128,255,255,255))
    Draw-Text $g 'ROM' 238 248 85 20 $script:accent
    Draw-RoundRect $g 82 218 64 64 $script:button 999
    Draw-Rgb565A8 $g 'google_logo.rgb565a8' 91 226 47 48
    Draw-Label $g 'Wi-Fi' 17 15 ([System.Drawing.Color]::White)
}

function Draw-Welcome($g) {
    Draw-Outline $g
    Draw-Rgb565A8 $g 'ring3.rgb565a8' 10 10 340 340
    Draw-Eye $g 203 38 27 81 5
    Draw-Eye $g 130 38 27 81 175
    Draw-Label $g 'Bienvenido de nuevo.' 154 22 ([System.Drawing.Color]::White) 184
    Draw-Label $g 'HAVIT' 182 22 $script:accent
    Draw-Rgb565A8 $g 'continue_pill.rgb565a8' 87 242 180 51
    Draw-Text $g 'Continuar' 98 253 150 24 ([System.Drawing.Color]::FromArgb(184,255,255,255))
    Draw-RoundRect $g 218 244 45 45 $script:button 999
    Draw-Rgb565A8 $g 'arrow.rgb565a8' 228 260 25 14 29 17
    Draw-Label $g 'Wi-Fi' 17 15 ([System.Drawing.Color]::White)
}

function Draw-Home($g) {
    Draw-Outline $g
    Draw-Label $g 'HAVIT OS' 145 24 ([System.Drawing.Color]::White)
    Draw-Label $g 'Inicio preparado' 180 15 ([System.Drawing.Color]::White) 160
    Draw-Label $g 'Tareas sincronizadas: 0' 208 15 ([System.Drawing.Color]::White) 160
    Draw-Label $g 'Wi-Fi' 17 15 ([System.Drawing.Color]::White)
}

function Draw-SetupButton($g,[string]$text,[int]$y) {
    Draw-RoundRect $g 60 $y 240 35 $script:button 999
    Draw-Text $g $text 60 ($y+8) 240 15 $script:background $true
}

function Draw-Wifi($g,[ValidateSet('connections','manual','forget')][string]$mode) {
    Draw-Outline $g
    if($mode -eq 'manual') {
        Draw-Label $g 'Wi-Fi de configuración' 63 20 ([System.Drawing.Color]::White)
        Draw-Label $g 'HAVIT-SETUP' 112 15 $script:accent
        Draw-Label $g 'Clave: —' 145 15 ([System.Drawing.Color]::White)
        Draw-Label $g "Únete a esta red y abre`nhttp://192.168.4.1" 178 15 ([System.Drawing.Color]::White) 184
        Draw-SetupButton $g 'Activar red de configuración' 236
        Draw-SetupButton $g 'Volver' 281
    } elseif($mode -eq 'forget') {
        Draw-Label $g '¿Olvidar redes Wi-Fi?' 95 20 ([System.Drawing.Color]::White)
        Draw-Label $g "Tendrás que configurar tu red de nuevo.`nTu cuenta seguirá guardada." 145 15 ([System.Drawing.Color]::White) 184
        Draw-SetupButton $g 'Sí, olvidar redes' 220
        Draw-SetupButton $g 'Cancelar' 266
    } else {
        Draw-Label $g 'Conexiones' 38 20 ([System.Drawing.Color]::White)
        Draw-Label $g 'Wi-Fi: sin conexión' 70 15 ([System.Drawing.Color]::White)
        Draw-Label $g 'Sin red conectada' 94 15 ([System.Drawing.Color]::White) 184
        Draw-Label $g 'Sin dirección IP' 116 15 ([System.Drawing.Color]::White) 160
        Draw-Label $g 'Bluetooth de configuración: apagado' 139 15 ([System.Drawing.Color]::White) 184
        Draw-SetupButton $g 'Activar Bluetooth' 160
        Draw-SetupButton $g 'Reconectar Wi-Fi' 200
        Draw-SetupButton $g 'Olvidar redes Wi-Fi' 240
        Draw-RoundRect $g 60 280 150 35 $script:button 999
        Draw-Text $g 'Wi-Fi manual' 80 288 110 15 $script:background
        Draw-RoundRect $g 218 280 82 35 $script:button 999
        Draw-Text $g 'Volver' 235 288 55 15 $script:background
    }
}

function Draw-Pairing($g) {
    Draw-Outline $g
    Draw-Label $g 'Vincula tu HAVIT' 46 22 ([System.Drawing.Color]::White)
    Draw-Label $g 'Wi-Fi sin conexión' 83 15 ([System.Drawing.Color]::FromArgb(0xFF,0xD0,0x80))
    Draw-Label $g 'Bluetooth apagado' 106 15 ([System.Drawing.Color]::White) 184
    Draw-Label $g '···· ····' 140 24 $script:accent
    Draw-Label $g "En la web: Buscar HAVIT por Bluetooth.`nConfigura tu Wi-Fi de 2.4 GHz." 182 15 ([System.Drawing.Color]::White) 220
    Draw-SetupButton $g 'Reintentar / pedir PIN' 242
    Draw-RoundRect $g 60 286 150 35 $script:button 999
    Draw-Text $g 'Conexiones' 87 294 100 15 $script:background
    Draw-RoundRect $g 218 286 82 35 $script:button 999
    Draw-Text $g 'Volver' 235 294 55 15 $script:background
}

function Save-Screen([string]$name,[string]$title,[scriptblock]$draw) {
    $canvas=New-Canvas
    & $draw $canvas.Graphics
    $path=Join-Path $output $name
    $canvas.Bitmap.Save($path,[System.Drawing.Imaging.ImageFormat]::Png)
    $canvas.Graphics.Dispose(); $canvas.Bitmap.Dispose()
    return [pscustomobject]@{Title=$title;Path=$path}
}

$screens = @(
    (Save-Screen '01_onboarding.png' 'Bienvenida / ojos abiertos' { param($g) Draw-Onboarding $g $false }),
    (Save-Screen '02_onboarding_blink.png' 'Bienvenida / parpadeo' { param($g) Draw-Onboarding $g $true }),
    (Save-Screen '03_sign_in.png' 'Vinculación' { param($g) Draw-SignIn $g }),
    (Save-Screen '04_welcome_back.png' 'Bienvenido de nuevo' { param($g) Draw-Welcome $g }),
    (Save-Screen '05_home.png' 'Inicio (placeholder)' { param($g) Draw-Home $g }),
    (Save-Screen '06_pairing.png' 'Vinculación pendiente' { param($g) Draw-Pairing $g }),
    (Save-Screen '07_wifi_connections.png' 'Conexiones' { param($g) Draw-Wifi $g 'connections' }),
    (Save-Screen '08_wifi_manual.png' 'Wi-Fi manual' { param($g) Draw-Wifi $g 'manual' }),
    (Save-Screen '09_wifi_forget_confirm.png' 'Confirmar olvido de redes' { param($g) Draw-Wifi $g 'forget' })
)

$thumbW=300; $thumbH=300; $labelH=38; $cols=3; $rows=[Math]::Ceiling($screens.Count/$cols)
$sheet=[System.Drawing.Bitmap]::new($cols*$thumbW,$rows*($thumbH+$labelH),[System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$sg=[System.Drawing.Graphics]::FromImage($sheet)
$sg.Clear([System.Drawing.Color]::FromArgb(32,32,38))
$sg.SmoothingMode=[System.Drawing.Drawing2D.SmoothingMode]::HighQuality
for($i=0;$i -lt $screens.Count;$i++) {
    $bmp=[System.Drawing.Bitmap]::FromFile($screens[$i].Path)
    $col=$i % $cols; $row=[Math]::Floor($i/$cols); $x=$col*$thumbW; $y=$row*($thumbH+$labelH)
    $sg.DrawImage($bmp,$x,$y,$thumbW,$thumbH)
    $font=[System.Drawing.Font]::new('Segoe UI',11,[System.Drawing.FontStyle]::Regular,[System.Drawing.GraphicsUnit]::Pixel)
    $brush=[System.Drawing.SolidBrush]::new([System.Drawing.Color]::White)
    $sg.DrawString($screens[$i].Title,$font,$brush,[System.Drawing.PointF]::new($x+8,$y+$thumbH+8))
    $font.Dispose(); $brush.Dispose(); $bmp.Dispose()
}
$sheet.Save((Join-Path $output '00_overview.png'),[System.Drawing.Imaging.ImageFormat]::Png)
$sg.Dispose(); $sheet.Dispose(); $script:fonts.Dispose(); $script:format.Dispose()
Write-Output "Created $($screens.Count) screens and overview in $output"
