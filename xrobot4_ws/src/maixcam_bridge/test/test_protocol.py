#!/usr/bin/env python3
import os
import sys
import unittest


PKG_SRC = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "src"))
sys.path.insert(0, PKG_SRC)

from maixcam_bridge.protocol import format_mode_command, parse_line


class ProtocolTest(unittest.TestCase):
    def test_parse_qr_payload(self):
        msg = parse_line("QR,123+231")

        self.assertEqual(msg.kind, "QR")
        self.assertEqual(msg.payload, "123+231")
        self.assertEqual(msg.fields, {"payload": "123+231"})

    def test_parse_blob_measurement(self):
        msg = parse_line("BLOB,RED,154,102,38,41,1558,-6,-18")

        self.assertEqual(msg.kind, "BLOB")
        self.assertEqual(msg.color, "RED")
        self.assertEqual(msg.fields["cx"], 154)
        self.assertEqual(msg.fields["cy"], 102)
        self.assertEqual(msg.fields["dx"], -6)
        self.assertEqual(msg.fields["dy"], -18)

    def test_parse_ring_measurement(self):
        msg = parse_line("RING,BLUE,160,119,0,-1,52,82,18,94,MULTI_HOUGH3")

        self.assertEqual(msg.kind, "RING")
        self.assertEqual(msg.color, "BLUE")
        self.assertEqual(msg.fields["radius"], 52)
        self.assertEqual(msg.fields["source"], "MULTI_HOUGH3")

    def test_parse_line_measurement(self):
        msg = parse_line("LINE,-12,87")

        self.assertEqual(msg.kind, "LINE")
        self.assertEqual(msg.fields, {"dx": -12, "theta": 87})

    def test_parse_none_result(self):
        msg = parse_line("NONE,BLOB,GREEN")

        self.assertEqual(msg.kind, "NONE")
        self.assertEqual(msg.target, "BLOB")
        self.assertEqual(msg.color, "GREEN")

    def test_format_mode_command_adds_newline(self):
        self.assertEqual(format_mode_command("MODE,BLOB,RED"), b"MODE,BLOB,RED\n")

    def test_parse_rejects_bad_integer(self):
        with self.assertRaises(ValueError):
            parse_line("BLOB,RED,154,nope,38,41")


if __name__ == "__main__":
    unittest.main()
