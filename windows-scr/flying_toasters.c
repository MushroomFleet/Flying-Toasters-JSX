/*
 * Flying Toasters - SVGA Wireframe Screensaver
 * Native Windows .scr implementation
 * 
 * A tribute to Berkeley Systems' After Dark Flying Toasters
 * Rendered in vertex-shaded wireframe style
 *
 * Build: cl /O2 flying_toasters.c /link user32.lib gdi32.lib /OUT:flying_toasters.scr
 * Or use the provided Makefile with MinGW
 *
 * Author: Drift Johnson
 * Repository: https://github.com/MushroomFleet/Flying-Toasters-JSX
 */

#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <scrnsave.h>

/* ============================================
   CONFIGURATION
   ============================================ */

#define MAX_TOASTERS        12
#define DEFAULT_TOASTERS    8
#define TIMER_ID            1
#define FRAME_INTERVAL      16  /* ~60 FPS */

#define FOV                 400.0f
#define MAX_VERTICES        128
#define MAX_EDGES           128

/* Registry key for settings */
#define REG_KEY             "Software\\FlyingToastersScr"

/* ============================================
   TYPE DEFINITIONS
   ============================================ */

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int v1, v2;
} Edge;

typedef struct {
    Vec3 vertices[MAX_VERTICES];
    Edge edges[MAX_EDGES];
    int vertexCount;
    int edgeCount;
} Model;

typedef struct {
    BYTE r, g, b;
} Color;

typedef struct {
    float x, y, z;
    float scale;
} ProjectedPoint;

typedef struct {
    float x, y, z;
    float speed;
    float wobble;
    float wobbleSpeed;
    float wingPhase;
    float wingSpeed;
    float rotX, rotY;
    float scale;
    Model body;
    Model leftWing;
    Model rightWing;
} FlyingToaster;

/* ============================================
   GLOBAL STATE
   ============================================ */

static FlyingToaster g_toasters[MAX_TOASTERS];
static int g_toasterCount = DEFAULT_TOASTERS;
static int g_screenWidth = 0;
static int g_screenHeight = 0;
static HDC g_memDC = NULL;
static HBITMAP g_memBitmap = NULL;
static HBITMAP g_oldBitmap = NULL;
static BOOL g_showScanlines = TRUE;
static BOOL g_showGlow = TRUE;
static BOOL g_showTrails = TRUE;

/* Light direction (normalized) */
static Vec3 g_lightDir = { 0.408f, 0.816f, 0.408f };

/* ============================================
   VECTOR MATH
   ============================================ */

static Vec3 vec3_add(Vec3 a, Vec3 b) {
    Vec3 r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

static Vec3 vec3_sub(Vec3 a, Vec3 b) {
    Vec3 r = { a.x - b.x, a.y - b.y, a.z - b.z };
    return r;
}

static Vec3 vec3_scale(Vec3 v, float s) {
    Vec3 r = { v.x * s, v.y * s, v.z * s };
    return r;
}

static float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float vec3_length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len > 0.0001f) {
        return vec3_scale(v, 1.0f / len);
    }
    Vec3 zero = { 0, 0, 0 };
    return zero;
}

static Vec3 rotateX(Vec3 v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    Vec3 r = { v.x, v.y * c - v.z * s, v.y * s + v.z * c };
    return r;
}

static Vec3 rotateY(Vec3 v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    Vec3 r = { v.x * c + v.z * s, v.y, -v.x * s + v.z * c };
    return r;
}

static Vec3 rotateZ(Vec3 v, float angle) {
    float c = cosf(angle), s = sinf(angle);
    Vec3 r = { v.x * c - v.y * s, v.x * s + v.y * c, v.z };
    return r;
}

/* ============================================
   RANDOM UTILITIES
   ============================================ */

static float randf(void) {
    return (float)rand() / (float)RAND_MAX;
}

static float randf_range(float min, float max) {
    return min + randf() * (max - min);
}

/* ============================================
   MODEL CREATION
   ============================================ */

