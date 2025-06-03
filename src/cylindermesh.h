#pragma once
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include "cylinderwidget.h"

/* ------------------------------------------------------------------ */
/* Toggle all diagnostics by (un)-defining this symbol once.          */
#define CYL_DEBUG           /*  ← comment-out for normal build       */
/* ------------------------------------------------------------------ */

#ifndef CYL_DEBUG
constexpr int CYL_SLICES = 40;     /* regular resolution             */
#else
constexpr int CYL_SLICES = 40;      /* just four quads – easy to see  */
#endif

/* Builds a closed cylinder centred at (0,0,0); radius = 1, height = 2
   This completely rebuilt version ensures consistent winding order and eliminates artifacts */
inline void makeCylinderMesh(QVector<Vtx>& vbo,
                             QVector<Tri>& ibo,
                             int n = CYL_SLICES)
{
    vbo.clear(); ibo.clear();

    /* STEP 1: Generate side vertices with careful attention to wrap-around
     * We create exactly n+1 columns of vertices, where the last column
     * is identical in position to the first, but has u=1.0 for texture wrapping */

    for (int i = 0; i <= n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);

        // UV coordinates: u goes from 0 to 1 across the cylinder circumference
        // v goes from 0 at bottom to 1 at top
        float u = float(i) / float(n);  // This naturally gives u=1.0 for the wrap-around column

        // Add bottom vertex (y = -1, v = 0)
        vbo << Vtx{ {x, -1.0f, z}, {u, 0.0f} };

        // Add top vertex (y = 1, v = 1)
        vbo << Vtx{ {x, 1.0f, z}, {u, 1.0f} };
    }

    /* STEP 2: Generate side triangles with consistent counter-clockwise winding
     * Critical insight: when viewed from outside the cylinder, vertices should
     * appear in counter-clockwise order to create outward-facing normals */

    for (int i = 0; i < n; ++i) {  // Note: only n quads, not n+1
        // Calculate vertex indices for current quad
        int bottomLeft  = 2 * i;        // Bottom vertex of current column
        int topLeft     = 2 * i + 1;    // Top vertex of current column
        int bottomRight = 2 * (i + 1);  // Bottom vertex of next column
        int topRight    = 2 * (i + 1) + 1; // Top vertex of next column

        // First triangle: bottom-left → bottom-right → top-left
        // When viewed from outside, this creates counter-clockwise winding
        ibo << Tri{bottomLeft, bottomRight, topLeft};

        // Second triangle: top-left → bottom-right → top-right
        // This also creates counter-clockwise winding from outside view
        ibo << Tri{topLeft, bottomRight, topRight};
    }

    /* STEP 3: Create cap centers */
    int topCenterIdx = vbo.size();
    vbo << Vtx{ {0.0f, 1.0f, 0.0f}, {0.5f, 0.5f} };  // Top cap center

    int bottomCenterIdx = vbo.size();
    vbo << Vtx{ {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f} };  // Bottom cap center

    /* STEP 4: Create cap rim vertices with circular UV mapping */
    int topRimStart = vbo.size();
    for (int i = 0; i < n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);

        // Map to unit circle in texture space, centered at (0.5, 0.5)
        float u = 0.5f + 0.4f * x;  // Scale by 0.4 to leave border
        float v = 0.5f + 0.4f * z;

        vbo << Vtx{ {x, 1.0f, z}, {u, v} };  // Top rim vertex
    }

    int bottomRimStart = vbo.size();
    for (int i = 0; i < n; ++i) {
        float theta = 2.0f * M_PI * float(i) / float(n);
        float x = std::cos(theta);
        float z = std::sin(theta);

        // Map to unit circle in texture space
        float u = 0.5f + 0.4f * x;
        float v = 0.5f + 0.4f * z;

        vbo << Vtx{ {x, -1.0f, z}, {u, v} };  // Bottom rim vertex
    }

    /* STEP 5: Create cap triangles with proper winding order
     * Key insight: the caps need opposite winding because they face
     * opposite directions (top cap faces up, bottom cap faces down) */

    // Top cap triangles (viewed from above, counter-clockwise)
    for (int i = 0; i < n; ++i) {
        int current = topRimStart + i;
        int next = topRimStart + ((i + 1) % n);  // Wrap around for last triangle

        // Center → current → next creates outward-facing normal for top cap
        ibo << Tri{topCenterIdx, current, next};
    }

    // Bottom cap triangles (viewed from below, counter-clockwise)
    for (int i = 0; i < n; ++i) {
        int current = bottomRimStart + i;
        int next = bottomRimStart + ((i + 1) % n);  // Wrap around for last triangle

        // Center → next → current creates outward-facing normal for bottom cap
        ibo << Tri{bottomCenterIdx, next, current};
    }
}
