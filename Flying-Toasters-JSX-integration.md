# Flying Toasters JSX Integration Guide

This guide covers how to integrate the Flying Toasters SVGA Wireframe component into your React projects.

## Prerequisites

- React 16.8+ (requires Hooks support)
- A modern browser with Canvas API support

## Quick Start

### 1. Copy the Component

Copy `flying-toasters.jsx` into your project's components directory:

```
src/
  components/
    flying-toasters.jsx
```

### 2. Import and Use

```jsx
import FlyingToastersScreensaver from './components/flying-toasters';

function App() {
  return (
    <div>
      <FlyingToastersScreensaver />
    </div>
  );
}
```

The component renders a full-viewport canvas screensaver by default.

## Component Architecture

### Core Modules

The component is organized into several logical sections:

| Module | Purpose |
|--------|---------|
| `vec3` | 3D vector math utilities (add, sub, scale, dot, cross, normalize) |
| `rotateX/Y/Z` | Rotation matrix functions |
| `createToasterModel` | Generates toaster geometry (vertices + edges) |
| `createWingModel` | Generates articulated wing geometry |
| `FlyingToaster` | Entity class managing position, animation state |
| `computeVertexColor` | Vertex shader for Gouraud-style coloring |
| `project` | 3D to 2D perspective projection |
| `drawWireframeLine` | Renders gradient-interpolated wireframe edges |

### Geometry Data Structure

Models use a simple vertex/edge representation:

```javascript
{
  vertices: [
    [x, y, z],  // vertex 0
    [x, y, z],  // vertex 1
    // ...
  ],
  edges: [
    [0, 1],     // edge connecting vertex 0 to vertex 1
    [1, 2],     // edge connecting vertex 1 to vertex 2
    // ...
  ]
}
```

## Customization Options

### Adjusting Toaster Count

Modify the initialization loop in the `useEffect`:

```javascript
// Change from 8 to your desired count
for (let i = 0; i < 12; i++) {
  toastersRef.current.push(new FlyingToaster(width, height));
}
```

### Changing Colors

Edit the `computeVertexColor` function to change the color palette:

```javascript
const computeVertexColor = (vertex, normal, lightDir) => {
  // Modify these for different color schemes
  const baseColor = { r: 0, g: 200, b: 255 };      // Cyan
  const highlightColor = { r: 255, g: 100, b: 255 }; // Magenta
  
  // For a green/yellow scheme:
  // const baseColor = { r: 0, g: 255, b: 100 };
  // const highlightColor = { r: 255, g: 255, b: 0 };
  
  // ... rest of function
};
```

### Adjusting Flight Speed

Modify values in the `FlyingToaster.update()` method:

```javascript
update(canvasWidth, canvasHeight) {
  // Increase multipliers for faster movement
  this.x -= this.speed * 2;   // Horizontal speed
  this.y += this.speed * 1.5; // Vertical speed
  // ...
}
```

### Changing Flight Direction

The default flight pattern is top-right to bottom-left. To change direction, modify the `update()` method:

```javascript
// Original: top-right to bottom-left
this.x -= this.speed * 2;
this.y += this.speed * 1.5;

// Alternative: top-left to bottom-right
this.x += this.speed * 2;
this.y += this.speed * 1.5;

// Alternative: bottom to top
this.x += Math.sin(this.wobble) * this.speed;
this.y -= this.speed * 2;
```

### Wing Flap Speed

Adjust `wingSpeed` in the `FlyingToaster.reset()` method:

```javascript
this.wingSpeed = 0.15 + Math.random() * 0.05; // Default
this.wingSpeed = 0.25 + Math.random() * 0.1;  // Faster flapping
```

### Disabling Visual Effects

#### Remove Scanlines

Delete or comment out this block in the render loop:

```javascript
// Scanline effect for CRT authenticity
ctx.fillStyle = 'rgba(0, 0, 0, 0.03)';
for (let y = 0; y < height; y += 3) {
  ctx.fillRect(0, y, width, 1);
}
```

#### Remove Motion Trails

Change the clear operation:

```javascript
// With trails (default)
ctx.fillStyle = 'rgba(0, 0, 10, 0.3)';
ctx.fillRect(0, 0, width, height);

// Without trails
ctx.fillStyle = '#000008';
ctx.fillRect(0, 0, width, height);
```

#### Remove Vertex Glow

Delete the glow rendering block that starts with:

```javascript
// Add glow vertices at corners for that SVGA sparkle
for (let i = 0; i < 4; i++) {
  // ...
}
```

## Adding Custom 3D Models

