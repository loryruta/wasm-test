import loadVoronoiModule from "./voronoi.mjs"


async function runVoronoi(canvas) {
    // Load module
    console.log("Loading voronoi WASM module...");

    WebAssembly.instantiateStreaming = null;
    let voronoiModule = await loadVoronoiModule({
        canvas: canvas,
    });
    
    console.log("Loaded");
    
    // Setup GL context
    console.log("Setting up WebGL context");

    const gl = voronoiModule.GL;
    const context = gl.createContext(canvas, {});
    gl.makeContextCurrent(context);

    console.log("Set up");

    // Initializing app
    console.log("Initializing the application");

    voronoiModule._app_init();

    console.log("Initialized");

    function loop() {
        if (canvas.width !== canvas.clientWidth)
            canvas.width = canvas.clientWidth;

        if (canvas.height !== canvas.clientHeight)
            canvas.height = canvas.clientHeight;

        voronoiModule._app_draw(canvas.clientWidth, canvas.clientHeight);

        window.requestAnimationFrame(loop);
    }

    window.requestAnimationFrame(loop);
}

export function voronoi(canvas) {
    window.addEventListener('load', () => runVoronoi(canvas));
}
