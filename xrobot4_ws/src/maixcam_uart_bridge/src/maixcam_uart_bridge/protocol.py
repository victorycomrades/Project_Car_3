"""
MaixCam UART 文本协议解析。

协议格式（以换行 \\n 结尾）：

    MaixCam -> Jetson:
    QR,payload
    BLOB,color,dx,dy,cx,cy,w,h,area
    BLOB,color,dx,dy,cx,cy,w,h,area,worksite,height_mm
    RING,color,dx,dy,cx,cy,radius,score,density,ratio,source
    NONE,type[,color]
    HELLO,text
    INFO,text
    ERR,text
    PONG,text

  Jetson -> MaixCam:
    MODE,QR
    MODE,BLOB,RAW|PROCESS|STORAGE1,RED|GREEN|BLUE|ALL
    MODE,BLOB,RED|GREEN|BLUE|ALL
    MODE,RING,RED|GREEN|BLUE|ALL
    MODE,IDLE
    PING
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional, Union

FieldValue = Union[int, str]


@dataclass(frozen=True)
class VisionMessage:
    kind: str
    fields: Dict[str, FieldValue] = field(default_factory=dict)
    raw: str = ""
    color: Optional[str] = None
    payload: Optional[str] = None
    target: Optional[str] = None


def _to_int(value: str, field_name: str) -> int:
    try:
        return int(value)
    except ValueError:
        raise ValueError("Field %s must be an integer, got %r" % (field_name, value))


def parse_line(line: str) -> VisionMessage:
    """解析一行 MaixCam 协议文本，返回 VisionMessage。"""
    raw = line.strip()
    if not raw:
        raise ValueError("Empty protocol line")

    parts = [part.strip() for part in raw.split(",")]
    kind = parts[0].upper()

    # ---- QR,payload ------------------------------------------------------
    if kind == "QR":
        if len(parts) < 2:
            raise ValueError("QR message requires payload")
        payload = parts[1]
        fields: Dict[str, FieldValue] = {"payload": payload}
        if len(parts) >= 4:
            fields["dx"] = _to_int(parts[2], "dx")
            fields["dy"] = _to_int(parts[3], "dy")
        return VisionMessage(kind="QR", payload=payload, fields=fields, raw=raw)

    # ---- BLOB,color,dx,dy,cx,cy,w,h,area[,worksite,height_mm] ------------
    if kind == "BLOB":
        if len(parts) < 9:
            raise ValueError("BLOB requires color,dx,dy,cx,cy,w,h,area (9 parts)")
        color = parts[1].upper()
        names = ["dx", "dy", "cx", "cy", "w", "h", "area"]
        fields = {}
        for name, value in zip(names, parts[2:9]):
            fields[name] = _to_int(value, name)
        if len(parts) >= 10 and parts[9]:
            fields["worksite"] = parts[9].upper()
        if len(parts) >= 11 and parts[10]:
            fields["height_mm"] = _to_int(parts[10], "height_mm")
        return VisionMessage(kind="BLOB", color=color, fields=fields, raw=raw)

    # ---- RING,color,dx,dy,cx,cy,radius,score,density,ratio,source --------
    if kind == "RING":
        if len(parts) < 11:
            raise ValueError("RING requires color,dx,dy,cx,cy,radius,score,density,ratio,source")
        color = parts[1].upper()
        int_names = ["dx", "dy", "cx", "cy", "radius", "score", "density", "ratio"]
        fields = {}
        for name, value in zip(int_names, parts[2:10]):
            fields[name] = _to_int(value, name)
        fields["source"] = parts[10]
        return VisionMessage(kind="RING", color=color, fields=fields, raw=raw)

    # ---- NONE,type[,color] -----------------------------------------------
    if kind == "NONE":
        if len(parts) < 2:
            raise ValueError("NONE requires target type")
        target = parts[1].upper()
        fields = {}
        color = None
        if target == "BLOB" and len(parts) >= 4:
            fields["worksite"] = parts[2].upper()
            color = parts[3].upper()
        elif len(parts) > 2:
            color = parts[2].upper()
        return VisionMessage(kind="NONE", target=target, color=color, fields=fields, raw=raw)

    # ---- HELLO / INFO / WARN / ERR / PONG --------------------------------
    if kind in ("HELLO", "INFO", "WARN", "ERR", "PONG"):
        return VisionMessage(kind=kind, fields={"text": ",".join(parts[1:]) if len(parts) > 1 else ""}, raw=raw)

    raise ValueError("Unknown protocol message kind: %s" % kind)


def format_mode_command(command: str) -> bytes:
    """将模式命令格式化为带换行的字节串。"""
    text = command.strip()
    if not text:
        raise ValueError("Mode command cannot be empty")
    if text.upper().startswith("PING"):
        return (text.strip() + "\n").encode("utf-8")
    if not text.upper().startswith("MODE,"):
        text = "MODE," + text
    return (text + "\n").encode("utf-8")
