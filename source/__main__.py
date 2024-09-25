import sys
import argparse
import lunasvg

parser = argparse.ArgumentParser(
    prog='lunasvg',
    description='SVG rendering library.'
)

parser.add_argument(
    'input',
    help='input filename or - for stdin'
)

parser.add_argument(
    'output',
    help='output filename or - for stdout'
)

parser.add_argument(
    '--version',
    action='version',
    version=f'LunaSVG version {lunasvg.LUNASVG_VERSION_STRING}'
)

parser.add_argument(
    '--width',
    type=int,
    default=-1,
    help='Sets the width in pixels'
)

parser.add_argument(
    '--height',
    type=int,
    default=-1,
    help='Sets the height in pixels'
)

def str2hex(x: str) -> int:
    return int(x, 16)

parser.add_argument(
    '--background',
    type=str2hex,
    default=0x00000000,
    help='Sets the background color in 0xRRGGBBAA format'
)

def main() -> int:
    args = parser.parse_args()

    input_file = sys.stdin if args.input == '-' else open(args.input, 'r')
    output_file = sys.stdout if args.output == '-' else open(args.output, 'wb')

    document = lunasvg.Document.load_from_data(input_file.read())
    bitmap = document.render_to_bitmap(args.width, args.height, args.background)

    bitmap.write_to_png_stream(output_file)
    return 0

if __name__ == "__main__":
    sys.exit(main())
