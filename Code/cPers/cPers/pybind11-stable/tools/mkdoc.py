#!/usr/bin/env python3
#
#  Syntax: mkdoc.py [-I<path> ..] [.. a list of header files ..]
#
#  Extract documentation from C++ header files to use it in Python bindings
#

import os, sys, platform, re, textwrap
from clang import cindex
from clang.cindex import CursorKind
from collections import OrderedDict
from threading import Thread, Semaphore
from multiprocessing import cpu_count

RECURSE_LIST = [
    CursorKind.TRANSLATION_UNIT,
    CursorKind.NAMESPACE,
    CursorKind.CLASS_DECL,
    CursorKind.STRUCT_DECL,
    CursorKind.CLASS_TEMPLATE
]

PRINT_LIST = [
    CursorKind.CLASS_DECL,
    CursorKind.STRUCT_DECL,
    CursorKind.CLASS_TEMPLATE,
    CursorKind.FUNCTION_DECL,
    CursorKind.FUNCTION_TEMPLATE,
    CursorKind.CXX_METHOD,
    CursorKind.CONSTRUCTOR,
    CursorKind.FIELD_DECL
]

CPP_OPERATORS = {
    '<=' : 'le', '>=' : 'ge', '==' : 'eq', '!=' : 'ne', '[]' : 'array',
    '+=' : 'iadd', '-=' : 'isub', '*=' : 'imul', '/=' : 'idiv', '%=' :
    'imod', '&=' : 'iand', '|=' : 'ior', '^=' : 'ixor', '<<=' : 'ilshift',
    '>>=' : 'irshift', '++' : 'inc', '--' : 'dec', '<<' : 'lshift', '>>' :
    'rshift', '&&' : 'land', '||' : 'lor', '!' : 'lnot', '~' : 'bnot', '&'
    : 'band', '|' : 'bor', '+' : 'add', '-' : 'sub', '*' : 'mul', '/' :
    'div', '%' : 'mod', '<' : 'lt', '>' : 'gt', '=' : 'assign'
}
CPP_OPERATORS = OrderedDict(sorted(CPP_OPERATORS.items(), key=lambda t: -len(t[0])))

job_count = cpu_count()
job_semaphore = Semaphore(job_count)

registered_names = dict()

def d(s):
    return s.decode('utf8')

def sanitize_name(name):
    global registered_names
    for k, v in CPP_OPERATORS.items():
        name = name.replace('operator%s' % k, 'operator_%s' % v)
    name = name.replace('<', '_')
    name = name.replace('>', '_')
    name = name.replace(' ', '_')
    name = name.replace(',', '_')
    if name in registered_names:
        registered_names[name] += 1
        name += '_' + str(registered_names[name])
    else:
        registered_names[name] = 1
    return '__doc_' + name

def process_comment(comment):
    result = ''

    # Remove C++ comment syntax
    for s in comment.splitlines():
        s = s.strip()
        if s.startswith('/*'):
            s = s[2:].lstrip('* \t')
        elif s.endswith('*/'):
            s = s[:-2].rstrip('* \t')
        elif s.startswith('///'):
            s = s[3:]
        if s.startswith('*'):
            s = s[1:]
        result += s.strip() + '\n'

    # Doxygen tags
    cpp_group = '([\w:]+)'
    param_group = '([\[\w:\]]+)'

    s = result
    s = re.sub(r'\\c\s+%s' % cpp_group, r'``\1``', s)
    s = re.sub(r'\\a\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\e\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\em\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\b\s+%s' % cpp_group, r'**\1**', s)
    s = re.sub(r'\\param%s?\s+%s' % (param_group, cpp_group), r'\n\n$Parameter ``\2``:\n\n', s)

    for in_, out_ in {
        'return' : 'Returns',
        'author' : 'Author',
        'authors' : 'Authors',
        'copyright' : 'Copyright',
        'date' : 'Date',
        'remark' : 'Remark',
        'sa' : 'See also',
        'see' : 'See also',
        'extends' : 'Extends',
        'throw' : 'Throws',
        'throws' : 'Throws' }.items():
        s = re.sub(r'\\%s\s*' % in_, r'\n\n$%s:\n\n' % out_, s)

    s = re.sub(r'\\details\s*', r'\n\n', s)
    s = re.sub(r'\\brief\s*', r'', s)
    s = re.sub(r'\\short\s*', r'', s)
    s = re.sub(r'\\ref\s*', r'', s)

    # HTML/TeX tags
    s = re.sub(r'<tt>([^<]*)</tt>', r'``\1``', s)
    s = re.sub(r'<em>([^<]*)</em>', r'*\1*', s)
    s = re.sub(r'<b>([^<]*)</b>', r'**\1**', s)
    s = re.sub(r'\\f\$([^\$]*)\\f\$', r'$\1$', s)

    s = s.replace('``true``', '``True``')
    s = s.replace('``false``', '``False``')

    # Re-flow text
    wrapper = textwrap.TextWrapper()
    wrapper.expand_tabs = True
    wrapper.replace_whitespace = True
    wrapper.width = 75
    wrapper.initial_indent = wrapper.subsequent_indent = ''

    result = ''
    for x in re.split(r'\n{2,}', s):
        wrapped = wrapper.fill(x.strip())
        if len(wrapped) > 0 and wrapped[0] == '$':
            result += wrapped[1:] + '\n'
            wrapper.initial_indent = wrapper.subsequent_indent = ' '*4
        else:
            result += wrapped + '\n\n'
            wrapper.initial_indent = wrapper.subsequent_indent = ''
    return result.rstrip()


