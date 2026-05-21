#!/usr/bin/env python3
"""maixcam_uart_bridge 协议解析单元测试。

运行方式：
  cd ~/xrobot4_ws
  python3 -m pytest src/maixcam_uart_bridge/test/test_protocol.py -v

或使用 unittest：
  python3 -m unittest discover -s src/maixcam_uart_bridge/test -v
"""

import os
import sys
import unittest

PKG_SRC = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "src"))
sys.path.insert(0, PKG_SRC)

from maixcam_uart_bridge.protocol import format_mode_command, parse_line


class ProtocolTest(unittest.TestCase):

    # ---- QR ----
    def test_parse_qr_full(self):
        msg = parse_line("QR,123+231,-5,-12")
        self.assertEqual(msg.kind, "QR")
        self.assertEqual(msg.payload, "123+231")
        self.assertEqual(msg.fields["dx"], -5)
        self.assertEqual(msg.fields["dy"], -12)

    def test_parse_qr_no_offset(self):
        msg = parse_line("QR,123+231")
        self.assertEqual(msg.payload, "123+231")

    # ---- BLOB (新格式: dx,dy,cx,cy,w,h,area) ----
    def test_parse_blob(self):
        msg = parse_line("BLOB,RED,-6,-18,154,102,38,41,1558")
        self.assertEqual(msg.kind, "BLOB")
        self.assertEqual(msg.color, "RED")
        self.assertEqual(msg.fields["dx"], -6)
        self.assertEqual(msg.fields["dy"], -18)
        self.assertEqual(msg.fields["cx"], 154)
        self.assertEqual(msg.fields["cy"], 102)
        self.assertEqual(msg.fields["area"], 1558)

    # ---- RING (新格式: dx,dy,cx,cy,radius,score,density,ratio,source) ----
    def test_parse_ring(self):
        msg = parse_line("RING,BLUE,0,-1,160,119,52,82,18,94,MULTI_HOUGH3")
        self.assertEqual(msg.kind, "RING")
        self.assertEqual(msg.color, "BLUE")
        self.assertEqual(msg.fields["dx"], 0)
        self.assertEqual(msg.fields["dy"], -1)
        self.assertEqual(msg.fields["radius"], 52)
        self.assertEqual(msg.fields["source"], "MULTI_HOUGH3")

    # ---- LINE ----
    def test_parse_line(self):
        msg = parse_line("LINE,-12,87")
        self.assertEqual(msg.kind, "LINE")
        self.assertEqual(msg.fields["dx"], -12)
        self.assertEqual(msg.fields["theta"], 87)

    # ---- NONE ----
    def test_parse_none(self):
        msg = parse_line("NONE,BLOB,GREEN")
        self.assertEqual(msg.kind, "NONE")
        self.assertEqual(msg.target, "BLOB")
        self.assertEqual(msg.color, "GREEN")

    # ---- 状态消息 ----
    def test_parse_hello(self):
        msg = parse_line("HELLO,MAIXCAM_UART,115200")
        self.assertEqual(msg.kind, "HELLO")
        self.assertEqual(msg.fields["text"], "MAIXCAM_UART,115200")

    # ---- 错误处理 ----
    def test_rejects_bad_integer(self):
        with self.assertRaises(ValueError):
            parse_line("BLOB,RED,-6,nope,154,102,38,41,1558")

    def test_rejects_empty(self):
        with self.assertRaises(ValueError):
            parse_line("")

    def test_rejects_unknown(self):
        with self.assertRaises(ValueError):
            parse_line("GARBAGE,x,y,z")

    # ---- format_mode_command ----
    def test_format_mode(self):
        self.assertEqual(format_mode_command("MODE,QR"), b"MODE,QR\n")

    def test_format_mode_auto_prefix(self):
        self.assertEqual(format_mode_command("QR"), b"MODE,QR\n")

    def test_format_ping(self):
        self.assertEqual(format_mode_command("PING"), b"PING\n")


if __name__ == "__main__":
    unittest.main()
