from collections import Counter

src = open('tools/inspect_png.py').read().split('amin = 255')[0]
ns = {}
exec(src, ns)
px = ns['px']

strips = {
    'player1': (0, 0), 'player2': (0, 128),
    'enemy1': (128, 0), 'enemy2': (128, 16), 'enemy3': (128, 32),
}

for name, (ox, oy) in strips.items():
    counts = Counter()
    for y in range(oy, oy + 16):
        for x in range(ox, ox + 128):
            counts[px(x, y)[:3]] += 1
    bg = counts.pop((0, 0, 1), 0)
    black = counts.pop((0, 0, 0), 0)
    dark = {c: n for c, n in counts.items() if max(c) < 64}
    print(f'{name}: bg(0,0,1)={bg}  pure-black(0,0,0)={black}  other-dark={dark}')
