#!/usr/bin/env python3
"""手眼标定脚本共用函数。"""

import json
import os
from typing import Iterable, List

import numpy as np


def make_chessboard_object_points(pattern_cols: int, pattern_rows: int, square_mm: float) -> np.ndarray:
    """生成棋盘格内角点三维坐标，棋盘格平面为 z=0，单位 mm。"""
    objp = np.zeros((pattern_cols * pattern_rows, 3), np.float32)
    objp[:, :2] = np.mgrid[0:pattern_cols, 0:pattern_rows].T.reshape(-1, 2)
    objp *= float(square_mm)
    return objp


def matrix_from_flat(values: Iterable[float]) -> np.ndarray:
    """把 16 个数转为 4x4 齐次变换矩阵。"""
    data = [float(v) for v in values]
    if len(data) != 16:
        raise ValueError("Transform matrix needs 16 numbers, got %d" % len(data))
    return np.array(data, dtype=np.float64).reshape(4, 4)


def load_transform_json(path: str, key: str = "T_base_gripper") -> np.ndarray:
    """从 JSON 文件读取 4x4 变换矩阵。"""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if isinstance(data, dict):
        data = data.get(key, data.get("matrix"))
    matrix = np.array(data, dtype=np.float64)
    if matrix.shape != (4, 4):
        raise ValueError("%s must contain a 4x4 matrix" % path)
    return matrix


def load_handeye_samples(path: str) -> List[dict]:
    """读取手眼样本 JSON，格式为 {'samples': [{'image': ..., 'T_base_gripper': ...}]}。"""
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    samples = data.get("samples", data if isinstance(data, list) else None)
    if not samples:
        raise ValueError("No hand-eye samples found in %s" % path)
    base_dir = os.path.dirname(os.path.abspath(path))
    normalized = []
    for sample in samples:
        image_path = sample["image"]
        if not os.path.isabs(image_path):
            image_path = os.path.join(base_dir, image_path)
        matrix = np.array(sample["T_base_gripper"], dtype=np.float64)
        if matrix.shape != (4, 4):
            raise ValueError("Sample %s has invalid T_base_gripper" % image_path)
        normalized.append({"image": image_path, "T_base_gripper": matrix})
    return normalized


def transform_from_rvec_tvec(rvec: np.ndarray, tvec: np.ndarray) -> np.ndarray:
    """把 OpenCV rvec/tvec 转为 4x4 齐次矩阵。"""
    import cv2

    rotation, _ = cv2.Rodrigues(rvec)
    matrix = np.eye(4, dtype=np.float64)
    matrix[:3, :3] = rotation
    matrix[:3, 3] = np.asarray(tvec, dtype=np.float64).reshape(3)
    return matrix


def invert_transform(matrix: np.ndarray) -> np.ndarray:
    """求刚体变换逆矩阵。"""
    rotation = matrix[:3, :3]
    translation = matrix[:3, 3]
    inv = np.eye(4, dtype=np.float64)
    inv[:3, :3] = rotation.T
    inv[:3, 3] = -rotation.T.dot(translation)
    return inv


def load_intrinsics(path: str):
    """读取 OpenCV FileStorage 保存的相机内参。"""
    import cv2

    fs = cv2.FileStorage(path, cv2.FILE_STORAGE_READ)
    if not fs.isOpened():
        raise RuntimeError("Cannot open intrinsics file: %s" % path)
    camera_matrix = fs.getNode("camera_matrix").mat()
    dist_coeffs = fs.getNode("dist_coeffs").mat()
    fs.release()
    if camera_matrix is None or dist_coeffs is None:
        raise ValueError("camera_matrix/dist_coeffs missing in %s" % path)
    return camera_matrix, dist_coeffs


def save_matrix_yaml(path: str, name: str, matrix: np.ndarray) -> None:
    """用 OpenCV YAML 格式保存一个矩阵。"""
    import cv2

    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    fs = cv2.FileStorage(path, cv2.FILE_STORAGE_WRITE)
    if not fs.isOpened():
        raise RuntimeError("Cannot write yaml file: %s" % path)
    fs.write(name, np.asarray(matrix, dtype=np.float64))
    fs.release()
