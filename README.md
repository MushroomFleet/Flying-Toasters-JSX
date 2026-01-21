# Flying Toasters JSX

A nostalgic tribute to Berkeley Systems' iconic **After Dark Flying Toasters** screensaver, reimagined in SVGA-style vertex-shaded wireframe graphics.

![SVGA Wireframe](https://img.shields.io/badge/Graphics-SVGA%20Wireframe-00aaff)
![React](https://img.shields.io/badge/React-16.8%2B-61dafb)
![License](https://img.shields.io/badge/License-MIT-green)

## ✨ Features

- **3D Wireframe Rendering** — Real-time perspective projection with depth sorting
- **Vertex Color Shading** — Gouraud-style gradient interpolation along wireframe edges
- **Articulated Wings** — Procedurally animated wing geometry with realistic flapping
- **Retro CRT Effects** — Scanlines and motion trails for authentic vintage aesthetics
- **Configurable Parameters** — Adjust toaster count, speeds, colors, and visual effects

## 🎮 Quick Preview

Open **[demo.html](demo.html)** in any modern browser for an interactive demonstration with real-time controls:

- Adjust toaster count (1-20)
- Control wing flap speed
- Modify flight speed
- Toggle CRT scanlines
- Toggle vertex glow effects
- Toggle motion trails

No build step required — just open the HTML file directly.

## 📁 Repository Structure

```
Flying-Toasters-JSX/
├── README.md                           # This file
├── demo.html                           # Standalone interactive demo
├── flying-toasters.jsx                 # React component
└── Flying-Toasters-JSX-integration.md  # Developer integration guide
```

## 🚀 Getting Started

### Option 1: Standalone Demo

Simply open `demo.html` in your browser. No installation required.

### Option 2: React Integration

1. Copy `flying-toasters.jsx` to your project
2. Import and use:

```jsx
import FlyingToastersScreensaver from './flying-toasters';

function App() {
  return <FlyingToastersScreensaver />;
}
```

For detailed integration instructions, customization options, and advanced usage, see the **[Integration Guide](Flying-Toasters-JSX-integration.md)**.

## 🎨 Technical Highlights

### 3D Pipeline

The renderer implements a simplified software 3D pipeline:

1. **Model Space** — Toaster and wing geometry defined as vertices and edges
2. **World Transform** — Scale, rotation, and translation per entity
3. **Wing Animation** — Real-time vertex manipulation for flapping motion
4. **Perspective Projection** — 3D to 2D conversion with FOV
5. **Depth Sorting** — Back-to-front rendering for proper occlusion
6. **Vertex Shading** — Per-vertex lighting with gradient interpolation

### Visual Style

The SVGA wireframe aesthetic is achieved through:

- **Cyan-Magenta Palette** — Classic VGA color scheme
- **Height-Based Color Blending** — Vertex colors vary by Y position
- **Diffuse Lighting** — Simple N·L shading for depth perception
- **Line Width Scaling** — Thicker lines for closer vertices
- **Corner Glow** — Radial gradients at key vertices

## 🖥️ Browser Support

| Browser | Version | Status |
|---------|---------|--------|
| Chrome | 60+ | ✅ Supported |
| Firefox | 55+ | ✅ Supported |
| Safari | 11+ | ✅ Supported |
| Edge | 79+ | ✅ Supported |

## 🔧 Customization

The component supports extensive customization:

- **Toaster Count** — Render 1 to 20+ flying toasters
- **Color Scheme** — Modify base and highlight colors
- **Flight Pattern** — Change direction, speed, and wobble
- **Wing Animation** — Adjust flap speed and amplitude
- **Visual Effects** — Toggle scanlines, glow, and trails

See the [Integration Guide](Flying-Toasters-JSX-integration.md) for detailed customization instructions.

## 🙏 Acknowledgments

- **Berkeley Systems** — Original Flying Toasters screensaver (After Dark, 1989)
- The demoscene community for keeping wireframe aesthetics alive

## 📄 License

MIT License — See [LICENSE](LICENSE) for details.

*Flying Toasters is a trademark of Berkeley Systems. This project is an independent tribute and is not affiliated with or endorsed by Berkeley Systems or its successors.*

---

## 📚 Citation

### Academic Citation

If you use this codebase in your research or project, please cite:

```bibtex
@software{flying_toasters_jsx,
  title = {Flying Toasters JSX: SVGA Wireframe Screensaver Component},
  author = {Drift Johnson},
  year = {2025},
  url = {https://github.com/MushroomFleet/Flying-Toasters-JSX},
  version = {1.0.0}
}
```

### Donate:

[![Ko-Fi](https://cdn.ko-fi.com/cdn/kofi3.png?v=3)](https://ko-fi.com/driftjohnson)
