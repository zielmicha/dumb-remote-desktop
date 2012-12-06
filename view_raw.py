import pygame
import sys
import os

def main():
    w, h = map(int, sys.argv[1:])

    data = map(ord, sys.stdin.read())
    surf = pygame.Surface((w, h))

    for x in xrange(w):
        for y in xrange(h):
            base = 3 * (w * y + x)
            surf.set_at((x, y), (data[base], data[base + 1], data[base + 2]))
    pygame.image.save(surf, 'foo.png')
    os.system('okular foo.png')

if __name__ == '__main__':
    main()
