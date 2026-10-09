import zlib
import struct
from collections import Counter

data = open('sprites.png', 'rb').read()
assert data[:8] == b'\x89PNG\r\n\x1a\n'
pos = 8
idat = b''
w = h = bd = ct = None
while pos < len(data):
    ln = struct.unpack('>I', data[pos:pos + 4])[0]
    typ = data[pos + 4:pos + 8]
    chunk = data[pos + 8:pos + 8 + ln]
    pos += 12 + ln
    if typ == b'IHDR':
        w, h, bd, ct, comp, filt, inter = struct.unpack('>IIBBBBB', chunk)
        print('IHDR', w, h, 'bitdepth', bd, 'colortype', ct, 'interlace', inter)
    elif typ == b'IDAT':
        idat += chunk
    elif typ == b'IEND':
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
    if f == 1:
        for i in range(bpp, stride):
            line[i] = (line[i] + line[i - bpp]) & 255
    elif f == 2:
        for i in range(stride):
            line[i] = (line[i] + prev[i]) & 255
    elif f == 3:
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            line[i] = (line[i] + ((a + prev[i]) >> 1)) & 255
    elif f == 4:
        for i in range(stride):
            a = line[i - bpp] if i >= bpp else 0
            b = prev[i]
            c = prev[i - bpp] if i >= bpp else 0
            pa = abs(b - c)
            pb = abs(a - c)
            pc = abs(a + b - 2 * c)
            pr = a if (pa <= pb and pa <= pc) else (b if pb <= pc else c)
            line[i] = (line[i] + pr) & 255
    out[y * stride:(y + 1) * stride] = line
    prev = line


def px(x, y):
    o = y * stride + x * 4
    return tuple(out[o:o + 4])


amin = 255
amax = 0
for i in range(3, len(out), 4):
    v = out[i]
    if v < amin:
        amin = v
    if v > amax:
        amax = v
print('alpha extrema', amin, amax)

c = Counter()
for y in range(0, 16):
    for x in range(0, 128):
        c[px(x, y)] += 1
print('player1 strip top colors:')
for col, n in c.most_common(10):
    print(' ', col, n)

print('corners', px(0, 0), px(127, 0), px(0, 255), px(399, 255))

# Count how many fully-opaque near-black pixels exist across the sheet.
blacks = sum(1 for i in range(0, len(out), 4)
             if out[i] < 8 and out[i + 1] < 8 and out[i + 2] < 8)
print('near-black pixels total:', blacks, 'of', w * h)
