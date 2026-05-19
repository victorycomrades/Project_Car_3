from maix import app, camera, display, image
import socket
import time


WIDTH = 320
HEIGHT = 240
HOST = "0.0.0.0"
PORT = 7000

# Jetson can switch this at runtime by sending:
#   MODE,QR
#   MODE,BLOB,RED
#   MODE,BLOB,GREEN
#   MODE,BLOB,BLUE
#   MODE,BLOB,ALL
#   MODE,RING,RED
#   MODE,RING,GREEN
#   MODE,RING,BLUE
#   MODE,RING,ALL
#   MODE,LINE
#   MODE,IDLE
MODE = "QR"

COLOR_CONFIGS = {
    "RED": ([0, 90, 35, 90, -10, 90], image.COLOR_RED),
    "GREEN": ([0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    "BLUE": ([0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
}

RING_CONFIGS = {
    "RED": ([0, 90, 35, 90, -10, 90], image.COLOR_RED),
    "GREEN": ([0, 90, -120, -10, -20, 80], image.COLOR_GREEN),
    "BLUE": ([0, 90, -20, 80, -128, -20], image.COLOR_BLUE),
}

PIXELS_THRESHOLD = 350
AREA_THRESHOLD = 350
LINE_THRESHOLD = [20, 90, -15, 15, -15, 15]
LINE_ROI = [0, HEIGHT // 2, WIDTH, HEIGHT // 2]


server = None
client = None
rx_buffer = ""


def make_server():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((HOST, PORT))
    srv.listen(1)
    srv.settimeout(0.02)
    return srv


def close_client():
    global client
    if client:
        try:
            client.close()
        except Exception:
            pass
    client = None


def accept_client():
    global client
    if client:
        return
    try:
        conn, addr = server.accept()
    except Exception:
        return
    client = conn
    client.settimeout(0.001)
    send_line("HELLO,MAIXCAM_USB_TCP,%d" % PORT)
    print("CLIENT,%s,%s" % (addr[0], addr[1]))


def send_line(text):
    print(text)
    if not client:
        return
    try:
        client.send((text + "\n").encode("utf-8"))
    except Exception as err:
        print("WARN,CLIENT_SEND_FAIL,%s" % err)
        close_client()


def normalize_mode(parts):
    if len(parts) < 2:
        return "IDLE"
    if parts[1] == "BLOB":
        color = parts[2] if len(parts) >= 3 else "ALL"
        return "BLOB_" + color
    if parts[1] == "RING":
        color = parts[2] if len(parts) >= 3 else "ALL"
        return "RING_" + color
    return parts[1]


def handle_command(line):
    global MODE
    parts = [p.strip().upper() for p in line.split(",")]
    if not parts:
        return
    if parts[0] == "MODE":
        MODE = normalize_mode(parts)
        send_line("INFO,MODE,%s" % MODE)
    elif parts[0] == "PING":
        send_line("PONG,MAIXCAM")
    else:
        send_line("ERR,UNKNOWN_COMMAND,%s" % line)


def read_commands():
    global rx_buffer
    if not client:
        return
    try:
        data = client.recv(256)
    except Exception:
        return
    if not data:
        close_client()
        return
    rx_buffer += data.decode("utf-8")
    while "\n" in rx_buffer:
        line, rx_buffer = rx_buffer.split("\n", 1)
        line = line.strip()
        if line:
            handle_command(line)


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


def blob_report(color_name, blob):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    area = w * h
    dx = cx - WIDTH // 2
    dy = cy - HEIGHT // 2
    return "BLOB,%s,%d,%d,%d,%d,%d,%d,%d" % (color_name, cx, cy, w, h, area, dx, dy)


def draw_blob(img, color_name, blob, draw_color):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    cx = x + w // 2
    cy = y + h // 2
    img.draw_rect(x, y, w, h, draw_color, 2)
    img.draw_cross(cx, cy, draw_color, 10, 2)
    img.draw_string(x, max(0, y - 16), "%s (%d,%d)" % (color_name, cx, cy), draw_color)


def run_qr(img, frame_id):
    qrcodes = img.find_qrcodes()
    if not qrcodes:
        img.draw_string(0, 0, "QR: none", image.COLOR_RED)
        if frame_id % 20 == 0:
            send_line("NONE,QR")
        return
    qr = qrcodes[0]
    payload = qr.payload()
    corners = qr.corners()
    for i in range(4):
        x1, y1 = corners[i][0], corners[i][1]
        x2, y2 = corners[(i + 1) % 4][0], corners[(i + 1) % 4][1]
        img.draw_line(x1, y1, x2, y2, image.COLOR_RED, 2)
    img.draw_string(qr.x(), max(0, qr.y() - 16), payload, image.COLOR_RED)
    if frame_id % 5 == 0:
        send_line("QR,%s" % payload)


def run_blob(img, target, frame_id):
    colors = ("RED", "GREEN", "BLUE") if target == "ALL" else (target,)
    found = False
    for color_name in colors:
        if color_name not in COLOR_CONFIGS:
            continue
        threshold, draw_color = COLOR_CONFIGS[color_name]
        blob = find_largest_blob(img, threshold)
        if not blob:
            continue
        found = True
        draw_blob(img, color_name, blob, draw_color)
        if frame_id % 5 == 0:
            send_line(blob_report(color_name, blob))
    if not found:
        img.draw_string(0, 0, "BLOB %s: none" % target, image.COLOR_RED)
        if frame_id % 20 == 0:
            send_line("NONE,BLOB,%s" % target)


def ring_score(blob):
    x, y, w, h = blob[0], blob[1], blob[2], blob[3]
    area = max(1, w * h)
    density = int((blob[4] * 100) / area) if len(blob) > 4 else 0
    ratio = int((min(w, h) * 100) / max(1, max(w, h)))
    return density, ratio


def run_ring(img, target, frame_id):
    colors = ("RED", "GREEN", "BLUE") if target == "ALL" else (target,)
    found = False
    for color_name in colors:
        if color_name not in RING_CONFIGS:
            continue
        threshold, draw_color = RING_CONFIGS[color_name]
        blob = find_largest_blob(img, threshold)
        if not blob:
            continue
        found = True
        x, y, w, h = blob[0], blob[1], blob[2], blob[3]
        cx = x + w // 2
        cy = y + h // 2
        radius = max(w, h) // 2
        dx = cx - WIDTH // 2
        dy = cy - HEIGHT // 2
        density, ratio = ring_score(blob)
        score = density + ratio
        img.draw_rect(x, y, w, h, draw_color, 2)
        img.draw_circle(cx, cy, radius, draw_color, 2)
        img.draw_cross(cx, cy, draw_color, 10, 2)
        if frame_id % 5 == 0:
            send_line(
                "RING,%s,%d,%d,%d,%d,%d,%d,%d,%d,BLOB"
                % (color_name, cx, cy, dx, dy, radius, score, density, ratio)
            )
    if not found:
        img.draw_string(0, 0, "RING %s: none" % target, image.COLOR_RED)
        if frame_id % 20 == 0:
            send_line("NONE,RING,%s" % target)


def run_line(img, frame_id):
    img.draw_rect(LINE_ROI[0], LINE_ROI[1], LINE_ROI[2], LINE_ROI[3], image.COLOR_BLUE, 1)
    lines = img.get_regression([LINE_THRESHOLD], roi=LINE_ROI, area_threshold=100)
    if not lines:
        img.draw_string(0, 0, "LINE: none", image.COLOR_RED)
        if frame_id % 20 == 0:
            send_line("NONE,LINE")
        return
    line = lines[0]
    img.draw_line(line.x1(), line.y1(), line.x2(), line.y2(), image.COLOR_GREEN, 2)
    theta = line.theta()
    rho = line.rho()
    if theta > 90:
        theta_out = 270 - theta
    else:
        theta_out = 90 - theta
    dx = rho - WIDTH // 2
    img.draw_string(0, 0, "LINE dx=%d theta=%d" % (dx, theta_out), image.COLOR_GREEN)
    if frame_id % 5 == 0:
        send_line("LINE,%d,%d" % (dx, theta_out))


cam = camera.Camera(WIDTH, HEIGHT)
disp = display.Display()
server = make_server()
frame_id = 0
last_heartbeat = time.time()

print("MaixCam USB TCP vision server started on port %d." % PORT)

while not app.need_exit():
    accept_client()
    read_commands()

    img = cam.read()
    img.draw_string(0, HEIGHT - 16, "MODE:%s" % MODE, image.COLOR_WHITE)

    if MODE == "QR":
        run_qr(img, frame_id)
    elif MODE.startswith("BLOB_"):
        run_blob(img, MODE.split("_", 1)[1], frame_id)
    elif MODE.startswith("RING_"):
        run_ring(img, MODE.split("_", 1)[1], frame_id)
    elif MODE == "LINE":
        run_line(img, frame_id)
    elif MODE == "IDLE":
        if time.time() - last_heartbeat > 2:
            send_line("INFO,IDLE")
            last_heartbeat = time.time()
    else:
        img.draw_string(0, 0, "Unknown mode: " + MODE, image.COLOR_RED)
        if frame_id % 20 == 0:
            send_line("ERR,UNKNOWN_MODE,%s" % MODE)

    frame_id += 1
    disp.show(img)

close_client()
server.close()
