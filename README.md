# wasm-test

## The hell is this? 🤨

Nice question, this is the typical toy project I work on when I have things to do (I always have things to do) but I want to focus my brain on something else not particularly useful.

The idea was to make a cool background for my portfolio by shading a [Voronoi diagram](https://it.wikipedia.org/wiki/Diagramma_di_Voronoi).

The first trial was to accelerate the nearest point search (for the Voronoi diagram generation) with a KD-tree but I was not able to upload the KD-tree efficiently on GPU because of the limited capabilities of OpenGL 3.00 ES, plus, debugging the tree search is very hard.

I eventually ended up writing a simple linear search and then I played a little bit with shading (you can see the result [here](https://loryruta.github.io/wasm-test)).

![:/](./screenshots/voronoi1.png)

I'm not satisfied of the result but _3 days_ of efforts are enough for this.

Probably this project may be used in the future as a basis for a Emscripten + WebGL application, but also the KD-tree implementation - even if not used - is well written and working (it's also provided with a test unit).

## How to build

### Linux

Run the build commands manually or get Windows 😈

### Windows

I've made a cool PowerShell script 😁, you can run it with:

```
.\build.ps1
```
