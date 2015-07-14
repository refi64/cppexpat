from fbuild.builders.cxx import guess_static
from fbuild.config import cxx as cxx_test
from fbuild.record import Record
from fbuild.path import Path
from fbuild.db import caches

from optparse import make_option

class Expat(cxx_test.Test):
    expat_h = cxx_test.header_test('expat.h')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.external_libs.append('expat')

def pre_options(parser):
    group = parser.add_option_group('config options')
    group.add_options((
        make_option('--cxx', help='Use the given C++ compiler'),
        make_option('--use-color', help='Use colored output from the compiler',
                    action='store_true')
    ))

@caches
def configure(ctx):
    fl = ['-fdiagnostics-color'] if ctx.options.use_color else []
    cxx = guess_static(ctx, flags=fl, includes=['.'], platform_options=[
        ({'windows'}, {'flags+': ['/EHsc']}),
        ({'posix'}, {'flags+': ['-std=c++11']})
    ])
    if not Expat(cxx).expat_h: return
    return Record(cxx=cxx)

def build(ctx):
    cxx = configure(ctx).cxx
    for exe in 'ex1', 'ex2':
        cxx.build_exe(exe, [Path(exe).replaceext('.cpp')],
            external_libs=['expat'])
    ctx.install('cppexpat.hpp', 'include')
