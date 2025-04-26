import sys
from PIL import Image
import numpy as np
from argparse import ArgumentParser


def main():
    parser = ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True)
    parser.add_argument('-o', '--output', type=str, required=True)
    args = parser.parse_args()

    img = Image.open(args.input).convert('L')  # 'L' => 8-bit grayscale
    img = img.resize((28, 28), Image.Resampling.BILINEAR)
    arr_uint8 = np.array(img, dtype=np.uint8)
    arr_int8 = (arr_uint8 - 128).astype(np.int8)
    arr_int8 = arr_int8.reshape(-1)
    open(args.output, 'wb').write(arr_int8.tobytes())

    sys.exit(0)


if __name__ == '__main__':
    main()
