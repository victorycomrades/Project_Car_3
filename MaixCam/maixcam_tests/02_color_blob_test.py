from maix import app, camera, display, image


WIDTH = 320
HEIGHT = 240

# LAB thresholds. Tune these values on the real field if detection is unstable.
COLOR_CONFIGS = [
    ("RED", [0, 80, 40, 80, 10, 80], image.COLOR_RED),
    ("GREEN", [0, 80, -120, -10, 0, 30], image.COLOR_GREEN),
    ("BLUE", [0, 80, 30, 100, -120, -60], image.COLOR_BLUE),
]

PIXELS_THRESHOLD = 500
AREA_THRESHOLD = 500


def find_largest_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],
        pixels_threshold=PIXELS_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
    )
    if not blobs:
        return None
    return max(blobs, key=lambda b: b[2] * b[3])


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Color blob test started. Show red, green, and blue objects to MaixCam.")

while not app.need_exit():
    img = cam.read()
    report_parts = []

    for name, threshold, draw_color in COLOR_CONFIGS:
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue

        x, y, w, h = blob[0], blob[1], blob[2], blob[3]
        cx = x + w // 2
        cy = y + h // 2

        img.draw_rect(x, y, w, h, draw_color, 2)
        img.draw_cross(cx, cy, draw_color, 8, 2)
        img.draw_string(x, max(0, y - 16), name, draw_color)
        report_parts.append("%s cx=%d cy=%d w=%d h=%d" % (name, cx, cy, w, h))

    if frame_id % 15 == 0:
        if report_parts:
            print("BLOB:", " | ".join(report_parts))
        else:
            print("BLOB: none")

    frame_id += 1
    disp.show(img)
