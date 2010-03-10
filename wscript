#!/usr/bin/env python
import os

APPNAME='logger'
srcdir = '.'

# We have to set our build root to symap2ic.
srcdir = os.environ['SYMAP2IC_PATH']


def set_options(opt):
    opt.tool_options('compiler_cc')
    opt.tool_options('compiler_cxx')


def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_cxx(header_name='boost/shared_ptr.hpp', mandatory=1)
    conf.check_cxx(header_name='boost/thread.hpp', lib="boost_thread-mt",
                   uselib_store='BOOST_THREAD', mandatory=1)

def build(bld):
    bld.new_task_gen(
        features='cxx cstaticlib',
        source='logger.cpp',
        target='logger',
        install_path=None,
        cxxflags = ['-fPIC']#, '-DPIC']
    )

    bld.add_subdirs('usage_example')
