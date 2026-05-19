#!/usr/bin/env python3
from dataclasses import dataclass
from typing import Dict, Optional, Union


FieldValue = Union[int, str]


@dataclass(frozen=True)
class VisionMessage:
    kind: str
    fields: Dict[str, FieldValue]
    raw: str
    color: Optional[str] = None
    payload: Optional[str] = None
    target: Optional[str] = None


def _to_int(value: str, field_name: str) -> int:
    try:
        return int(value)
    except ValueError as exc:
        raise ValueError("Field %s must be an integer: %r" % (field_name, value)) from exc


def parse_line(line: str) -> VisionMessage:
    raw = line.strip()
    if not raw:
        raise ValueError("Empty protocol line")

    parts = [part.strip() for part in raw.split(",")]
    kind = parts[0].upper()

    if kind == "QR":
        if len(parts) < 2:
            raise ValueError("QR message requires payload")
        payload = ",".join(parts[1:])
        return VisionMessage(kind="QR", payload=payload, fields={"payload": payload}, raw=raw)

    if kind == "BLOB":
        if len(parts) < 7:
            raise ValueError("BLOB message requires at least color,cx,cy,w,h,area")
        names = ["cx", "cy", "w", "h", "area", "dx", "dy"]
        values = parts[2:]
        fields: Dict[str, FieldValue] = {}
        for name, value in zip(names, values):
            fields[name] = _to_int(value, name)
        return VisionMessage(kind="BLOB", color=parts[1].upper(), fields=fields, raw=raw)

    if kind == "RING":
        if len(parts) < 11:
            raise ValueError("RING message requires color,cx,cy,dx,dy,radius,score,density,ratio,source")
        int_names = ["cx", "cy", "dx", "dy", "radius", "score", "density", "ratio"]
        fields = {}
        for name, value in zip(int_names, parts[2:10]):
            fields[name] = _to_int(value, name)
        fields["source"] = parts[10]
        return VisionMessage(kind="RING", color=parts[1].upper(), fields=fields, raw=raw)

    if kind == "LINE":
        if len(parts) < 3:
            raise ValueError("LINE message requires dx,theta")
        return VisionMessage(
            kind="LINE",
            fields={"dx": _to_int(parts[1], "dx"), "theta": _to_int(parts[2], "theta")},
            raw=raw,
        )

    if kind == "NONE":
        if len(parts) < 2:
            raise ValueError("NONE message requires target")
        color = parts[2].upper() if len(parts) > 2 else None
        return VisionMessage(kind="NONE", target=parts[1].upper(), color=color, fields={}, raw=raw)

    if kind in ("HELLO", "ERR", "WARN", "INFO"):
        return VisionMessage(kind=kind, fields={"text": ",".join(parts[1:])}, raw=raw)

    raise ValueError("Unknown protocol message kind: %s" % kind)


def format_mode_command(command: str) -> bytes:
    text = command.strip()
    if not text:
        raise ValueError("Mode command cannot be empty")
    return (text + "\n").encode("utf-8")
