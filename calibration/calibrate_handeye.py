#!/usr/bin/env python3
"""眼在手上手眼标定脚本。"""

import argparse
import os

import cv2
import numpy as np

from common import (
    load_handeye_samples,
    load_intrinsics,
    make_chessboard_object_points,
    transform_from_rvec_tvec,
)


def main():
    parser = argparse.ArgumentParser(description="Calibrate eye-in-hand extrinsics.")
    parser.add_argument("--samples", required=True, help="手眼样本 JSON")
    parser.add_argument("--intrinsics", default="calibration/output/camera_intrinsics.yaml")
    parser.add_argument("--pattern-cols", type=int, default=9)
    parser.add_argument("--pattern-rows", type=int, default=6)
    parser.add_argument("--square-mm", type=float, default=27.0)
    parser.add_argument("--output", default="calibration/output/T_gripper_camera.yaml")
    parser.add_argument("--method", default="TSAI", choices=["TSAI", "PARK", "HORAUD", "ANDREFF", "DANIILIDIS"])
    args = parser.parse_args()

    methods = {
        "TSAI": cv2.CALIB_HAND_EYE_TSAI,
        "PARK": cv2.CALIB_HAND_EYE_PARK,
        "HORAUD": cv2.CALIB_HAND_EYE_HORAUD,
        "ANDREFF": cv2.CALIB_HAND_EYE_ANDREFF,
        "DANIILIDIS": cv2.CALIB_HAND_EYE_DANIILIDIS,
    }

    camera_matrix, dist_coeffs = load_intrinsics(args.intrinsics)
    object_points = make_chessboard_object_points(args.pattern_cols, args.pattern_rows, args.square_mm)
    pattern_size = (args.pattern_cols, args.pattern_rows)
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

    samples = load_handeye_samples(args.samples)
    r_gripper2base = []
    t_gripper2base = []
    r_target2cam = []
    t_target2cam = []
    used = 0

    for sample in samples:
        image = cv2.imread(sample["image"])
        if image is None:
            print("WARN: cannot read image:", sample["image"])
            continue
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        found, corners = cv2.findChessboardCorners(gray, pattern_size, None)
        if not found:
            print("WARN: chessboard not found:", sample["image"])
            continue
        corners = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
        ok, rvec, tvec = cv2.solvePnP(object_points, corners, camera_matrix, dist_coeffs)
        if not ok:
            print("WARN: solvePnP failed:", sample["image"])
            continue

        t_base_gripper = sample["T_base_gripper"]
        t_camera_target = transform_from_rvec_tvec(rvec, tvec)

        r_gripper2base.append(t_base_gripper[:3, :3])
        t_gripper2base.append(t_base_gripper[:3, 3])
        r_target2cam.append(t_camera_target[:3, :3])
        t_target2cam.append(t_camera_target[:3, 3])
        used += 1
        print("OK:", sample["image"])

    if used < 8:
        raise RuntimeError("Need at least 8 valid hand-eye samples, got %d" % used)

    r_cam2gripper, t_cam2gripper = cv2.calibrateHandEye(
        r_gripper2base,
        t_gripper2base,
        r_target2cam,
        t_target2cam,
        method=methods[args.method],
    )

    t_gripper_camera = np.eye(4, dtype=np.float64)
    t_gripper_camera[:3, :3] = r_cam2gripper
    t_gripper_camera[:3, 3] = np.asarray(t_cam2gripper).reshape(3)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    fs = cv2.FileStorage(args.output, cv2.FILE_STORAGE_WRITE)
    fs.write("T_gripper_camera", t_gripper_camera)
    fs.write("method", args.method)
    fs.write("used_samples", int(used))
    fs.release()

    print("Saved:", args.output)
    print("T_gripper_camera:\n", t_gripper_camera)


if __name__ == "__main__":
    main()
