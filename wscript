#!/usr/bin/env python
import sys, os
from waflib import Options
sys.path.insert(0, os.path.join(os.environ['SYMAP2IC_PATH'], 'src/waf'))
from symwaf2ic import *

APPNAME='logger'

def options(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')

    ctx.add_option('--log-color', action='store', default=False,
            help='use color for log output (default: False)')


def configure(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')

    ctx.fix_boost_paths()
    ctx.check_boost(lib='serialization system thread program_options',
            uselib_store='BOOST4LOGGER')

    ctx.env.INCLUDES_LOGGER    = ['.',]

    if Options.options.log_color:
        ctx.env.CXXFLAGS_LOGGER += ['-DLOG_COLOR_OUTPUT',]


def build(bld):
    OBJCXXFLAGS = [ '-fPIC', '-O0', '-g' ]

    bld.objects (
        target          = 'logger_obj',
        source          = 'logger.cpp',
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = ['BOOST4LOGGER', 'LOGGER'],
        cxxflags        = OBJCXXFLAGS,
    )

    bld.objects (
        target          = 'logger_c_obj',
        source          = 'logger_c.cpp',
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = ['logger_obj', ],
        cxxflags        = OBJCXXFLAGS,
    )

    bld.recurse('usage_example')