You can add your own models following the vertex/edge pattern:

```javascript
const createCustomModel = () => {
  return {
    vertices: [
      // Define your vertices as [x, y, z] arrays
      [-1, -1, -1],
      [1, -1, -1],
      [1, 1, -1],
      [-1, 1, -1],
      // ... more vertices
    ],
    edges: [
      // Define edges as pairs of vertex indices
      [0, 1], [1, 2], [2, 3], [3, 0],
      // ... more edges
    ]
  };
};
```

### Tips for Model Creation

1. Center your model around origin (0, 0, 0)
2. Keep vertex coordinates in the -1 to 1 range for consistent scaling
3. Only define edges you want visible (no hidden line removal)
4. For complex models, organize vertices by face/section

## Integrating with React Router

For use as a screensaver route:

```jsx
import { BrowserRouter, Routes, Route } from 'react-router-dom';
import FlyingToastersScreensaver from './components/flying-toasters';

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Home />} />
        <Route path="/screensaver" element={<FlyingToastersScreensaver />} />
      </Routes>
    </BrowserRouter>
  );
}
```

## Idle Detection Integration

Trigger the screensaver after user inactivity:

```jsx
import { useState, useEffect } from 'react';
import FlyingToastersScreensaver from './components/flying-toasters';

function App() {
  const [isIdle, setIsIdle] = useState(false);
  const IDLE_TIMEOUT = 60000; // 1 minute

  useEffect(() => {
    let timeout;
    
    const resetTimer = () => {
      clearTimeout(timeout);
      setIsIdle(false);
      timeout = setTimeout(() => setIsIdle(true), IDLE_TIMEOUT);
    };

    const events = ['mousedown', 'mousemove', 'keypress', 'scroll', 'touchstart'];
    events.forEach(event => document.addEventListener(event, resetTimer));
    
    resetTimer();

    return () => {
      clearTimeout(timeout);
      events.forEach(event => document.removeEventListener(event, resetTimer));
    };
  }, []);

  if (isIdle) {
    return <FlyingToastersScreensaver />;
  }

  return <YourMainApp />;
}
```

## TypeScript Support

For TypeScript projects, add type definitions:

```typescript
interface Vec3 {
  add: (a: number[], b: number[]) => number[];
  sub: (a: number[], b: number[]) => number[];
  scale: (v: number[], s: number) => number[];
  dot: (a: number[], b: number[]) => number;
  cross: (a: number[], b: number[]) => number[];
  normalize: (v: number[]) => number[];
}

interface Model {
  vertices: number[][];
  edges: number[][];
}

interface ProjectedPoint {
  x: number;
  y: number;
  z: number;
  scale: number;
}

interface VertexColor {
  r: number;
  g: number;
  b: number;
}
```

## Performance Considerations

- **Toaster count**: Each toaster adds ~68 vertices and ~56 edges to render. Keep count under 20 for smooth 60fps on most devices.
- **Canvas size**: Full viewport rendering can be intensive on 4K displays. Consider limiting resolution.
- **Gradient strokes**: The gradient interpolation on each edge is expensive. For better performance, use solid colors.

### Performance Mode

For lower-end devices, create a simplified version:

```javascript
// Use solid colors instead of gradients
const drawWireframeLine = (ctx, p1, p2, color1, color2) => {
  if (!p1 || !p2) return;
  
  // Average the colors for solid line
  ctx.strokeStyle = `rgb(${(color1.r + color2.r) / 2}, ${(color1.g + color2.g) / 2}, ${(color1.b + color2.b) / 2})`;
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(p1.x, p1.y);
  ctx.lineTo(p2.x, p2.y);
  ctx.stroke();
};
```

## Browser Compatibility

| Browser | Support |
|---------|---------|
| Chrome 60+ | ✅ Full |
| Firefox 55+ | ✅ Full |
| Safari 11+ | ✅ Full |
| Edge 79+ | ✅ Full |
| IE 11 | ❌ Not supported |

## Troubleshooting

### Canvas not rendering

Ensure the parent container has defined dimensions:

```jsx
<div style={{ width: '100vw', height: '100vh' }}>
  <FlyingToastersScreensaver />
</div>
```

### Stuttering animation

- Reduce toaster count
- Disable motion trails
- Check for React StrictMode double-rendering in development

### Memory leaks

The component properly cleans up via `useEffect` return. If embedding in a larger app, ensure the component unmounts correctly.

## License

This component is provided for educational and nostalgic purposes. The original Flying Toasters screensaver was created by Berkeley Systems.
