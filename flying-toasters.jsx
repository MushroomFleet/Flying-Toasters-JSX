import React, { useEffect, useRef, useState } from 'react';

// 3D Math utilities
const vec3 = {
  add: (a, b) => [a[0] + b[0], a[1] + b[1], a[2] + b[2]],
  sub: (a, b) => [a[0] - b[0], a[1] - b[1], a[2] - b[2]],
  scale: (v, s) => [v[0] * s, v[1] * s, v[2] * s],
  dot: (a, b) => a[0] * b[0] + a[1] * b[1] + a[2] * b[2],
  cross: (a, b) => [
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0]
  ],
  normalize: (v) => {
    const len = Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    return len > 0 ? [v[0] / len, v[1] / len, v[2] / len] : [0, 0, 0];
  },
  length: (v) => Math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])
};

// Rotation matrices
const rotateX = (v, angle) => {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0], v[1] * c - v[2] * s, v[1] * s + v[2] * c];
};

const rotateY = (v, angle) => {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0] * c + v[2] * s, v[1], -v[0] * s + v[2] * c];
};

const rotateZ = (v, angle) => {
  const c = Math.cos(angle), s = Math.sin(angle);
  return [v[0] * c - v[1] * s, v[0] * s + v[1] * c, v[2]];
};

// Toaster 3D model - vertices and edges
const createToasterModel = () => {
  // Main body - a rounded rectangular box
  const body = {
    vertices: [
      // Front face
      [-1, -0.6, 0.5], [1, -0.6, 0.5], [1, 0.6, 0.5], [-1, 0.6, 0.5],
      // Back face  
      [-1, -0.6, -0.5], [1, -0.6, -0.5], [1, 0.6, -0.5], [-1, 0.6, -0.5],
      // Bread slots (top)
      [-0.7, 0.6, 0.3], [-0.3, 0.6, 0.3], [-0.3, 0.6, -0.3], [-0.7, 0.6, -0.3],
      [0.3, 0.6, 0.3], [0.7, 0.6, 0.3], [0.7, 0.6, -0.3], [0.3, 0.6, -0.3],
      // Lever
      [0.9, 0.2, 0.51], [1.1, 0.2, 0.51], [1.1, 0.5, 0.51], [0.9, 0.5, 0.51],
    ],
    edges: [
      // Front face
      [0, 1], [1, 2], [2, 3], [3, 0],
      // Back face
      [4, 5], [5, 6], [6, 7], [7, 4],
      // Connecting edges
      [0, 4], [1, 5], [2, 6], [3, 7],
      // Slot 1
      [8, 9], [9, 10], [10, 11], [11, 8],
      // Slot 2
      [12, 13], [13, 14], [14, 15], [15, 12],
      // Lever
      [16, 17], [17, 18], [18, 19], [19, 16],
    ],
    vertexColors: [] // Will be computed based on lighting
  };
  
  return body;
};

// Wing model - feathered wing shape
const createWingModel = (side) => {
  const mirror = side === 'left' ? -1 : 1;
  const vertices = [];
  const edges = [];
  
  // Wing base attached to toaster
  const baseX = mirror * 1.0;
  
  // Create wing segments (feather-like)
  const wingLength = 1.8;
  const wingSegments = 5;
  
  for (let i = 0; i <= wingSegments; i++) {
    const t = i / wingSegments;
    const x = baseX + mirror * t * wingLength;
    const featherWidth = 0.4 * (1 - t * 0.5);
    
    vertices.push([x, 0.3, featherWidth]);
    vertices.push([x, 0.3, -featherWidth]);
    vertices.push([x, 0.1, featherWidth * 0.7]);
    vertices.push([x, 0.1, -featherWidth * 0.7]);
  }
  
  // Connect wing segments
  for (let i = 0; i < wingSegments; i++) {
    const base = i * 4;
    // Horizontal connections
    edges.push([base, base + 1]);
    edges.push([base + 2, base + 3]);
    // Vertical connections  
    edges.push([base, base + 2]);
    edges.push([base + 1, base + 3]);
    // To next segment
    edges.push([base, base + 4]);
    edges.push([base + 1, base + 5]);
    edges.push([base + 2, base + 6]);
    edges.push([base + 3, base + 7]);
    // Cross bracing for detail
    edges.push([base, base + 5]);
    edges.push([base + 1, base + 4]);
  }
  // Final segment horizontal
  const last = wingSegments * 4;
  edges.push([last, last + 1]);
  edges.push([last + 2, last + 3]);
  edges.push([last, last + 2]);
  edges.push([last + 1, last + 3]);
  
  return { vertices, edges, side };
};

// Flying toaster entity
class FlyingToaster {
  constructor(canvasWidth, canvasHeight) {
    this.reset(canvasWidth, canvasHeight, true);
    this.body = createToasterModel();
    this.leftWing = createWingModel('left');
    this.rightWing = createWingModel('right');
  }
  
