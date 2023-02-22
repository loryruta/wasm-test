
mkdir ./build -erroraction 'silentlycontinue'

Copy-Item ./src/app.cpp ./build/resolved_app.cpp

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

EmbedContent -FilePath ./build/resolved_app.cpp -PlaceholderName '{{ SCREEN_QUAD_SRC }}' -ResourcePath ./shaders/screen_quad.glsl
EmbedContent -FilePath ./build/resolved_app.cpp -PlaceholderName '{{ BOUNCE_POINTS_SRC }}' -ResourcePath ./shaders/bounce_points.glsl
EmbedContent -FilePath ./build/resolved_app.cpp -PlaceholderName '{{ VORONOI_SRC }}' -ResourcePath ./shaders/voronoi.glsl

emcc ./build/resolved_app.cpp -O0 -o voronoi.mjs `
    -s EXPORTED_FUNCTIONS=_app_init,_app_draw `
    -s EXPORTED_RUNTIME_METHODS=GL,cwrap `
    -s ASSERTIONS=1 `
    -s GL_DEBUG=1 `
    -s GL_PREINITIALIZED_CONTEXT=1 `
    -s USE_GLFW=3 `
    -s MIN_WEBGL_VERSION=2
