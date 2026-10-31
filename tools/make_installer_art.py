# Renders the macOS installer side art (1x + 2x) and the app icon from the bndt artwork.
# Usage: python3 tools/make_installer_art.py   (writes into installer/mac/art and assets/)
import os
import numpy as np
from PIL import Image, ImageDraw, ImageFont, ImageFilter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BG = os.path.join(ROOT, "assets", "background.jpg")
FONT = os.path.join(ROOT, "assets", "GochiHand-Regular.ttf")
OUT = os.path.join(ROOT, "installer", "mac", "art")
os.makedirs(OUT, exist_ok=True)

PINK, PINK_LIGHT, PINK_DEEP, TEAL = (252, 108, 168), (249, 153, 205), (240, 28, 99), (30, 198, 166)
INK = (30, 25, 24)

# same letter geometry as PixelLogo in the plugin
LETTERS = [
    [(0, 0, 1, 6), (0, 1, 4, 1), (3, 1, 1, 5), (0, 5, 4, 1)],
    [(0, 1, 4, 1), (0, 1, 1, 5), (3, 1, 1, 5)],
    [(3, 0, 1, 6), (0, 1, 4, 1), (0, 1, 1, 5), (0, 5, 4, 1)],
    [(0, 0, 1, 6), (0, 5, 3, 1), (1, 1, 0.55, 1)],
]
WIDTHS, GAP = [4, 4, 4, 3], 1.15
TOTAL_UNITS = sum(WIDTHS) + GAP * 3


def draw_logo(img, x, y, u):
    d = ImageDraw.Draw(img)
    for col, dx, dy in [(TEAL, -0.2, -0.2), (PINK_DEEP, 0.16, 0.14), (PINK_LIGHT, 0.08, 0.07), (PINK, 0, 0)]:
        x0 = x
        for i, rects in enumerate(LETTERS):
            for rx, ry, rw, rh in rects:
                d.rectangle([x0 + (rx + dx) * u, y + (ry + dy) * u, x0 + (rx + dx + rw) * u - 1, y + (ry + dy + rh) * u - 1], fill=col)
            x0 += (WIDTHS[i] + GAP) * u


def halo_text(img, xy, text, font, fill):
    halo = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ImageDraw.Draw(halo).text(xy, text, font=font, fill=(234, 247, 232, 255))
    halo = halo.filter(ImageFilter.GaussianBlur(radius=max(1, font.size // 12)))
    img.alpha_composite(halo)
    ImageDraw.Draw(img).text(xy, text, font=font, fill=fill + (255,))


def side_art(scale):
    # bottom-left art for the installer window: the photo, fading out at the top and right
    w, h = 200 * scale, 250 * scale
    photo = Image.open(BG).convert("RGB")
    crop = photo.crop((0, photo.height - int(photo.height * 0.42), int(photo.width * 0.28), photo.height))
    crop = crop.resize((w, h), Image.LANCZOS).convert("RGBA")
    yy, xx = np.mgrid[0:h, 0:w].astype(np.float32)
    fade_top = np.clip(yy / (h * 0.45), 0, 1) ** 1.4
    fade_right = np.clip((w - xx) / (w * 0.35), 0, 1) ** 1.2
    alpha = (fade_top * fade_right * 255).astype(np.uint8)
    crop.putalpha(Image.fromarray(alpha))
    u = (w * 0.72) / TOTAL_UNITS
    draw_logo(crop, int(w * 0.09), int(h - u * 6 - 52 * scale), u)
    font = ImageFont.truetype(FONT, 17 * scale)
    halo_text(crop, (int(w * 0.10), int(h - 40 * scale)), "BY SWITCHKICKER", font, INK)
    return crop


def icon(size=1024):
    photo = Image.open(BG).convert("RGB")
    side = min(photo.size)
    sq = photo.crop(((photo.width - side) // 2, (photo.height - side) // 2, (photo.width + side) // 2, (photo.height + side) // 2))
    sq = sq.resize((size, size), Image.LANCZOS).filter(ImageFilter.GaussianBlur(radius=size // 90)).convert("RGBA")
    # macOS icon grid: 824 px rounded square inside a 1024 canvas
    inset = int(size * 100 / 1024)
    body = size - inset * 2
    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle([inset, inset, inset + body, inset + body], radius=int(body * 0.225), fill=255)
    canvas = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(shadow).rounded_rectangle([inset, inset + size // 80, inset + body, inset + body + size // 80],
                                             radius=int(body * 0.225), fill=(0, 0, 0, 110))
    canvas.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(size // 60)))
    canvas.paste(sq, (0, 0), mask)
    u = body * 0.78 / TOTAL_UNITS
    draw_logo(canvas, int(inset + (body - u * TOTAL_UNITS) / 2), int(inset + (body - u * 6) / 2), u)
    return canvas


side_art(1).save(os.path.join(OUT, "background.png"))
side_art(2).save(os.path.join(OUT, "background@2x.png"))
os.makedirs(os.path.join(ROOT, "assets", "branding"), exist_ok=True)
icon().save(os.path.join(ROOT, "assets", "branding", "icon.png"))   # not embedded: assets/ glob is non-recursive
print("art written")
