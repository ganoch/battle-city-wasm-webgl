#!/usr/bin/env python3
"""Make the sprite sheet's background fully transparent.

The artwork ships as an opaque RGBA PNG whose background is the exact
colour (0, 0, 1), with some pure black (0, 0, 0) mixed in.  Every pixel of
either colour gets alpha 0; every other colour is left untouched.

This uses only the standard library: the PNG is decoded and re-encoded by
hand with zlib, so no Pillow/numpy is required.

Usage:
    python3 tools/strip_bg.py [png ...]

With no arguments it processes sprites.png and src/sprites.png in place.
"""

import struct
import sys
import zlib

BACKGROUND = ((0, 0, 1), (0, 0, 0))
SIGNATURE = b"\x89PNG\r\n\x1a\n"


def _chunk(tag, payload):
    return (
        struct.pack(">I", len(payload))
        + tag
        + payload
        + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
    )


def read_rgba(path):
    """Return (width, height, bytearray RGBA) for an 8-bit non-interlaced PNG."""
    data = open(path, "rb").read()
    if data[:8] != SIGNATURE:
        raise ValueError(f"{path}: not a PNG file")

    pos = 8
    idat = b""
    w = h = None
    while pos < len(data):
        (length,) = struct.unpack(">I", data[pos:pos + 4])
        tag = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if tag == b"IHDR":
            w, h, depth, colour, _comp, _filt, interlace = struct.unpack(
                ">IIBBBBB", payload
            )
            if (depth, colour, interlace) != (8, 6, 0):
                raise ValueError(
                    f"{path}: expected 8-bit RGBA, non-interlaced "
                    f"(got depth={depth} colour={colour} interlace={interlace})"
                )
        elif tag == b"IDAT":
            idat += payload
        elif tag == b"IEND":
            break

    raw = zlib.decompress(idat)
    bpp = 4
    stride = w * bpp
    out = bytearray(h * stride)
    prev = bytearray(stride)
    p = 0
    for y in range(h):
        f = raw[p]
        p += 1
        line = bytearray(raw[p:p + stride])
        p += stride
        if f == 1:  # Sub
            for i in range(bpp, stride):
                line[i] = (line[i] + line[i - bpp]) & 255
        elif f == 2:  # Up
            for i in range(stride):
                line[i] = (line[i] + prev[i]) & 255
        elif f == 3:  # Average
            for i in range(stride):
                a = line[i - bpp] if i >= bpp else 0
                line[i] = (line[i] + ((a + prev[i]) >> 1)) & 255
        elif f == 4:  # Paeth
            for i in range(stride):
                a = line[i - bpp] if i >= bpp else 0
                b = prev[i]
                c = prev[i - bpp] if i >= bpp else 0
                pa = abs(b - c)
                pb = abs(a - c)
                pc = abs(a + b - 2 * c)
                pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
                line[i] = (line[i] + pr) & 255
        elif f != 0:
            raise ValueError(f"{path}: unsupported filter type {f}")
        out[y * stride:(y + 1) * stride] = line
        prev = line
    return w, h, out


def write_rgba(path, w, h, pixels):
    """Write an 8-bit RGBA PNG (each scanline filtered with 'None')."""
    stride = w * 4
    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter type: None
        raw += pixels[y * stride:(y + 1) * stride]
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)
    png = (
        SIGNATURE
        + _chunk(b"IHDR", ihdr)
        + _chunk(b"IDAT", zlib.compress(bytes(raw), 9))
        + _chunk(b"IEND", b"")
    )
    with open(path, "wb") as fh:
        fh.write(png)


def strip(path, colours=BACKGROUND):
    w, h, pixels = read_rgba(path)
    keys = {(r, g, b) for r, g, b in colours}
    changed = 0
    for i in range(0, len(pixels), 4):
        if (pixels[i], pixels[i + 1], pixels[i + 2]) in keys:
            if pixels[i + 3] != 0:
                pixels[i + 3] = 0
                changed += 1
    write_rgba(path, w, h, pixels)
    joined = ", ".join(f"rgb{c}" for c in colours)
    print(f"{path}: {changed} pixel(s) made transparent ({joined})")


def main(argv):
    paths = argv[1:] or ["sprites.png", "src/sprites.png"]
    for path in paths:
        strip(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