  reset(canvasWidth, canvasHeight, initial = false) {
    if (initial) {
      this.x = Math.random() * canvasWidth;
      this.y = Math.random() * canvasHeight;
    } else {
      // Spawn from top-right
      this.x = canvasWidth + 100 + Math.random() * 200;
      this.y = -100 - Math.random() * 200;
    }
    this.z = 200 + Math.random() * 400;
    this.speed = 1.5 + Math.random() * 1.5;
    this.wobble = Math.random() * Math.PI * 2;
    this.wobbleSpeed = 0.02 + Math.random() * 0.02;
    this.wingPhase = Math.random() * Math.PI * 2;
    this.wingSpeed = 0.15 + Math.random() * 0.05;
    this.rotY = -0.3 + Math.random() * 0.2;
    this.rotX = 0.2 + Math.random() * 0.1;
    this.scale = 40 + Math.random() * 30;
  }
  
  update(canvasWidth, canvasHeight) {
    // Flying direction: top-right to bottom-left
    this.x -= this.speed * 2;
    this.y += this.speed * 1.5;
    this.wobble += this.wobbleSpeed;
    this.wingPhase += this.wingSpeed;
    
    // Small wobble in flight
    const wobbleOffset = Math.sin(this.wobble) * 0.5;
    
    // Reset if off screen
    if (this.x < -200 || this.y > canvasHeight + 200) {
      this.reset(canvasWidth, canvasHeight);
    }
  }
  
  getWingAngle() {
    // Flapping motion
    return Math.sin(this.wingPhase) * 0.5;
  }
}

// Vertex shader - compute color based on position and normal approximation
const computeVertexColor = (vertex, normal, lightDir) => {
  // Base colors - cyan/magenta SVGA palette feel
  const baseColor = { r: 0, g: 200, b: 255 };
  const highlightColor = { r: 255, g: 100, b: 255 };
  
  // Simple diffuse lighting
  const ndotl = Math.max(0, vec3.dot(normal, lightDir));
  const ambient = 0.3;
  const diffuse = 0.7 * ndotl;
  const intensity = ambient + diffuse;
  
  // Height-based color blend for that vertex shader look
  const heightBlend = (vertex[1] + 1) / 2;
  
  return {
    r: Math.floor(baseColor.r * (1 - heightBlend) + highlightColor.r * heightBlend) * intensity,
    g: Math.floor(baseColor.g * (1 - heightBlend) + highlightColor.g * heightBlend) * intensity,
    b: Math.floor(baseColor.b * (1 - heightBlend) + highlightColor.b * heightBlend) * intensity
  };
};

// Project 3D to 2D with perspective
const project = (vertex, centerX, centerY, fov = 400) => {
  const z = vertex[2] + fov;
  if (z <= 0) return null;
  const scale = fov / z;
  return {
    x: centerX + vertex[0] * scale,
    y: centerY - vertex[1] * scale,
    z: z,
    scale: scale
  };
};

// Draw wireframe line with vertex color interpolation (Gouraud-style)
const drawWireframeLine = (ctx, p1, p2, color1, color2) => {
  if (!p1 || !p2) return;
  
  const gradient = ctx.createLinearGradient(p1.x, p1.y, p2.x, p2.y);
  gradient.addColorStop(0, `rgb(${color1.r}, ${color1.g}, ${color1.b})`);
  gradient.addColorStop(1, `rgb(${color2.r}, ${color2.g}, ${color2.b})`);
  
  ctx.strokeStyle = gradient;
  ctx.lineWidth = Math.max(1, (p1.scale + p2.scale) * 0.5);
  ctx.beginPath();
  ctx.moveTo(p1.x, p1.y);
  ctx.lineTo(p2.x, p2.y);
  ctx.stroke();
};

