#!/usr/bin/env python
import sys, os
sys.path.insert(0, os.path.join(os.environ['SYMAP2IC_PATH'], 'src/waf'))
from symwaf2ic import *

APPNAME='logger'

# We have to set our build root to symap2ic.
top = SYMAP2IC_PATH


def options(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')


def configure(ctx):
    ctx.load('compiler_c')
    ctx.load('compiler_cxx')
    ctx.load('boost')

    ctx.check_boost(lib='thread', mandatory=True)
    ctx.check_cxx(header_name='boost/shared_ptr.hpp', mandatory=True)

    # def check_boost_thread():
    #     import Configure
    #     try:
    #         conf.check_cxx(header_name='boost/thread.hpp', lib="boost_thread-mt",
    #                        uselib_store='BOOST_THREAD', mandatory=1)
    #         return
    #     except Configure.ConfigurationError:
    #         conf.check_message_2('Non-standard boost_thread installation? Another try:', 'PINK')
    #     try:
    #         conf.check_cxx(header_name='boost/thread.hpp', lib="boost_thread",
    #                        uselib_store='BOOST_THREAD', mandatory=1)
    #         return
    #     except Configure.ConfigurationError:
    #         raise
    # check_boost_thread()
    #check_BOOST_THREAD(conf)


def build(bld):
    bld.objects (
        target          = 'logger_obj',
        source          = 'logger.cpp',
        includes        = '.',
        export_includes = '.',
        cxxflags        = ['-O0', '-g', '-fPIC'],
        use             = ['BOOST_THREAD'],
    )

    # this target is deprecated
    bld.new_task_gen (
        target          = 'logger',
        features        = 'cxx cxxstlib',
        use             = ['BOOST_THREAD', 'logger_obj'],
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
