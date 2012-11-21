#!/usr/bin/env python
import sys, os
from waflib import Options
symap2ic = os.environ.get('SYMAP2IC_PATH')
sys.path.insert(0, os.path.join(symap2ic, 'src/waf'))
from symwaf2ic import *

components = [os.path.join(symap2ic , 'components', 'chatty'), ]
recurse    = lambda ctx : map(lambda proj: ctx.recurse(proj), components)

APPNAME='logger'

def options(ctx):
    #ctx.add_option('--log-color', action='store', default=False,
            #help='use color for log output (default: False)')

    recurse(ctx)

def configure(ctx):
    #if Options.options.log_color:
        #ctx.env.CXXFLAGS_LOGGER += ['-DLOG_COLOR_OUTPUT',]

    recurse(ctx)

def build(bld):
    recurse(bld)

    bld (
        target          = 'logger_obj',
        export_includes = '.',
        use             = ['chatty'],
    )

    bld(
        features        = 'cxx cxxprogram',
        source          = 'usage_example/main.cpp',
        target          = 'logger_example',
        use             = ['logger_obj'],
        install_path    = None,
        cxxflags        = [],
        lib             = ['log4cxx',],
    )