// Main component
export default function FlyingToastersScreensaver() {
  const canvasRef = useRef(null);
  const toastersRef = useRef([]);
  const animationRef = useRef(null);
  const [dimensions, setDimensions] = useState({ width: 800, height: 600 });
  
  useEffect(() => {
    const updateDimensions = () => {
      setDimensions({ width: window.innerWidth, height: window.innerHeight });
    };
    updateDimensions();
    window.addEventListener('resize', updateDimensions);
    return () => window.removeEventListener('resize', updateDimensions);
  }, []);
  
  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const { width, height } = dimensions;
    
    // Initialize toasters
    if (toastersRef.current.length === 0) {
      for (let i = 0; i < 8; i++) {
        toastersRef.current.push(new FlyingToaster(width, height));
      }
    }
    
    // Light direction (from top-right-front)
    const lightDir = vec3.normalize([0.5, 0.7, 0.5]);
    
    const render = () => {
      // Clear with slight trail effect for that CRT feel
      ctx.fillStyle = 'rgba(0, 0, 10, 0.3)';
      ctx.fillRect(0, 0, width, height);
      
      // Sort toasters by Z for proper depth ordering
      const sortedToasters = [...toastersRef.current].sort((a, b) => b.z - a.z);
      
      for (const toaster of sortedToasters) {
        toaster.update(width, height);
        
        const centerX = toaster.x;
        const centerY = toaster.y;
        const wingAngle = toaster.getWingAngle();
        
        // Transform and render body
        const transformVertex = (v) => {
          let transformed = vec3.scale(v, toaster.scale);
          transformed = rotateX(transformed, toaster.rotX);
          transformed = rotateY(transformed, toaster.rotY);
          transformed[2] += toaster.z;
          return transformed;
        };
        
        // Transform wing vertex with flap
        const transformWingVertex = (v, wing) => {
          let transformed = [...v];
          // Apply wing flap rotation around the attachment point
          const flapAngle = wing.side === 'left' ? -wingAngle : wingAngle;
          // Rotate around Z axis at wing base
          const pivotX = wing.side === 'left' ? -1.0 : 1.0;
          transformed[0] -= pivotX;
          transformed = rotateZ(transformed, flapAngle);
          transformed[0] += pivotX;
          // Scale and main rotation
          transformed = vec3.scale(transformed, toaster.scale);
          transformed = rotateX(transformed, toaster.rotX);
          transformed = rotateY(transformed, toaster.rotY);
          transformed[2] += toaster.z;
          return transformed;
        };
        
        // Render body
        const bodyVertices = toaster.body.vertices.map(transformVertex);
        const bodyProjected = bodyVertices.map(v => project(v, centerX, centerY));
        const bodyColors = bodyVertices.map(v => {
          const normal = vec3.normalize([v[0] * 0.3, v[1], v[2] * 0.5]);
          return computeVertexColor(v, normal, lightDir);
        });
        
        ctx.lineCap = 'round';
        ctx.lineJoin = 'round';
        
        for (const edge of toaster.body.edges) {
          drawWireframeLine(
            ctx,
            bodyProjected[edge[0]],
            bodyProjected[edge[1]],
            bodyColors[edge[0]],
            bodyColors[edge[1]]
          );
        }
        
        // Render wings
        for (const wing of [toaster.leftWing, toaster.rightWing]) {
          const wingVertices = wing.vertices.map(v => transformWingVertex(v, wing));
          const wingProjected = wingVertices.map(v => project(v, centerX, centerY));
          const wingColors = wingVertices.map(v => {
            const normal = vec3.normalize([0, 1, 0]);
            // Wings get a golden/white color
            const base = computeVertexColor(v, normal, lightDir);
            return {
              r: Math.min(255, base.r + 100),
              g: Math.min(255, base.g + 50),
              b: base.b
            };
          });
          
          for (const edge of wing.edges) {
            if (wingProjected[edge[0]] && wingProjected[edge[1]]) {
              drawWireframeLine(
                ctx,
                wingProjected[edge[0]],
                wingProjected[edge[1]],
                wingColors[edge[0]],
                wingColors[edge[1]]
              );
            }
          }
        }
        
        // Add glow vertices at corners for that SVGA sparkle
        for (let i = 0; i < 4; i++) {
          const p = bodyProjected[i];
          if (p && p.z > 0) {
            const glowSize = 3 * p.scale;
            const gradient = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, glowSize);
            gradient.addColorStop(0, 'rgba(255, 255, 255, 0.8)');
            gradient.addColorStop(0.5, 'rgba(100, 200, 255, 0.3)');
            gradient.addColorStop(1, 'rgba(100, 200, 255, 0)');
            ctx.fillStyle = gradient;
            ctx.beginPath();
            ctx.arc(p.x, p.y, glowSize, 0, Math.PI * 2);
            ctx.fill();
          }
        }
      }
      
      // Scanline effect for CRT authenticity
      ctx.fillStyle = 'rgba(0, 0, 0, 0.03)';
      for (let y = 0; y < height; y += 3) {
        ctx.fillRect(0, y, width, 1);
      }
      
      animationRef.current = requestAnimationFrame(render);
    };
    
    render();
    
    return () => {
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, [dimensions]);
  
  return (
    <div style={{ 
      width: '100vw', 
      height: '100vh', 
      overflow: 'hidden', 
      background: '#000008',
      cursor: 'none'
    }}>
      <canvas
        ref={canvasRef}
        width={dimensions.width}
        height={dimensions.height}
        style={{ display: 'block' }}
      />
      <div style={{
        position: 'absolute',
        bottom: 20,
        left: 20,
        color: '#0088aa',
        fontFamily: 'monospace',
        fontSize: 12,
        opacity: 0.6,
        textShadow: '0 0 10px #00aaff'
      }}>
        FLYING TOASTERS • SVGA WIREFRAME • 640×480×256
      </div>
    </div>
  );
}
