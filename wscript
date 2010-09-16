#!/usr/bin/env python
import sys, os
sys.path.insert(0, os.path.join(os.environ['SYMAP2IC_PATH'], 'src/waf'))
from symwaf2ic import *

APPNAME='logger'

# We have to set our build root to symap2ic.
srcdir = SYMAP2IC_PATH


def set_options(opt):
    opt.tool_options('compiler_cxx')
    opt.tool_options('compiler_cc')


def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('compiler_cc')
    conf.check_cxx(header_name='boost/shared_ptr.hpp', mandatory=1)

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
    check_BOOST_THREAD(conf)


def build(bld):
    bld.new_task_gen(
        target         = 'logger',
        features       = 'cxx cstaticlib',
        source         = 'logger.cpp',
        includes       = '.',
        export_incdirs = '.',
        cxxflags       = ['-O0', '-g', '-fPIC'],
        uselib         = ['BOOST_THREAD'],
        install_path   = None,
    )

    bld.new_task_gen(
        target         = 'logger_c',
        features       = 'cxx cstaticlib',
        source         = 'logger_c.cpp',
        includes       = '.',
        export_incdirs = '.',
        cxxflags       = ['-O0', '-g', '-fPIC'],
        uselib         = ['BOOST_THREAD'],
        uselib_local   = 'logger',
        install_path   = None,
    )

    bld.add_subdirs('usage_example')