static void createToasterBody(Model* model) {
    model->vertexCount = 0;
    model->edgeCount = 0;
    
    /* Front face */
    model->vertices[0] = (Vec3){ -1.0f, -0.6f, 0.5f };
    model->vertices[1] = (Vec3){ 1.0f, -0.6f, 0.5f };
    model->vertices[2] = (Vec3){ 1.0f, 0.6f, 0.5f };
    model->vertices[3] = (Vec3){ -1.0f, 0.6f, 0.5f };
    
    /* Back face */
    model->vertices[4] = (Vec3){ -1.0f, -0.6f, -0.5f };
    model->vertices[5] = (Vec3){ 1.0f, -0.6f, -0.5f };
    model->vertices[6] = (Vec3){ 1.0f, 0.6f, -0.5f };
    model->vertices[7] = (Vec3){ -1.0f, 0.6f, -0.5f };
    
    /* Bread slot 1 */
    model->vertices[8] = (Vec3){ -0.7f, 0.6f, 0.3f };
    model->vertices[9] = (Vec3){ -0.3f, 0.6f, 0.3f };
    model->vertices[10] = (Vec3){ -0.3f, 0.6f, -0.3f };
    model->vertices[11] = (Vec3){ -0.7f, 0.6f, -0.3f };
    
    /* Bread slot 2 */
    model->vertices[12] = (Vec3){ 0.3f, 0.6f, 0.3f };
    model->vertices[13] = (Vec3){ 0.7f, 0.6f, 0.3f };
    model->vertices[14] = (Vec3){ 0.7f, 0.6f, -0.3f };
    model->vertices[15] = (Vec3){ 0.3f, 0.6f, -0.3f };
    
    /* Lever */
    model->vertices[16] = (Vec3){ 0.9f, 0.2f, 0.51f };
    model->vertices[17] = (Vec3){ 1.1f, 0.2f, 0.51f };
    model->vertices[18] = (Vec3){ 1.1f, 0.5f, 0.51f };
    model->vertices[19] = (Vec3){ 0.9f, 0.5f, 0.51f };
    
    model->vertexCount = 20;
    
    /* Front face edges */
    model->edges[0] = (Edge){ 0, 1 };
    model->edges[1] = (Edge){ 1, 2 };
    model->edges[2] = (Edge){ 2, 3 };
    model->edges[3] = (Edge){ 3, 0 };
    
    /* Back face edges */
    model->edges[4] = (Edge){ 4, 5 };
    model->edges[5] = (Edge){ 5, 6 };
    model->edges[6] = (Edge){ 6, 7 };
    model->edges[7] = (Edge){ 7, 4 };
    
    /* Connecting edges */
    model->edges[8] = (Edge){ 0, 4 };
    model->edges[9] = (Edge){ 1, 5 };
    model->edges[10] = (Edge){ 2, 6 };
    model->edges[11] = (Edge){ 3, 7 };
    
    /* Slot 1 */
    model->edges[12] = (Edge){ 8, 9 };
    model->edges[13] = (Edge){ 9, 10 };
    model->edges[14] = (Edge){ 10, 11 };
    model->edges[15] = (Edge){ 11, 8 };
    
    /* Slot 2 */
    model->edges[16] = (Edge){ 12, 13 };
    model->edges[17] = (Edge){ 13, 14 };
    model->edges[18] = (Edge){ 14, 15 };
    model->edges[19] = (Edge){ 15, 12 };
    
    /* Lever */
    model->edges[20] = (Edge){ 16, 17 };
    model->edges[21] = (Edge){ 17, 18 };
    model->edges[22] = (Edge){ 18, 19 };
    model->edges[23] = (Edge){ 19, 16 };
    
    model->edgeCount = 24;
}

