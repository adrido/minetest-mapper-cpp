#!/usr/bin/env python
import sys
from PIL import Image

def avg2(a, b):
	return int((a + b) / 2.0)

def avg2t3i0(a, b):
	return tuple(avg2(t[0], t[1]) for t in zip(a[:3], b[:3]))

if len(sys.argv) <= 1:
	print("Usage: %s <input>" % sys.argv[0])
else:
	inp = Image.open(sys.argv[1])
	inp = inp.convert('RGBA')
	ind = inp.load()
	avgc = -1
	for x in range(inp.size[0]):
		for y in range(inp.size[1]):
			pxl = ind[x, y]
			if pxl[3] < 128:
				continue
			if avgc == -1:
				avgc = pxl[:3]
			else:
				avgc = avg2t3i0(avgc, pxl)
	if avgc != -1:
		print("%d %d %d" % avgc)
