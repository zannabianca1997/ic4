#!/bin/env python3
""" 
    this script will examin the c source file given as argument in search of test functions and write a main file that will call them all
"""

import logging
from pycparser import c_ast, parse_file, c_generator
import zlib
from typing import List
from sys import stdout
from pathlib import Path
from datetime import datetime
from argparse import ArgumentParser, FileType
VERSION = "1.0"

logger = logging.getLogger(__name__)


class TestFunctions:
    """Iter through all the function found in the file"""

    class FunctionDiscoverer(c_ast.NodeVisitor):
        """Append all the function it visit to a list"""

        def __init__(self) -> None:
            super().__init__()
            self.function_bin = []

        def visit_FuncDef(self, node):
            self.function_bin.append(node.decl)

    def __init__(self, filename: str, test_prefix: str) -> None:
        super().__init__()
        self.filename = filename
        self.test_prefix = test_prefix
        self.function_bin = None

    def __iter__(self) -> None:
        if self.function_bin is None:
            # parse file
            # Note that cpp is used. Provide a path to your own cpp or
            # make sure one exists in PATH.
            ast = parse_file(self.filename, use_cpp=True,
                             cpp_args=r'-Iutils/fake_libc_include')

            # visit ast, finding all the funtion defined
            v = self.FunctionDiscoverer()
            v.visit(ast)
            # filter for functions that match the test prefix
            candidate_fun_bin = [
                f for f in v.function_bin if f.name.startswith(self.test_prefix)]

            # checking for signature -> return type int, no parameters
            function_bin = []
            for f in candidate_fun_bin:
                valid = True
                if f.type.args is not None:
                    logger.warning(
                        f"function {f.name} seems a test function, but has wrong signature (should have no arguments)")
                    valid = False

                rettype = f.type.type
                if not (
                    isinstance(rettype, c_ast.PtrDecl) and
                    not rettype.quals and
                    isinstance(rettype.type, c_ast.TypeDecl) and
                    rettype.type.quals == ["const"] and
                    rettype.type.align is None and
                    isinstance(rettype.type.type, c_ast.IdentifierType) and
                    rettype.type.type.names == ["char"]
                ):
                    logger.warning(
                        f"function {f.name} seems a test function, but has wrong signature (return type should be 'const char *', not '{c_generator.CGenerator().visit(rettype)}')")
                    valid = False

                if valid:
                    function_bin.append(f.name)

            self.function_bin = function_bin

        # yield from the bin
        yield from self.function_bin


def make_test_main(filenames: List[str], main_skel: str, test_prefix: str, gen_script_name: str):
    """Create the test main from the template"""
    funcs = []
    for id, filename in enumerate(filenames):
        funcs.append((f"functable_{id}", filename, list(
            TestFunctions(filename, test_prefix))))

    # replacing integer constants and file names
    main_skel = main_skel.replace(
        "__GEN_SCRIPT_NAME__", gen_script_name
    ).replace(
        "__GEN_SCRIPT_VERSION__", VERSION
    ).replace(
        "__GEN_DATE__", datetime.now().strftime("%Y-%m-%d %H:%M")
    ).replace(
        "__FILE_NAME_LENGTH__", str(
            max(len(f[1]) for f in funcs) if funcs else 0)
    ).replace(
        "__FUNCTIONS_NAME_LENGTH__", str(max(len(fname)
                                             for f in funcs for fname in f[2]) if funcs else 0)
    )

    # splitting skel into section and replicating file list the needed times
    prolog, functable_element, postlog = main_skel.split("__FILES_LIST__")
    main_skel = prolog + "".join(
        functable_element.replace(
            "__FILENAME__", filename
        ).replace(
            "__FUNCTABLE_NAME__", functable_name
        ).replace(
            "__DISCOVERED_TEST_FUNCTIONS__", "\n".join(
                f"const char *{func_name}();" for func_name in file_funcs)
        ).replace(
            "__DISCOVERED_FUNCS_TABLE__", "\n    ".join(
                f"{{\"{func_name}\", &{func_name}}},"for func_name in file_funcs)
        ) for functable_name, filename, file_funcs in funcs) + postlog

    # replacing filetable
    main_skel = main_skel.replace(
        "__DISCOVERED_FILES_TABLE__", "\n    ".join(
            f"{{\"{filename}\", {functable_name}}},"for functable_name, filename, _ in funcs)
    )

    return main_skel


def get_argument_parser(prog):
    parser = ArgumentParser(prog=prog, description=__doc__)

    parser.add_argument("test_source", nargs="*", type=str,
                        help="the source files to search for test functions")
    parser.add_argument("--test_prefix", type=str,
                        default="test_", help="the prefix to test functions")
    parser.add_argument("--main_skel", type=FileType("r"),
                        default=FileType("r")(str(Path(__file__).parent / "test_main.c.skel")), help="the template file for the test main")
    parser.add_argument("--output", "-o", type=FileType("w"),
                        default=stdout, help="The file to generate")

    return parser


def main(argv):
    parsed_args = get_argument_parser(argv[0]).parse_args(argv[1:])

    parsed_args.output.write(make_test_main(
        parsed_args.test_source, parsed_args.main_skel.read(), parsed_args.test_prefix, argv[0]))


if __name__ == "__main__":
    from sys import argv
    main(argv)
