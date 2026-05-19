from maix import app, camera, display, image


WIDTH = 320
HEIGHT = 240

# Change this value before running in MaixVision:
#   "QR"          detect QR code
#   "BLOB_RED"    detect only red object
#   "BLOB_GREEN"  detect only green object
#   "BLOB_BLUE"   detect only blue object
#   "BLOB_ALL"    detect red, green, and blue objects
#   "LINE"        detect line / gray road area
MODE = "BLOB_BLUE"

# LAB thresholds. These are broad starting values. Tune them with MaixCam's
# built-in Find Blobs app for your real objects and lighting.
COLOR_CONFIGS = {
    "RED": ([0, 90, 35, 90, -10, 90], image.COLOR_RED),
    "GREEN": ([0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    "BLUE": ([0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
}

PIXELS_THRESHOLD = 350
AREA_THRESHOLD = 350

# Starting threshold for a gray / dark road or line.
LINE_THRESHOLD = [20, 90, -15, 15, -15, 15]
LINE_ROI = [0, HEIGHT // 2, WIDTH, HEIGHT // 2]


def draw_image_center(img):
    img.draw_cross(WIDTH // 2, HEIGHT // 2, image.COLOR_WHITE, 6, 1)


def find_largest_blob(img, threshold):
    blobs = img.find_blobs(
        [threshold],
        pixels_threshold=PIXELS_THRESHOLD,
        area_threshold=AREA_THRESHOLD,
        merge=True,
    )
    if not blobs:
        return None
    return max(blobs, key=lambda b: b[2] * b[3])


def draw_and_report_blob(img, color_name, blob, draw_color):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    area = w * h
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2

    # The box is for visual debugging. cx/cy/dx/dy are the useful control data.
    img.draw_rect(x, y, w, h, draw_color, 2)
    img.draw_cross(cx, cy, draw_color, 10, 2)
    img.draw_string(x, max(0, y - 16), "%s (%d,%d)" % (color_name, cx, cy), draw_color)

    return "BLOB,%s,%d,%d,%d,%d,%d,%d,%d" % (
        color_name,
        cx,
        cy,
        w,
        h,
        area,
        dx,
        dy,
    )


def run_qr(img, frame_id):
    qrcodes = img.find_qrcodes()
    if not qrcodes:
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)
        if frame_id % 30 == 0:
            print("NONE,QR")
        return

    for qr in qrcodes:
        payload = qr.payload()
        corners = qr.corners()

        for i in range(4):
            x1, y1 = corners[i][0], corners[i][1]
            x2, y2 = corners[(i + 1) % 4][0], corners[(i + 1) % 4][1]
            img.draw_line(x1, y1, x2, y2, image.COLOR_RED, 2)

        img.draw_string(qr.x(), max(0, qr.y() - 16), payload, image.COLOR_RED)

        if frame_id % 10 == 0:
            print("QR,%s" % payload)


def run_one_blob(img, color_name, frame_id):
    threshold, draw_color = COLOR_CONFIGS[color_name]
    blob = find_largest_blob(img, threshold)

    if blob is None:
        img.draw_string(0, 0, "BLOB %s: none" % color_name, image.COLOR_RED)
        if frame_id % 20 == 0:
            print("NONE,BLOB,%s" % color_name)
        return

    report = draw_and_report_blob(img, color_name, blob, draw_color)
    if frame_id % 10 == 0:
        print(report)


def run_all_blobs(img, frame_id):
    reports = []

    for color_name in ("RED", "GREEN", "BLUE"):
        threshold, draw_color = COLOR_CONFIGS[color_name]
        blob = find_largest_blob(img, threshold)
        if blob is None:
            continue
        reports.append(draw_and_report_blob(img, color_name, blob, draw_color))

    if frame_id % 10 == 0:
        if reports:
            for report in reports:
                print(report)
        else:
            print("NONE,BLOB,ALL")


def run_line(img, frame_id):
    img.draw_rect(LINE_ROI[0], LINE_ROI[1], LINE_ROI[2], LINE_ROI[3], image.COLOR_BLUE, 1)
    lines = img.get_regression([LINE_THRESHOLD], roi=LINE_ROI, area_threshold=100)

    if not lines:
        img.draw_string(0, 0, "LINE: none", image.COLOR_RED)
        if frame_id % 20 == 0:
            print("NONE,LINE")
        return

    line = lines[0]
    img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.COLOR_GREEN, 2)

    theta = line.theta()
    rho = line.rho()
    if theta > 90:
        theta_out = 270 - theta
    else:
        theta_out = 90 - theta

    img.draw_string(0, 0, "LINE theta=%d rho=%d" % (theta_out, rho), image.COLOR_GREEN)

    if frame_id % 10 == 0:
        print("LINE,%d,%d" % (theta_out, rho))


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
frame_id = 0

print("Vision mode test started.")
print("MODE =", MODE)

while not app.need_exit():
    img = cam.read()
    draw_image_center(img)

    if MODE == "QR":
        run_qr(img, frame_id)
    elif MODE == "BLOB_RED":
        run_one_blob(img, "RED", frame_id)
    elif MODE == "BLOB_GREEN":
        run_one_blob(img, "GREEN", frame_id)
    elif MODE == "BLOB_BLUE":
        run_one_blob(img, "BLUE", frame_id)
    elif MODE == "BLOB_ALL":
        run_all_blobs(img, frame_id)
    elif MODE == "LINE":
        run_line(img, frame_id)
    else:
        img.draw_string(0, 0, "Unknown MODE: " + MODE, image.COLOR_RED)
        if frame_id % 20 == 0:
            print("ERR,UNKNOWN_MODE,%s" % MODE)

    frame_id += 1
    disp.show(img)
