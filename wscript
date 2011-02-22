#!/usr/bin/env python
import sys, os
sys.path.insert(0, os.path.join(os.environ['SYMAP2IC_PATH'], 'src/waf'))
from symwaf2ic import *

APPNAME='logger'

# this should support top-hack and normal building
import Options
(top,out) = tophack(Options.Context.launch_dir)


def options(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')


def configure(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')

    check_BOOST_THREAD(ctx)
    ctx.check_cxx(header_name='boost/shared_ptr.hpp', mandatory=True)

    ctx.env.INCLUDES_LOGGER = ['.',]
    ctx.env.CXXFLAGS_LOGGER = ['-O0', '-g', '-fPIC']


def build(bld):
    bld.objects (
        target          = 'logger_obj',
        source          = 'logger.cpp',
        export_includes = bld.env.INCLUDES_LOGGER,
        uselib          = ['BOOST_THREAD', 'LOGGER'],
    )

    # this target is deprecated
    bld.new_task_gen (
        target          = 'logger',
        features        = 'cxx cxxstlib',
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = ['BOOST_THREAD', 'LOGGER', 'logger_obj'],
        install_path    = None,
    )

    bld.objects (
        target          = 'logger_c_obj',
        source          = 'logger_c.cpp',
        includes        = '.',
        export_includes = '.',
        cxxflags        = ['-O0', '-g', '-fPIC'],
        use             = ['BOOST_THREAD'],
    )

    # this target is deprecated
    bld.new_task_gen(
        target          = 'logger_c',
        features        = 'cxx cxxstlib',
        use             = ['BOOST_THREAD', 'logger_obj', 'logger_c_obj'],
        install_path    = None,
    )

    bld.add_subdirs('usage_example')