def extract(filename, node, prefix, output):
    num_extracted = 0
    if not (node.location.file is None or os.path.samefile(d(node.location.file.name), filename)):
        return 0
    if node.kind in RECURSE_LIST:
        sub_prefix = prefix
        if node.kind != CursorKind.TRANSLATION_UNIT:
            if len(sub_prefix) > 0:
                sub_prefix += '_'
            sub_prefix += d(node.spelling)
        for i in node.get_children():
            num_extracted += extract(filename, i, sub_prefix, output)
        if num_extracted == 0:
            return 0
    if node.kind in PRINT_LIST:
        comment = d(node.raw_comment) if node.raw_comment is not None else ''
        comment = process_comment(comment)
        name = sanitize_name(prefix + '_' + d(node.spelling))
        output.append('\nstatic const char *%s = %sR"doc(%s)doc";' % (name, '\n' if '\n' in comment else '', comment))
        num_extracted += 1
    return num_extracted

class ExtractionThread(Thread):
    def __init__ (self, filename, parameters, output):
        Thread.__init__(self)
        self.filename = filename
        self.parameters = parameters
        self.output = output
        job_semaphore.acquire()

    def run(self):
        print('Processing "%s" ..' % self.filename, file = sys.stderr)
        try:
            index = cindex.Index(cindex.conf.lib.clang_createIndex(False, True))
            tu = index.parse(self.filename, self.parameters)
            extract(self.filename, tu.cursor, '', self.output)
        finally:
            job_semaphore.release()

if __name__ == '__main__':
    parameters = ['-x', 'c++', '-std=c++11']
    filenames = []

    if platform.system() == 'Darwin':
        libclang = '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/libclang.dylib'
        if os.path.exists(libclang):
            cindex.Config.set_library_path(os.path.dirname(libclang))

        base_path = '/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs'
        if os.path.exists(base_path):
            sysroot = os.path.join(base_path, next(os.walk(base_path))[1][0])
            parameters.append('-isysroot')
            parameters.append(sysroot)

    for item in sys.argv[1:]:
        if item.startswith('-'):
            parameters.append(item)
        else:
            filenames.append(item)

    if len(filenames) == 0:
        print('Syntax: %s [.. a list of header files ..]' % sys.argv[0])
        exit(-1)

    print('''/*
  This file contains docstrings for the Python bindings.
  Do not edit! These were automatically extracted by mkdoc.py
 */

#define __EXPAND(x)                              x
#define __COUNT(_1, _2, _3, _4, _5, COUNT, ...)  COUNT
#define __VA_SIZE(...)                           __EXPAND(__COUNT(__VA_ARGS__, 5, 4, 3, 2, 1))
#define __CAT1(a, b)                             a ## b
#define __CAT2(a, b)                             __CAT1(a, b)
#define __DOC1(n1)                               __doc_##n1
#define __DOC2(n1, n2)                           __doc_##n1##_##n2
#define __DOC3(n1, n2, n3)                       __doc_##n1##_##n2##_##n3
#define __DOC4(n1, n2, n3, n4)                   __doc_##n1##_##n2##_##n3##_##n4
#define __DOC5(n1, n2, n3, n4, n5)               __doc_##n1##_##n2##_##n3##_##n4_##n5
#define DOC(...)                                 __EXPAND(__EXPAND(__CAT2(__DOC, __VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
''')

    output = []
    for filename in filenames:
        thr = ExtractionThread(filename, parameters, output)
        thr.start()

    print('Waiting for jobs to finish ..', file = sys.stderr)
    for i in range(job_count):
        job_semaphore.acquire()

    output.sort()
    for l in output:
        print(l)

    print('''
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
''')
