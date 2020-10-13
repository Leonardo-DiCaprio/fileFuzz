#-----------------------------------------------------------------
# pycparser: __init__.py
#
# This package file exports some convenience functions for
# interacting with pycparser
#
# Copyright (C) 2008-2012, Eli Bendersky
# License: BSD
#-----------------------------------------------------------------
__all__ = ['c_lexer', 'c_parser', 'c_ast']
__version__ = '{{VERSION}}'


import os
import subprocess
import tempfile


from .c_parser import CParser
from py010parser.ply import cpp
import py010parser.ply.lex as lex


def preprocess_file(filename, cpp_path='cpp', cpp_args=''):
    """ Preprocess a file using cpp.

        filename:
            Name of the file you want to preprocess.

        cpp_path:
        cpp_args:
            Refer to the documentation of parse_file for the meaning of these
            arguments.

        When successful, returns the preprocessed file's contents.
        Errors from cpp will be printed out.
    """
    path_list = [cpp_path]
    if isinstance(cpp_args, list):
        path_list += cpp_args
    elif cpp_args != '':
        path_list += [cpp_args]
    path_list += [filename]

    try:
        # Note the use of universal_newlines to treat all newlines
        # as \n for Python's purpose
        #
        pipe = subprocess.Popen(
            path_list,
            stdout             = subprocess.PIPE,
            universal_newlines = True
        )
        text = pipe.communicate()[0]
    except OSError as e:
        raise RuntimeError("Unable to invoke 'cpp'.  " +
            'Make sure its path was passed correctly\n' +
            ('Original error: %s' % e))

    return text


def parse_file(filename, use_cpp=True, cpp_path='cpp', cpp_args='',
               parser=None, predefine_types=True, keep_scopes=False):
    """ Parse a C file using pycparser.

        filename:
            Name of the file you want to parse.

        use_cpp:
            Set to True if you want to execute the C pre-processor
            on the file prior to parsing it.

        cpp_path:
            If use_cpp is True, this is the path to 'cpp' on your
            system. If no path is provided, it attempts to just
            execute 'cpp', so it must be in your PATH.

        cpp_args:
            If use_cpp is True, set this to the command line arguments strings
            to cpp. Be careful with quotes - it's best to pass a raw string
            (r'') here. For example:
            r'-I../utils/fake_libc_include'
            If several arguments are required, pass a list of strings.

        parser:
            Optional parser object to be used instead of the default CParser

        keep_scopes:
            If the parser should retain its scope stack for type names from previous
            parsings.

        When successful, an AST is returned. ParseError can be
        thrown if the file doesn't parse successfully.

        Errors from cpp will be printed out.
    """
    if use_cpp:
        text = preprocess_file(filename, cpp_path, cpp_args)
    else:
        with open(filename, 'rU') as f:
            text = f.read()

    if parser is None:
        parser = CParser()

    return parser.parse(
        text,
        filename,
        predefine_types = predefine_types,
        keep_scopes     = keep_scopes
    )

def parse_string(text, parser=None, filename="<string>", optimize=True, predefine_types=True,
        use_cpp=True, cpp_path='cpp', cpp_args='', keep_scopes=False):
    
    if use_cpp:
        tempfile_path = ''
        with tempfile.NamedTemporaryFile("w", delete=False) as f:
            f.write(text)
            f.flush()
            tempfile_path = f.name

        text = preprocess_file(tempfile_path, cpp_path, cpp_args)
        os.unlink(tempfile_path)

    if parser is None:
        parser = CParser(
            lex_optimize=optimize,
            yacc_optimize=optimize
        )

    return parser.parse(
        text,
        filename,
        predefine_types = predefine_types,
        keep_scopes     = keep_scopes
    )
