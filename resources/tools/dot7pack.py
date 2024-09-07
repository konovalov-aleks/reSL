#!/usr/bin/env python3

''' Utility to convert an image to .7 format '''

import argparse
import os
import struct

from PIL import Image
import numpy as np

from palette import palette


_DOT7HDR = b'\x22\x00\x07\x38\x3F\x3E\x06\x19\x11\x24\x04\x1B\x03\x12\x02\x3F'


def export_dot7(img, f):
    ''' encodes image data into .7 format '''
    f.write(_DOT7HDR)
    for y in range(0, img.shape[0], 7):
        for x in range(0, img.shape[1], 8):
            planes = {}
            for bit in range(4):
                plane_mask = 1 << bit
                col_masks = []
                for row in range(7):
                    cur_col_mask = 0
                    for col in range(8):
                        cur_col_mask <<= 1
                        if x + col >= img.shape[1]:
                            break
                        if y + row >= img.shape[0]:
                            break
                        if img[y + row, x + col] & plane_mask:
                            cur_col_mask |= 1
                    col_masks.append(cur_col_mask)

                if np.all(np.array(col_masks) == col_masks[0]):
                    col_masks = [col_masks[0]]

                col_masks_bytes = \
                    b''.join(struct.pack('B', x) for x in col_masks)
                planes.setdefault(col_masks_bytes, 0)
                planes[col_masks_bytes] |= plane_mask

            for col_masks, plane_mask in planes.items():
                assert len(col_masks) == 1 or len(col_masks) == 7
                if len(col_masks) == 7:
                    plane_mask |= 0b10000
                f.write(struct.pack('B', plane_mask))
                for b in col_masks:
                    f.write(struct.pack('B', b))


def pack(imgfile, outpfile):
    img = Image.open(imgfile)
    img = img.convert('P', palette=palette())
    with open(outpfile, 'wb') as f:
        export_dot7(np.array(img), f)


def main():
    p = argparse.ArgumentParser()
    p.add_argument('-o', '--output')
    p.add_argument('file')
    args = p.parse_args()
    if not args.output:
        args.output = os.path.splitext(args.file)[0] + '.7'
    pack(args.file, args.output)


if __name__ == '__main__':
    main()
