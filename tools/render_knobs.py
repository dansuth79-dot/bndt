# Renders the worn '65 blackface-style skirted knob as three layers:
#   knob_shadow.png  - static drop + contact shadow (never rotates)
#   knob_body_N.png  - skirt, numbers, fluted grip, cap (rotates with value)
#   knob_light.png   - static lighting, rim light and cap specular (never rotates)
# Numbers run 1..10 anticlockwise so the setting is read at 12 o'clock.
import numpy as np, cv2, math, sys
from PIL import Image, ImageDraw, ImageFont

OUT = sys.argv[1] if len(sys.argv) > 1 else "assets"
S = 1024            # supersampled canvas
FINAL = 256         # exported size (2x of a 128 px box)
C = S / 2
R_SKIRT = S * 0.415
R_SKIRT_IN = S * 0.300   # where the raised grip begins
R_GRIP = S * 0.285
R_CAP = S * 0.205

yy, xx = np.mgrid[0:S, 0:S].astype(np.float32)
dx, dy = xx - C, yy - C
rr = np.sqrt(dx * dx + dy * dy)
ang = np.arctan2(dy, dx)

def aa_disc(r, soft=1.5):
    return np.clip((r - rr) / soft + 0.5, 0, 1)

def to_img(rgba):
    a = np.clip(rgba, 0, 1)
    small = cv2.resize(a, (FINAL, FINAL), interpolation=cv2.INTER_AREA)
    return Image.fromarray((small * 255).astype(np.uint8), "RGBA")

