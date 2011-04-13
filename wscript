#!/usr/bin/env python
import sys, os
sys.path.insert(0, os.path.join(os.environ['SYMAP2IC_PATH'], 'src/waf'))
from symwaf2ic import *

APPNAME='logger'

# this should support top-hack and normal building
from waflib import Options
(top,out) = tophack(Options.Context.launch_dir)


def options(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')
    ctx.add_option('--log_color', action='store', default=False, help='use color for log output (default: False)')


def configure(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')

    ctx.check_boost(lib='thread', mandatory=True)
    ctx.check_cxx(header_name='boost/shared_ptr.hpp', mandatory=True)

    ctx.env.INCLUDES_LOGGER    = ['.',]
    ctx.env.CXXFLAGS_LOGGER    = ['-O0', '-g', '-fPIC']
    ctx.env.CXXFLAGS_LOGGEROBJ = ['-O0', '-g', '-fPIC']
    if Options.options.log_color:
        ctx.env.CXXFLAGS_LOGGEROBJ += ['-DLOG_COLOR_OUTPUT',]
        ctx.env.color = True


def build(bld):
    bld.objects (
        target          = 'loggerobj',
        source          = 'logger.cpp',
        use             = ['BOOST_THREAD', 'LOGGEROBJ'],
    )

    # this target is deprecated
    bld(
        target          = 'logger',
        features        = 'cxx cxxstlib',
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = ['loggerobj', ],
        install_path    = None,
    )

    bld.objects (
        target          = 'logger_c_obj',
        source          = 'logger_c.cpp',
        export_includes = bld.env.INCLUDES_LOGGER,
        cxxflags        = bld.env.CXXFLAGS_LOGGER,
        use             = ['BOOST_THREAD'],
    )

    # this target is deprecated
    bld(
        target          = 'logger_c',
        features        = 'cxx cxxstlib',
        use             = ['loggerobj', 'logger_c_obj'],
        install_path    = None,
    )

    if bld.env.color:
        bld.recurse('usage_example')
