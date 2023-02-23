
mkdir ./build -ErrorAction 'silentlycontinue'

$ResolvedAppPath = './docs/resolved_app.cpp'

Copy-Item ./native/src/app.cpp $ResolvedAppPath

function FormatStringForCCode {
    param (
        $String
    )

    $OutString='';
    foreach ($Char in $String.ToCharArray()) {
        $OutString += '\x' + ("{0:x}" -f ([int][char]$Char));
    }
    Write-Output $OutString
}

function EmbedContent {
    param (
        $FilePath,
        $PlaceholderName,
        $ResourcePath
    )

    $ResourceContent = Get-Content $ResourcePath -Encoding UTF8 -Raw
    $ResourceContent = FormatStringForCCode -String $ResourceContent
    (Get-Content $FilePath).replace($PlaceholderName, $ResourceContent) | Set-Content $FilePath
}

EmbedContent -FilePath $ResolvedAppPath -PlaceholderName '{{ SCREEN_QUAD_SRC }}' -ResourcePath ./native/shaders/screen_quad.glsl
EmbedContent -FilePath $ResolvedAppPath -PlaceholderName '{{ BOUNCE_POINTS_SRC }}' -ResourcePath ./native/shaders/bounce_points.glsl
EmbedContent -FilePath $ResolvedAppPath -PlaceholderName '{{ VORONOI_SRC }}' -ResourcePath ./native/shaders/voronoi.glsl

emcc $ResolvedAppPath -O0 -o ./docs/voronoi.mjs `
    -s EXPORTED_FUNCTIONS=_app_init,_app_draw `
    -s EXPORTED_RUNTIME_METHODS=GL,cwrap `
    -s ASSERTIONS=1 `
    -s GL_DEBUG=1 `
    -s GL_PREINITIALIZED_CONTEXT=1 `
    -s USE_GLFW=3 `
    -s MIN_WEBGL_VERSION=2

echo 'app.cpp compiled'

Copy-Item ./web/* -Destination ./docs/ -Recurse

echo 'web folder copied'
