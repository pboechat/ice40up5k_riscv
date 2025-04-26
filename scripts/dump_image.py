from argparse import ArgumentParser
import sys
from PIL import Image
import numpy as np


def _to_str(a: bytes | bytearray) -> str:
    return '\n'.join(f'{b:08b}' for b in a)


# source: https://en.wikipedia.org/wiki/Otsu%27s_method
def _otsu_intraclass_variance(image_array, threshold):
    return np.nansum(
        [
            np.mean(cls) * np.var(image_array, where=cls)
            for cls in [image_array >= threshold, image_array < threshold]
        ]
    )


def _otsu_threshold(image_array):
    return min(
        range(np.min(image_array) + 1, np.max(image_array)),
        key=lambda th: _otsu_intraclass_variance(image_array, th),
    )


def main():
    parser = ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True)
    parser.add_argument('-o', '--output', type=str, required=True)
    parser.add_argument('-f', '--format', type=str,
                        choices=['rgb565', 'bin'], required=True)
    parser.add_argument('-t', '--text', action='store_true')
    parser.add_argument('-v', '--verbose', action='store_true')
    args = parser.parse_args()

    try:
        image = Image.open(args.input)
        image = image.convert('L' if args.format == 'bin' else 'RGB')

        pixels = image.load()

        image_data = bytearray()

        if args.format == 'bin':
            image_array = np.array(image)
            assert (image_array.dtype == np.uint8)
            threshold = _otsu_threshold(image_array)
            binary_array = (image_array > threshold).flatten()
            padding = (-len(binary_array)) % 8
            if padding:
                binary_array = np.pad(
                    binary_array, (0, padding), constant_values=0)
            packed_array = np.packbits(binary_array)
            image_data = packed_array.tobytes()

            if args.verbose:
                w = image.width / 8
                c = 0
                for b in image_data:
                    print(f'{b:08b}', end='')
                    if c == (w - 1):
                        print()
                    c = (c + 1) % w
        elif args.format == 'rgb565':
            for y in range(image.height):
                for x in range(image.width):
                    r, g, b = pixels[x, y]
                    r = r * 31 // 255
                    g = g * 63 // 255
                    b = b * 31 // 255
                    image_data += (r << 11 | g << 5 |
                                   b).to_bytes(2, byteorder='big')
        else:
            assert (False)

        if args.text:
            image_data = _to_str(image_data)
            mode = 't'
        else:
            mode = 'b'

        open(args.output, f'w{mode}').write(image_data)

        sys.exit(0)
    except RuntimeError as e:
        print()
        print(e)
        sys.exit(-1)


if __name__ == "__main__":
    main()
