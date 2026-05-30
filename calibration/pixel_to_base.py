#!/usr/bin/env python3
"""把图像点和已知目标高度换算为机械臂基坐标点。"""

import argparse
import json

import cv2
import numpy as np

from common import load_intrinsics, load_transform_json


def load_handeye(path: str) -> np.ndarray:
    fs = cv2.FileStorage(path, cv2.FILE_STORAGE_READ)
    if not fs.isOpened():
        raise RuntimeError("Cannot open hand-eye file: %s" % path)
    matrix = fs.getNode("T_gripper_camera").mat()
    fs.release()
    if matrix is None:
        raise ValueError("T_gripper_camera missing in %s" % path)
    return matrix


def pixel_to_base(camera_matrix, dist_coeffs, t_gripper_camera, t_base_gripper, u, v, plane_z_mm):
    """用相机射线与基坐标系 z=height 平面求交。"""
    pixel = np.array([[[float(u), float(v)]]], dtype=np.float64)
    undistorted = cv2.undistortPoints(pixel, camera_matrix, dist_coeffs)
    x = undistorted[0, 0, 0]
    y = undistorted[0, 0, 1]
    ray_camera = np.array([x, y, 1.0], dtype=np.float64)
    ray_camera /= np.linalg.norm(ray_camera)

    t_base_camera = t_base_gripper.dot(t_gripper_camera)
    origin_base = t_base_camera[:3, 3]
    direction_base = t_base_camera[:3, :3].dot(ray_camera)

    if abs(direction_base[2]) < 1e-9:
        raise RuntimeError("Camera ray is parallel to target height plane")

    scale = (float(plane_z_mm) - origin_base[2]) / direction_base[2]
    if scale < 0:
        raise RuntimeError("Target plane is behind camera ray")
    return origin_base + scale * direction_base


def main():
    parser = argparse.ArgumentParser(description="Project image pixel to robot base frame.")
    parser.add_argument("--intrinsics", default="calibration/output/camera_intrinsics.yaml")
    parser.add_argument("--handeye", default="calibration/output/T_gripper_camera.yaml")
    parser.add_argument("--t-base-gripper", required=True, help="包含 T_base_gripper 的 JSON 文件")
    parser.add_argument("--cx", type=float, required=True)
    parser.add_argument("--cy", type=float, required=True)
    parser.add_argument("--height-mm", type=float, required=True)
    args = parser.parse_args()

    camera_matrix, dist_coeffs = load_intrinsics(args.intrinsics)
    t_gripper_camera = load_handeye(args.handeye)
    t_base_gripper = load_transform_json(args.t_base_gripper)
    point = pixel_to_base(
        camera_matrix,
        dist_coeffs,
        t_gripper_camera,
        t_base_gripper,
        args.cx,
        args.cy,
        args.height_mm,
    )

    print(json.dumps({
        "frame": "base",
        "x_mm": float(point[0]),
        "y_mm": float(point[1]),
        "z_mm": float(point[2]),
    }, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