def noise(seed, scale, octaves=3):
    rng = np.random.default_rng(seed)
    acc = np.zeros((S, S), np.float32); amp = 1.0; tot = 0
    for o in range(octaves):
        n = rng.normal(0, 1, (S // scale + 2, S // scale + 2)).astype(np.float32)
        n = cv2.resize(n, (S, S), interpolation=cv2.INTER_CUBIC)
        acc += n * amp; tot += amp; amp *= 0.5; scale = max(2, scale // 2)
    return acc / tot

# ---------- shadow (static) ----------
def shadow():
    a = np.zeros((S, S), np.float32)
    sh = aa_disc(R_SKIRT * 1.02, 3)
    M = np.float32([[1, 0, S * 0.035], [0, 1, S * 0.055]])
    sh = cv2.warpAffine(sh, M, (S, S))
    a += cv2.GaussianBlur(sh, (0, 0), S * 0.035) * 0.62
    contact = cv2.GaussianBlur(aa_disc(R_SKIRT * 1.0, 3), (0, 0), S * 0.008)
    a = np.maximum(a, contact * 0.55)
    rgba = np.zeros((S, S, 4), np.float32)
    rgba[..., 3] = a
    return rgba

# ---------- body (rotating) ----------
def body(seed):
    rgba = np.zeros((S, S, 4), np.float32)
    skirt = aa_disc(R_SKIRT)
    base = np.array([0.055, 0.052, 0.050], np.float32)
    col = np.ones((S, S, 3), np.float32) * base
    # worn plastic: mottled sheen, rubbed patches
    mott = noise(seed, 96) * 0.018 + noise(seed + 1, 24) * 0.010
    col += mott[..., None]
    rub = np.clip(noise(seed + 2, 160) * 1.6 - 0.9, 0, 1) * 0.05
    col += rub[..., None] * np.array([1.0, 0.95, 0.9])
    # skirt slope: slightly lighter flat ring with a darker step into the grip
    ring = np.clip((rr - R_SKIRT_IN) / (R_SKIRT - R_SKIRT_IN), 0, 1)
    col += (0.018 * (1 - ring))[..., None]
    rgba[..., :3] = col
    rgba[..., 3] = skirt

    # fine scratches (circular-ish from years of fingers)
    rng = np.random.default_rng(seed + 3)
    scr = np.zeros((S, S), np.float32)
    for _ in range(70):
        r0 = rng.uniform(R_SKIRT_IN * 0.95, R_SKIRT * 0.98)
        a0 = rng.uniform(-math.pi, math.pi); span = rng.uniform(0.05, 0.5)
        pts = [(C + r0 * math.cos(a0 + t * span), C + r0 * math.sin(a0 + t * span)) for t in np.linspace(0, 1, 12)]
        cv2.polylines(scr, [np.int32(pts)], False, rng.uniform(0.3, 1.0), 1, cv2.LINE_AA)
    for _ in range(40):
        x0, y0 = rng.uniform(C - R_SKIRT, C + R_SKIRT, 2)
        L = rng.uniform(10, 60); th = rng.uniform(0, math.pi)
        cv2.line(scr, (int(x0), int(y0)), (int(x0 + L * math.cos(th)), int(y0 + L * math.sin(th))), rng.uniform(0.2, 0.8), 1, cv2.LINE_AA)
    scr *= skirt
    rgba[..., :3] += (scr * 0.09)[..., None]

    # numbers 1..10, anticlockwise, 300 degree sweep, read at 12 o'clock
    txt = Image.new("L", (S, S), 0)
    d = ImageDraw.Draw(txt)
    font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed-Bold.ttf", int(S * 0.082))
    r_num = (R_SKIRT + R_SKIRT_IN) / 2 + S * 0.004
    for i in range(10):
        a_deg = -90 - i * (300 / 9)            # 1 at top, then anticlockwise
        a = math.radians(a_deg)
        tile = Image.new("L", (int(S * 0.16), int(S * 0.16)), 0)
        td = ImageDraw.Draw(tile)
        s = str(i + 1)
        bb = td.textbbox((0, 0), s, font=font)
        w, h = bb[2] - bb[0], bb[3] - bb[1]
        td.text(((tile.width - w) / 2 - bb[0], (tile.height - h) / 2 - bb[1]), s, fill=255, font=font)
        rot = tile.rotate(-(a_deg + 90), resample=Image.BICUBIC)
        px = C + r_num * math.cos(a) - rot.width / 2
        py = C + r_num * math.sin(a) - rot.height / 2
        txt.paste(rot, (int(px), int(py)), rot)
    t = np.asarray(txt, np.float32) / 255
    # wear on the stamping: erosion noise, a couple of numbers badly rubbed
    wear = np.clip(noise(seed + 4, 14) * 1.2 + 1.05, 0, 1)
    heavy = np.clip(noise(seed + 5, 220) * 1.6 + 0.85, 0.35, 1)
    t *= wear * heavy
    ink = np.array([0.90, 0.88, 0.80], np.float32)   # aged white
    rgba[..., :3] = rgba[..., :3] * (1 - t[..., None] * 0.92) + ink * t[..., None] * 0.92

    # fluted grip (raised cylinder): ridges around the edge
    grip = aa_disc(R_GRIP)
    nflutes = 36
    flute = 0.5 + 0.5 * np.cos(ang * nflutes)
    edge = np.clip((rr - (R_GRIP - S * 0.045)) / (S * 0.045), 0, 1)
    gcol = np.ones((S, S, 3), np.float32) * np.array([0.045, 0.043, 0.042])
    gcol += ((flute * 0.05 - 0.02) * edge)[..., None]
    gcol += (noise(seed + 6, 40) * 0.012)[..., None]
    # grime packed into the flutes
    grime = (1 - flute) * edge * np.clip(noise(seed + 7, 60) + 0.5, 0, 1) * 0.04
    gcol += grime[..., None] * np.array([0.9, 0.8, 0.6])
    rgba[..., :3] = rgba[..., :3] * (1 - grip[..., None]) + gcol * grip[..., None]

    # silver cap: brushed concentric aluminium, dulled and spotted
    cap = aa_disc(R_CAP)
    brushed = noise(seed + 8, 4, 2)
    # concentric brushing: smear noise along angle by sampling polar
    rings = np.sin(rr * 1.7 + noise(seed + 9, 64) * 2) * 0.5 + 0.5
    ccol = 0.50 + rings * 0.035 + brushed * 0.015
    dull = np.clip(noise(seed + 10, 90) * 1.5, -1, 1) * 0.05
    ccol = ccol - np.abs(dull)
    spots = np.clip(noise(seed + 11, 12) * 2 - 2.1, 0, 1) * 0.10
    ccol = ccol - spots
    ccolrgb = np.stack([ccol * 0.98, ccol * 0.98, ccol * 0.96], -1)
    # thin dark groove around the cap
    groove = np.exp(-((rr - R_CAP) / (S * 0.006)) ** 2) * 0.8
    rgba[..., :3] = rgba[..., :3] * (1 - cap[..., None]) + ccolrgb * cap[..., None]
    rgba[..., :3] *= (1 - groove)[..., None]
    return rgba

# ---------- lighting (static) ----------
def light():
    rgba = np.zeros((S, S, 4), np.float32)
    lx, ly = -0.6, -0.8                   # light from top-left
    skirt = aa_disc(R_SKIRT); grip = aa_disc(R_GRIP); cap = aa_disc(R_CAP)
    nx, ny = dx / (rr + 1e-6), dy / (rr + 1e-6)
    facing = nx * lx + ny * ly            # +1 toward the light
    # skirt: soft broad sheen on the lit side, darker on the far side
    sk = skirt * (1 - grip)
    lit = np.clip(facing, 0, 1) ** 2 * 0.10 * sk
    dark = np.clip(-facing, 0, 1) * 0.28 * sk
    # rim highlight on the skirt's outer bevel
    rim = np.exp(-((rr - R_SKIRT * 0.985) / (S * 0.006)) ** 2) * np.clip(facing, 0, 1) * 0.45
    # grip: cylinder side catches light along the lit edge, shadow on far edge
    gedge = np.exp(-((rr - R_GRIP * 0.97) / (S * 0.012)) ** 2)
    glit = gedge * np.clip(facing, 0, 1) * 0.30
    gdark = grip * (1 - cap) * np.clip(-facing, 0, 1) * 0.30
    # grip casts a shadow onto the skirt (away from light)
    gs = aa_disc(R_GRIP, 2)
    gs = cv2.warpAffine(gs, np.float32([[1, 0, S * 0.022], [0, 1, S * 0.03]]), (S, S))
    gs = cv2.GaussianBlur(gs, (0, 0), S * 0.012) * sk * (1 - grip) * 0.55
    # cap: broad specular streak + small hot spot
    capspec = cap * np.exp(-(((dx + S * 0.05) * 0.8 + (dy + S * 0.07)) ** 2) / (2 * (S * 0.05) ** 2)) * 0.35
    hot = cap * np.exp(-(((dx + S * 0.06) ** 2 + (dy + S * 0.075) ** 2)) / (2 * (S * 0.018) ** 2)) * 0.55
    phiL = math.atan2(ly, lx)
    aniso = np.abs(np.cos(ang - phiL)) ** 10
    capspec = capspec * 0.5 + cap * aniso * np.clip(rr / R_CAP, 0.15, 1) * 0.38
    capdark = cap * (np.abs(np.sin(ang - phiL)) ** 6) * 0.30

    white = lit + rim + glit + capspec + hot
    black = dark + gdark + gs + capdark
    # premultiplied-style composite: white over black
    a = np.clip(white + black, 0, 1)
    colr = np.where(a > 1e-4, white / (a + 1e-6), 0)
    rgba[..., :3] = np.stack([colr, colr * 0.99, colr * 0.96], -1)
    rgba[..., 3] = a
    return rgba

to_img(shadow()).save(f"{OUT}/knob_shadow.png")
for i, seed in enumerate([11, 23, 37]):
    to_img(body(seed)).save(f"{OUT}/knob_body_{i}.png")
to_img(light()).save(f"{OUT}/knob_light.png")
print("ok")
