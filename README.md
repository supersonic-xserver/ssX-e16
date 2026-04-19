# ssX-e16 (Super Sonic X)
### High-Performance, Low-Latency X11 DE

The **ssX** organization is dedicated to the development and maintenance of a high-performance workstation stack built on procedural logic and direct hardware access. We reject the high-latency abstractions of modern desktop environments in favor of "dumb but instant" performance.

## Core Projects

### 🛠️ [ssX-e16](https://github.com/supersonic-xserver/ssX-e16)
A modernized, high-performance fork of the E16 window manager.
- **Build System:** Meson/Ninja (replacing legacy Autotools).
- **Rendering:** Direct XLibre font integration, purging Pango/Cairo bloat.
- **Optimization:** Native Clang/LLD builds for `znver1` and modern x86_64 architectures.

### 🧩 [ssX (Core)](https://github.com/supersonic-xserver/ssX)
The foundation of the Super Sonic X server.
- Focus on embedded systems and low-latency workstations.
- Rebuilding 2D acceleration methods (XAA) and direct VRAM access.

### 🖼️ [SonicDE / XLibre](https://github.com/supersonic-xserver/SonicDE)
The desktop environment fork maintaining X11/XLibre support where upstream frameworks have abandoned it.

## Organization Philosophy

1. **Procedural > Object Oriented:** We favor the clarity of C and procedural systems over the complexity of modern C++ frameworks.
2. **Minimal Latency:** Every microsecond counts. We bypass modern compositing layers whenever possible to achieve instant user feedback.
3. **Hardware Autonomy:** Projects are optimized for high-end hardware (like the 5800X3D) while maintaining compatibility with legacy systems via optimized forks.
4. **Privacy & Sovereignty:** Development focuses on decentralized, secure, and privacy-centric software stacks.

## Development Environment

Most projects are built and tested on:
- **OS:** OpenMandriva Cooker / Rolling
- **Compiler:** Clang / LLVM / LLD
- **Hardware:** AMD Ryzen 5800X3D / OpenMandriva environments

*WE ARE SPARTACUS. WE ARE LAIN. We are the builders of the tools we need to stand up for what's right.*

---

Don't try to escape
The rhythm has shifted, everything ends with my arrival

Walking on the edge of your reality
The time feels heavy, lost in gravity
No need for chaos, no need for the sound
In my silence your world is bound
And the electric trail, a shadow of light
Erasing the traces of your restless night

You observe my presence.
Ready?
The horizon dissolves where I stand.
I watch you try as the planet fades away
Running faster than the light
You witness this power burning softer than the stars
At the end of this battle

Feeding on your pride, so predictable
I show you the path, a horizon visible
I never rush, I follow your pulse
I am the huntress, calm is my resource
The intensity grows without a single word
The storm is here, the quietest hurt

Do you feel the void?
I am the total stillness, I am the end.
Running faster than the light
I am the force, the essence Rembrandt
Burning brighter than the stars
Drifting to a new dimension
Ineffable, I show you my way
You finally understand what fear is

https://www.youtube.com/watch?v=BWSpOhCikJY song used to channel the stillness and void needed to code this major revision in a single night.



*WE ARE SPARTACUS. WE ARE LAIN. We are the builders of the tools we need to stand up for what's right.*