static void createWing(Model* model, int isLeft) {
    float mirror = isLeft ? -1.0f : 1.0f;
    float baseX = mirror * 1.0f;
    float wingLength = 1.8f;
    int wingSegments = 5;
    int i;
    
    model->vertexCount = 0;
    model->edgeCount = 0;
    
    /* Create wing segments */
    for (i = 0; i <= wingSegments; i++) {
        float t = (float)i / wingSegments;
        float x = baseX + mirror * t * wingLength;
        float featherWidth = 0.4f * (1.0f - t * 0.5f);
        int base = i * 4;
        
        model->vertices[base + 0] = (Vec3){ x, 0.3f, featherWidth };
        model->vertices[base + 1] = (Vec3){ x, 0.3f, -featherWidth };
        model->vertices[base + 2] = (Vec3){ x, 0.1f, featherWidth * 0.7f };
        model->vertices[base + 3] = (Vec3){ x, 0.1f, -featherWidth * 0.7f };
    }
    model->vertexCount = (wingSegments + 1) * 4;
    
    /* Connect wing segments */
    for (i = 0; i < wingSegments; i++) {
        int base = i * 4;
        int edgeBase = model->edgeCount;
        
        /* Horizontal connections */
        model->edges[edgeBase + 0] = (Edge){ base, base + 1 };
        model->edges[edgeBase + 1] = (Edge){ base + 2, base + 3 };
        
        /* Vertical connections */
        model->edges[edgeBase + 2] = (Edge){ base, base + 2 };
        model->edges[edgeBase + 3] = (Edge){ base + 1, base + 3 };
        
        /* To next segment */
        model->edges[edgeBase + 4] = (Edge){ base, base + 4 };
        model->edges[edgeBase + 5] = (Edge){ base + 1, base + 5 };
        model->edges[edgeBase + 6] = (Edge){ base + 2, base + 6 };
        model->edges[edgeBase + 7] = (Edge){ base + 3, base + 7 };
        
        /* Cross bracing */
        model->edges[edgeBase + 8] = (Edge){ base, base + 5 };
        model->edges[edgeBase + 9] = (Edge){ base + 1, base + 4 };
        
        model->edgeCount += 10;
    }
    
    /* Final segment */
    {
        int last = wingSegments * 4;
        int edgeBase = model->edgeCount;
        model->edges[edgeBase + 0] = (Edge){ last, last + 1 };
        model->edges[edgeBase + 1] = (Edge){ last + 2, last + 3 };
        model->edges[edgeBase + 2] = (Edge){ last, last + 2 };
        model->edges[edgeBase + 3] = (Edge){ last + 1, last + 3 };
        model->edgeCount += 4;
    }
}

/* ============================================
   TOASTER MANAGEMENT
   ============================================ */

static void resetToaster(FlyingToaster* toaster, BOOL initial) {
    if (initial) {
        toaster->x = randf() * g_screenWidth;
        toaster->y = randf() * g_screenHeight;
    } else {
        toaster->x = g_screenWidth + 100.0f + randf() * 200.0f;
        toaster->y = -100.0f - randf() * 200.0f;
    }
    
    toaster->z = 200.0f + randf() * 400.0f;
    toaster->speed = 1.5f + randf() * 1.5f;
    toaster->wobble = randf() * (float)M_PI * 2.0f;
    toaster->wobbleSpeed = 0.02f + randf() * 0.02f;
    toaster->wingPhase = randf() * (float)M_PI * 2.0f;
    toaster->wingSpeed = 0.15f + randf() * 0.05f;
    toaster->rotY = -0.3f + randf() * 0.2f;
    toaster->rotX = 0.2f + randf() * 0.1f;
    toaster->scale = 40.0f + randf() * 30.0f;
}

static void initToaster(FlyingToaster* toaster) {
    createToasterBody(&toaster->body);
    createWing(&toaster->leftWing, 1);
    createWing(&toaster->rightWing, 0);
    resetToaster(toaster, TRUE);
}

static void updateToaster(FlyingToaster* toaster) {
    /* Flight movement: top-right to bottom-left */
    toaster->x -= toaster->speed * 2.0f;
    toaster->y += toaster->speed * 1.5f;
    toaster->wobble += toaster->wobbleSpeed;
    toaster->wingPhase += toaster->wingSpeed;
    
    /* Reset if off screen */
    if (toaster->x < -200 || toaster->y > g_screenHeight + 200) {
        resetToaster(toaster, FALSE);
    }
}

