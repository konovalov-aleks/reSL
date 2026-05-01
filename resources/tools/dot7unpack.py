#!/usr/bin/env python3

''' Utility to extract images from the original .7 resource files '''

import argparse
import os

from PIL import Image
import numpy as np

from palette import Colors


def palette():
    ''' generate palette for PIL Image '''
    res = []
    for i in range(16):
        r, g, b = Colors[i]
        res += [r, g, b]
    return res


def import_image(width, height, data):
    ''' import data to PIL Image '''

    # mask: m m m m | p p p p    // p - planes
    #                            // m - bool, different data for rows

    data_ptr = 0x10

    image_data = np.zeros([height, width], dtype=np.uint8)

    for y in range(0, height, 7):
        for x in range(0, width, 8):
            drawn = 0
            while drawn != 0xF:
                m = data[data_ptr]
                planes = m & 0xF
                drawn |= planes
                data_ptr += 1

                use_const = (m & 0xF0) == 0
                for row in range(7):
                    col_mask = data[data_ptr]
                    if not use_const:
                        data_ptr += 1
                    for col in range(8):
                        if x + col >= image_data.shape[1]:
                            break
                        if y + row >= image_data.shape[0]:
                            break
                        image_data[y + row, x + col] = \
                            image_data[y + row, x + col] & (~planes & 0xFF)
                        if col_mask & 0x80:
                            image_data[y + row, x + col] |= planes
                        col_mask <<= 1
                if use_const:
                    data_ptr += 1
    if len(data) > data_ptr:
        print('WARNING: Not all data was read. '
              'Probably incorrect resolution specified')
    image = Image.fromarray(image_data, 'P')
    image.putpalette(palette())
    return image


def extract(inpfile, outpfile, width, height):
    with open(inpfile, 'rb') as f:
        data = f.read()
    import_image(width, height, data).save(outpfile)
    print(f'exported to {outpfile}')


def main():
    p = argparse.ArgumentParser()
    p.add_argument('-W', '--width', required=True, type=int)
    p.add_argument('-H', '--height', required=True, type=int)
    p.add_argument('-o', '--output')
    p.add_argument('file')
    args = p.parse_args()
    if not args.output:
        args.output = os.path.splitext(args.file)[0] + '.png'
    extract(args.file, args.output, args.width, args.height)


if __name__ == '__main__':
    main()
