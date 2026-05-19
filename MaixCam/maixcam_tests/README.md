# MaixCam / MaixVision 功能测试脚本

这些脚本用于在 MaixVision 中快速验证 MaixCam 的单项视觉功能。代码实际运行在 MaixCam 设备上，不是在 Windows 本地 Python 解释器中运行。

## 脚本说明

- `01_qrcode_test.py`：二维码识别，打印二维码内容并画出二维码轮廓。
- `02_color_blob_test.py`：红、绿、蓝色块识别基础版。
- `02_color_blob_center_test.py`：色块中心点输出版，更接近后续机械臂对准需求。
- `03_line_tracking_test.py`：直线/巡线测试，使用 `get_regression()` 对阈值命中的像素拟合直线。
- `04_vision_mode_test.py`：综合模式测试，通过修改 `MODE` 切换 `QR`、`BLOB_RED`、`BLOB_GREEN`、`BLOB_BLUE`、`BLOB_ALL`、`LINE`。
- `05_color_ring_test.py`：基础色环中心识别版本，用于快速验证 LAB 阈值和色环粗定位。
- `06_color_ring_multi_circle_test.py`：多真实圆检测与圆心融合实验版，保留 `05_color_ring_test.py` 不变。

## 建议测试顺序

1. 运行 `01_qrcode_test.py`，确认摄像头、显示和二维码识别正常。
2. 运行 `02_color_blob_center_test.py`，确认红绿蓝物料中心点能输出。
3. 运行 `05_color_ring_test.py`，先确认色环颜色阈值能粗定位。
4. 运行 `06_color_ring_multi_circle_test.py`，验证多圆融合是否能提升圆心稳定性。
5. 有真实赛道或线条样片后，再运行 `03_line_tracking_test.py`。
6. 单功能稳定后，用 `04_vision_mode_test.py` 模拟后续状态机切换。

## 重要注意事项

- 优先使用 `320x240` 分辨率，避免 MaixCam 快帧缓冲内存不足。
- 色彩识别不稳定时，使用 MaixCam 自带 `Find Blobs` 应用重新标定 LAB 阈值。
- 巡线不稳定时，先缩小 `ROI`，避免桌面边缘、纸张边缘、阴影参与拟合。
- 色环检测建议先单色调试，例如 `TARGET = "RED"`，稳定后再使用 `TARGET = "ALL"`。
- 调试图形只用于观察，机械臂最终需要的是 `cx, cy` 和 `dx, dy`。
- 如果 `06` 输出经常是 `BLOB`，说明霍夫圆没有稳定检测到，应先回到 `05` 调阈值和 ROI。