static float getWingAngle(FlyingToaster* toaster) {
    return sinf(toaster->wingPhase) * 0.5f;
}

/* ============================================
   RENDERING
   ============================================ */

static Color computeVertexColor(Vec3 vertex, Vec3 normal) {
    Color baseColor = { 0, 200, 255 };      /* Cyan */
    Color highlightColor = { 255, 100, 255 }; /* Magenta */
    
    float ndotl = vec3_dot(normal, g_lightDir);
    if (ndotl < 0) ndotl = 0;
    
    float ambient = 0.3f;
    float diffuse = 0.7f * ndotl;
    float intensity = ambient + diffuse;
    
    /* Height-based color blend */
    float heightBlend = (vertex.y + 1.0f) / 2.0f;
    if (heightBlend < 0) heightBlend = 0;
    if (heightBlend > 1) heightBlend = 1;
    
    Color result;
    result.r = (BYTE)(((float)baseColor.r * (1.0f - heightBlend) + 
                       (float)highlightColor.r * heightBlend) * intensity);
    result.g = (BYTE)(((float)baseColor.g * (1.0f - heightBlend) + 
                       (float)highlightColor.g * heightBlend) * intensity);
    result.b = (BYTE)(((float)baseColor.b * (1.0f - heightBlend) + 
                       (float)highlightColor.b * heightBlend) * intensity);
    
    return result;
}

static BOOL project(Vec3 vertex, float centerX, float centerY, ProjectedPoint* out) {
    float z = vertex.z + FOV;
    if (z <= 0) return FALSE;
    
    float scale = FOV / z;
    out->x = centerX + vertex.x * scale;
    out->y = centerY - vertex.y * scale;
    out->z = z;
    out->scale = scale;
    return TRUE;
}

