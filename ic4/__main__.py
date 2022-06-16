"""
    Main entry point of the program
"""
from sys import stdin
from typing import Iterable


from .assembler.parser import parse
from .assembler.codegen import generate


def usage(argv):
    """Print the usage message"""
    print(f"Usage: {argv[0]} assemble [FILE]")


def main(argv: Iterable[str]) -> int:
    try:
        if len(argv) < 2 or argv[1] != "assemble":
            raise ValueError("For now, only valid action is 'assemble'")

        # read source code
        if len(argv) > 2:
            with open(argv[2]) as inp:
                source = inp.read()
        else:
            source = stdin.read()
    except Exception:
        usage(argv)
        raise

    # parse the source code
    commands = parse(source)

    # generate output code
    code = generate(commands)

    print(','.join(str(val) for val in code))


if __name__ == "__main__":
    from sys import argv
    exit(main(argv))
