#!/usr/bin/env python3
"""Procedurally generate a wood-crate diffuse map and matching specular map.

Usage: python3 scripts/generate_crate_textures.py
Writes textures/crate_diffuse.png and textures/crate_specular.png.

No external assets or network access — pure procedural generation, so the
diffuse/specular contrast (wood = dull, metal braces = shiny) is guaranteed
and reproducible (fixed random seed).
"""

import os
import random

from PIL import Image, ImageDraw

SIZE = 512
BORDER = 28          # metal frame thickness
BRACE_WIDTH = 22      # diagonal cross-brace thickness
PLANK_COUNT = 6

WOOD_BASE = (133, 94, 55)
WOOD_DARK = (98, 67, 38)
METAL_BASE = (150, 150, 155)
METAL_DARK = (90, 90, 96)

random.seed(42)


def is_metal(x, y):
    """True where the metal frame/cross-braces cover this pixel."""
    if x < BORDER or x >= SIZE - BORDER or y < BORDER or y >= SIZE - BORDER:
        return True

    # Two diagonals (X brace) through the center, clipped to the panel interior.
    for sign in (1, -1):
        dist = abs((x - y * sign) - (SIZE / 2 - SIZE / 2 * sign))
        if dist < BRACE_WIDTH / 2:
            return True
    return False


def wood_color(x, y):
    plank = int(x / (SIZE / PLANK_COUNT))
    plank_shade = 1.0 - (plank % 2) * 0.08

    grain = 0.05 * (
        (y * 0.13 + plank * 3.7) % 1.0 - 0.5
    )
    noise = random.uniform(-0.03, 0.03)

    plank_edge = x % (SIZE / PLANK_COUNT)
    seam = plank_edge < 2 or plank_edge > (SIZE / PLANK_COUNT) - 2

    base = WOOD_DARK if seam else WOOD_BASE
    factor = plank_shade + grain + noise
    return tuple(max(0, min(255, int(c * factor))) for c in base)


def metal_color(x, y):
    edge_x = min(x % SIZE, SIZE - 1 - (x % SIZE))
    edge_y = min(y % SIZE, SIZE - 1 - (y % SIZE))
    near_edge = min(edge_x, edge_y) < 3
    noise = random.uniform(-0.05, 0.05)
    base = METAL_DARK if near_edge else METAL_BASE
    factor = 1.0 + noise
    return tuple(max(0, min(255, int(c * factor))) for c in base)


def generate():
    diffuse = Image.new("RGB", (SIZE, SIZE))
    specular = Image.new("RGB", (SIZE, SIZE))
    dpix = diffuse.load()
    spix = specular.load()

    for y in range(SIZE):
        for x in range(SIZE):
            if is_metal(x, y):
                dpix[x, y] = metal_color(x, y)
                spix[x, y] = (235, 235, 235)  # shiny metal
            else:
                dpix[x, y] = wood_color(x, y)
                spix[x, y] = (12, 12, 12)  # dull wood

    return diffuse, specular


def main():
    out_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "textures")
    os.makedirs(out_dir, exist_ok=True)

    diffuse, specular = generate()
    diffuse.save(os.path.join(out_dir, "crate_diffuse.png"))
    specular.save(os.path.join(out_dir, "crate_specular.png"))
    print(f"Wrote {out_dir}/crate_diffuse.png and crate_specular.png ({SIZE}x{SIZE})")


if __name__ == "__main__":
    main()