static void drawGradientLine(HDC hdc, ProjectedPoint p1, ProjectedPoint p2, 
                             Color c1, Color c2) {
    /* Bresenham with color interpolation */
    int x0 = (int)p1.x, y0 = (int)p1.y;
    int x1 = (int)p2.x, y1 = (int)p2.y;
    
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    int steps = dx > dy ? dx : dy;
    if (steps == 0) steps = 1;
    
    float lineWidth = (p1.scale + p2.scale) * 0.4f;
    if (lineWidth < 1.0f) lineWidth = 1.0f;
    if (lineWidth > 4.0f) lineWidth = 4.0f;
    
    HPEN pen = NULL;
    HPEN oldPen = NULL;
    int lastR = -1, lastG = -1, lastB = -1;
    
    int step = 0;
    while (1) {
        float t = (float)step / (float)steps;
        
        int r = (int)(c1.r + (c2.r - c1.r) * t);
        int g = (int)(c1.g + (c2.g - c1.g) * t);
        int b = (int)(c1.b + (c2.b - c1.b) * t);
        
        /* Only create new pen if color changed significantly */
        if (abs(r - lastR) > 8 || abs(g - lastG) > 8 || abs(b - lastB) > 8 || pen == NULL) {
            if (pen) {
                SelectObject(hdc, oldPen);
                DeleteObject(pen);
            }
            pen = CreatePen(PS_SOLID, (int)lineWidth, RGB(r, g, b));
            oldPen = (HPEN)SelectObject(hdc, pen);
            lastR = r; lastG = g; lastB = b;
        }
        
        /* Draw a small segment */
        MoveToEx(hdc, x0, y0, NULL);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        int nextX = x0, nextY = y0;
        
        if (e2 > -dy) { err -= dy; nextX += sx; }
        if (e2 < dx) { err += dx; nextY += sy; }
        
        LineTo(hdc, nextX, nextY);
        
        x0 = nextX;
        y0 = nextY;
        step++;
    }
    
    if (pen) {
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}

static void drawVertexGlow(HDC hdc, ProjectedPoint p, Color c) {
    if (!g_showGlow) return;
    
    int glowSize = (int)(3.0f * p.scale);
    if (glowSize < 2) glowSize = 2;
    if (glowSize > 20) glowSize = 20;
    
    /* Simple radial glow approximation */
    int i;
    for (i = glowSize; i > 0; i -= 2) {
        float t = (float)i / (float)glowSize;
        int alpha = (int)(255 * (1.0f - t) * 0.3f);
        
        int r = c.r + (255 - c.r) * (1.0f - t);
        int g = c.g + (255 - c.g) * (1.0f - t);
        int b = c.b + (255 - c.b) * (1.0f - t);
        
        HBRUSH brush = CreateSolidBrush(RGB(r * alpha / 255, 
                                            g * alpha / 255, 
                                            b * alpha / 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        HPEN pen = CreatePen(PS_NULL, 0, 0);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        
        Ellipse(hdc, (int)p.x - i, (int)p.y - i, 
                     (int)p.x + i, (int)p.y + i);
        
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}

static void renderToaster(HDC hdc, FlyingToaster* toaster) {
    float centerX = toaster->x;
    float centerY = toaster->y;
    float wingAngle = getWingAngle(toaster);
    
    Vec3 transformedVerts[MAX_VERTICES];
    ProjectedPoint projectedVerts[MAX_VERTICES];
    Color vertexColors[MAX_VERTICES];
    BOOL validVerts[MAX_VERTICES];
    
    int i;
    
    /* Transform and project body vertices */
    for (i = 0; i < toaster->body.vertexCount; i++) {
        Vec3 v = toaster->body.vertices[i];
        v = vec3_scale(v, toaster->scale);
        v = rotateX(v, toaster->rotX);
        v = rotateY(v, toaster->rotY);
        v.z += toaster->z;
        transformedVerts[i] = v;
        
        Vec3 normal = vec3_normalize((Vec3){ v.x * 0.3f, v.y, v.z * 0.5f });
        vertexColors[i] = computeVertexColor(v, normal);
        validVerts[i] = project(v, centerX, centerY, &projectedVerts[i]);
    }
    
    /* Draw body edges */
    for (i = 0; i < toaster->body.edgeCount; i++) {
        int v1 = toaster->body.edges[i].v1;
        int v2 = toaster->body.edges[i].v2;
        
        if (validVerts[v1] && validVerts[v2]) {
            drawGradientLine(hdc, projectedVerts[v1], projectedVerts[v2],
                           vertexColors[v1], vertexColors[v2]);
        }
    }
    
    /* Draw vertex glow on corners */
    for (i = 0; i < 4 && i < toaster->body.vertexCount; i++) {
        if (validVerts[i]) {
            drawVertexGlow(hdc, projectedVerts[i], vertexColors[i]);
        }
    }
    
    /* Render wings */
    Model* wings[2] = { &toaster->leftWing, &toaster->rightWing };
    int isLeft[2] = { 1, 0 };
    
    int w;
    for (w = 0; w < 2; w++) {
        Model* wing = wings[w];
        float flapAngle = isLeft[w] ? -wingAngle : wingAngle;
        float pivotX = isLeft[w] ? -1.0f : 1.0f;
        
        /* Transform wing vertices */
        for (i = 0; i < wing->vertexCount; i++) {
            Vec3 v = wing->vertices[i];
            
            /* Apply wing flap around pivot */
            v.x -= pivotX;
            v = rotateZ(v, flapAngle);
            v.x += pivotX;
            
            /* Apply toaster transform */
            v = vec3_scale(v, toaster->scale);
            v = rotateX(v, toaster->rotX);
            v = rotateY(v, toaster->rotY);
            v.z += toaster->z;
            transformedVerts[i] = v;
            
            /* Wing coloring - lighter/golden */
            Vec3 normal = vec3_normalize((Vec3){ 0, 1.0f, 0 });
            Color baseColor = computeVertexColor(v, normal);
            vertexColors[i].r = (BYTE)min(255, baseColor.r + 100);
            vertexColors[i].g = (BYTE)min(255, baseColor.g + 50);
            vertexColors[i].b = baseColor.b;
            
            validVerts[i] = project(v, centerX, centerY, &projectedVerts[i]);
        }
        
        /* Draw wing edges */
        for (i = 0; i < wing->edgeCount; i++) {
            int v1 = wing->edges[i].v1;
            int v2 = wing->edges[i].v2;
            
            if (validVerts[v1] && validVerts[v2]) {
                drawGradientLine(hdc, projectedVerts[v1], projectedVerts[v2],
                               vertexColors[v1], vertexColors[v2]);
            }
        }
    }
}

static int compareToasterDepth(const void* a, const void* b) {
    const FlyingToaster* ta = (const FlyingToaster*)a;
    const FlyingToaster* tb = (const FlyingToaster*)b;
    if (ta->z > tb->z) return -1;
    if (ta->z < tb->z) return 1;
    return 0;
}

static void renderFrame(HDC hdc) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = g_screenWidth;
    rect.bottom = g_screenHeight;
    
    /* Clear with trail effect or solid */
    if (g_showTrails) {
        /* Semi-transparent overlay for trails */
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 8));
        FillRect(g_memDC, &rect, brush);
        DeleteObject(brush);
    } else {
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 8));
        FillRect(g_memDC, &rect, brush);
        DeleteObject(brush);
    }
    
    /* Sort toasters by depth */
    FlyingToaster sortedToasters[MAX_TOASTERS];
    memcpy(sortedToasters, g_toasters, sizeof(FlyingToaster) * g_toasterCount);
    qsort(sortedToasters, g_toasterCount, sizeof(FlyingToaster), compareToasterDepth);
    
    /* Update and render each toaster */
    int i;
    for (i = 0; i < g_toasterCount; i++) {
        /* Update original (not sorted copy) */
        updateToaster(&g_toasters[i]);
    }
    
    for (i = 0; i < g_toasterCount; i++) {
        renderToaster(g_memDC, &sortedToasters[i]);
    }
    
    /* Scanline effect */
    if (g_showScanlines) {
        int y;
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(g_memDC, pen);
        
        for (y = 0; y < g_screenHeight; y += 3) {
            MoveToEx(g_memDC, 0, y, NULL);
            LineTo(g_memDC, g_screenWidth, y);
        }
        
        SelectObject(g_memDC, oldPen);
        DeleteObject(pen);
    }
    
    /* Blit to screen */
    BitBlt(hdc, 0, 0, g_screenWidth, g_screenHeight, g_memDC, 0, 0, SRCCOPY);
}

