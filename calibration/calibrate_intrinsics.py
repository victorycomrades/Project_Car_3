#!/usr/bin/env python3
"""相机内参标定脚本。"""

import argparse
import glob
import os

import cv2
import numpy as np

from common import make_chessboard_object_points


def find_images(image_dir: str):
    patterns = ("*.jpg", "*.jpeg", "*.png", "*.bmp")
    images = []
    for pattern in patterns:
        images.extend(glob.glob(os.path.join(image_dir, pattern)))
    return sorted(images)


def main():
    parser = argparse.ArgumentParser(description="Calibrate MaixCam intrinsics from chessboard images.")
    parser.add_argument("--images", required=True, help="标定图片目录")
    parser.add_argument("--pattern-cols", type=int, default=9, help="棋盘格横向内角点数量")
    parser.add_argument("--pattern-rows", type=int, default=6, help="棋盘格纵向内角点数量")
    parser.add_argument("--square-mm", type=float, default=27.0, help="棋盘格每格实际边长，单位 mm")
    parser.add_argument("--output", default="calibration/output/camera_intrinsics.yaml")
    parser.add_argument("--corners-dir", default="calibration/output/intrinsics_corners")
    args = parser.parse_args()

    image_paths = find_images(args.images)
    if not image_paths:
        raise RuntimeError("No calibration images found in %s" % args.images)

    pattern_size = (args.pattern_cols, args.pattern_rows)
    object_template = make_chessboard_object_points(args.pattern_cols, args.pattern_rows, args.square_mm)
    object_points = []
    image_points = []
    image_size = None

    os.makedirs(args.corners_dir, exist_ok=True)
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    for image_path in image_paths:
        image = cv2.imread(image_path)
        if image is None:
            print("WARN: cannot read image:", image_path)
            continue
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        image_size = gray.shape[::-1]
        found, corners = cv2.findChessboardCorners(gray, pattern_size, None)
        if not found:
            print("WARN: chessboard not found:", image_path)
            continue

        corners = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
        object_points.append(object_template)
        image_points.append(corners)

        debug = image.copy()
        cv2.drawChessboardCorners(debug, pattern_size, corners, found)
        cv2.imwrite(os.path.join(args.corners_dir, os.path.basename(image_path)), debug)
        print("OK:", image_path)

    if len(object_points) < 10:
        raise RuntimeError("Need at least 10 valid chessboard images, got %d" % len(object_points))

    rms, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
        object_points, image_points, image_size, None, None)

    total_error = 0.0
    for i, objp in enumerate(object_points):
        projected, _ = cv2.projectPoints(objp, rvecs[i], tvecs[i], camera_matrix, dist_coeffs)
        total_error += cv2.norm(image_points[i], projected, cv2.NORM_L2) / len(projected)
    mean_error = total_error / len(object_points)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    fs = cv2.FileStorage(args.output, cv2.FILE_STORAGE_WRITE)
    fs.write("image_width", int(image_size[0]))
    fs.write("image_height", int(image_size[1]))
    fs.write("pattern_cols", int(args.pattern_cols))
    fs.write("pattern_rows", int(args.pattern_rows))
    fs.write("square_mm", float(args.square_mm))
    fs.write("rms", float(rms))
    fs.write("mean_reprojection_error", float(mean_error))
    fs.write("camera_matrix", camera_matrix)
    fs.write("dist_coeffs", dist_coeffs)
    fs.release()

    print("Saved:", args.output)
    print("RMS:", rms)
    print("Mean reprojection error:", mean_error)
    print("Camera matrix:\n", camera_matrix)
    print("Dist coeffs:\n", dist_coeffs)


if __name__ == "__main__":
    main()
