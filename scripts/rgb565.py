from argparse import ArgumentParser
import sys
from PIL import Image

def _to_str(a: bytes) -> str:
    return '\n'.join(f'{b:08b}' for b in a)


def main():
    parser = ArgumentParser()
    parser.add_argument('-i', '--input', type=str, required=True)
    parser.add_argument('-o', '--output', type=str, required=True)
    parser.add_argument('-t', '--text', action='store_true')
    args = parser.parse_args()
    
    try:
        img = Image.open(args.input)
        img = img.convert('RGB')
            
        pixels = img.load()
        r5g6b5_data = bytearray()
        for y in range(img.height):
            for x in range(img.width):
                r, g, b = pixels[x, y]
                
                r = r * 31 // 255
                g = g * 63 // 255
                b = b * 31 // 255
                
                r5g6b5_data += (r << 11 | g << 5 | b).to_bytes(2, byteorder='big')
            

        if args.text:
            r5g6b5_data = _to_str(r5g6b5_data)
            mode = 't'
        else:
            mode = 'b'

        open(args.output, f'w{mode}').write(r5g6b5_data)

        sys.exit(0)
    except RuntimeError as e:
        print()
        print(e)
        sys.exit(-1)


if __name__ == "__main__":
    main()