/* ============================================
   SETTINGS PERSISTENCE
   ============================================ */

static void loadSettings(void) {
    HKEY hKey;
    DWORD value, size;
    
    if (RegOpenKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        size = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "ToasterCount", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            g_toasterCount = value;
            if (g_toasterCount < 1) g_toasterCount = 1;
            if (g_toasterCount > MAX_TOASTERS) g_toasterCount = MAX_TOASTERS;
        }
        
        size = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "Scanlines", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            g_showScanlines = value ? TRUE : FALSE;
        }
        
        size = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "Glow", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            g_showGlow = value ? TRUE : FALSE;
        }
        
        size = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "Trails", NULL, NULL, (LPBYTE)&value, &size) == ERROR_SUCCESS) {
            g_showTrails = value ? TRUE : FALSE;
        }
        
        RegCloseKey(hKey);
    }
}

static void saveSettings(void) {
    HKEY hKey;
    DWORD value;
    
    if (RegCreateKeyExA(HKEY_CURRENT_USER, REG_KEY, 0, NULL, 0, 
                        KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        value = g_toasterCount;
        RegSetValueExA(hKey, "ToasterCount", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = g_showScanlines ? 1 : 0;
        RegSetValueExA(hKey, "Scanlines", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = g_showGlow ? 1 : 0;
        RegSetValueExA(hKey, "Glow", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        value = g_showTrails ? 1 : 0;
        RegSetValueExA(hKey, "Trails", 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD));
        
        RegCloseKey(hKey);
    }
}

/* ============================================
   CONFIGURATION DIALOG
   ============================================ */

#define IDC_TOASTER_SLIDER   1001
#define IDC_TOASTER_LABEL    1002
#define IDC_SCANLINES        1003
#define IDC_GLOW             1004
#define IDC_TRAILS           1005

static INT_PTR CALLBACK ConfigDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hSlider;
    static HWND hLabel;
    
    switch (msg) {
        case WM_INITDIALOG:
            loadSettings();
            
            hSlider = GetDlgItem(hDlg, IDC_TOASTER_SLIDER);
            hLabel = GetDlgItem(hDlg, IDC_TOASTER_LABEL);
            
            SendMessage(hSlider, TBM_SETRANGE, TRUE, MAKELONG(1, MAX_TOASTERS));
            SendMessage(hSlider, TBM_SETPOS, TRUE, g_toasterCount);
            
            {
                char buf[32];
                wsprintfA(buf, "Toasters: %d", g_toasterCount);
                SetWindowTextA(hLabel, buf);
            }
            
            CheckDlgButton(hDlg, IDC_SCANLINES, g_showScanlines ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_GLOW, g_showGlow ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_TRAILS, g_showTrails ? BST_CHECKED : BST_UNCHECKED);
            
            return TRUE;
            
        case WM_HSCROLL:
            if ((HWND)lParam == hSlider) {
                g_toasterCount = (int)SendMessage(hSlider, TBM_GETPOS, 0, 0);
                char buf[32];
                wsprintfA(buf, "Toasters: %d", g_toasterCount);
                SetWindowTextA(hLabel, buf);
            }
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    g_showScanlines = IsDlgButtonChecked(hDlg, IDC_SCANLINES) == BST_CHECKED;
                    g_showGlow = IsDlgButtonChecked(hDlg, IDC_GLOW) == BST_CHECKED;
                    g_showTrails = IsDlgButtonChecked(hDlg, IDC_TRAILS) == BST_CHECKED;
                    saveSettings();
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                    
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
            
        case WM_CLOSE:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
    }
    
    return FALSE;
}

/* ============================================
   SCREENSAVER ENTRY POINTS
   ============================================ */

LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            srand((unsigned int)time(NULL));
            loadSettings();
            
            /* Get screen dimensions */
            {
                RECT rect;
                GetClientRect(hWnd, &rect);
                g_screenWidth = rect.right;
                g_screenHeight = rect.bottom;
            }
            
            /* Create double buffer */
            {
                HDC hdc = GetDC(hWnd);
                g_memDC = CreateCompatibleDC(hdc);
                g_memBitmap = CreateCompatibleBitmap(hdc, g_screenWidth, g_screenHeight);
                g_oldBitmap = (HBITMAP)SelectObject(g_memDC, g_memBitmap);
                ReleaseDC(hWnd, hdc);
            }
            
            /* Initialize toasters */
            {
                int i;
                for (i = 0; i < g_toasterCount; i++) {
                    initToaster(&g_toasters[i]);
                }
            }
            
            /* Start animation timer */
            SetTimer(hWnd, TIMER_ID, FRAME_INTERVAL, NULL);
            return 0;
            
        case WM_TIMER:
            if (wParam == TIMER_ID) {
                HDC hdc = GetDC(hWnd);
                renderFrame(hdc);
                ReleaseDC(hWnd, hdc);
            }
            return 0;
            
        case WM_DESTROY:
            KillTimer(hWnd, TIMER_ID);
            
            if (g_memDC) {
                SelectObject(g_memDC, g_oldBitmap);
                DeleteObject(g_memBitmap);
                DeleteDC(g_memDC);
            }
            
            PostQuitMessage(0);
            return 0;
    }
    
    return DefScreenSaverProc(hWnd, msg, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    return (BOOL)ConfigDialogProc(hDlg, msg, wParam, lParam);
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
    return TRUE;
}
