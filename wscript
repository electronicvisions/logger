#!/usr/bin/env python
from waflib import Errors, Logs
from waflib.extras.gtest import summary
import argparse, os

def options(opt):
    opt.load('compiler_cxx')
    opt.load('boost')
    opt.load('gtest')

    try:
        # ECM: transition to new logger requires catching overlapping options with old logger
        hopts = opt.add_option_group('Logger Options')
        hopts.add_option('--enable-deprecated', action='store_true', default=False,
                       help='Enable old logger (non-log4cxx version)')
        hopts.add_option('--disable-colorlog', action='store_true', default=False,
                       help='Disable color output for logger')
    except argparse.ArgumentError:
        pass


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('boost')
    cfg.load('gtest')

    cfg.check_boost('system thread filesystem', uselib_store='BOOST4LOGGER')
    if getattr(cfg.options, 'enable_deprecated', False):
        Logs.pprint('PINK', "Using old-style logger (deprecated!)")
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('include').find_node('logger').find_node('deprecated').abspath()
    else:
        cfg.check_cxx(lib='log4cxx', uselib_store='LOG4CXX', mandatory=True)
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('include').find_node('logger').find_node('log4cxx').abspath()

    if cfg.options.disable_colorlog:
        cfg.env.DEFINES_LOGGER = [ 'CONFIG_NO_COLOR' ]

def build(bld):
    bld(
        target = 'logger_inc',
        export_includes = 'include',
    )

    bld.shlib(
        target          = 'logger',
        source          = bld.path.ant_glob('src/logger/log4cxx/*.cpp'),
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = [
            'logger_inc',
            'LOGGER',
            'BOOST4LOGGER',
            'LOG4CXX',
        ],
        install_path = '${PREFIX}/lib',
    )

    bld.program(
        features     = 'gtest',
        source       = bld.path.ant_glob('tests/*.cpp'),
        target       = 'test_logger',
        install_path = 'bin',
        use          = ['logger'],
    )


    for program in bld.path.ant_glob('usage_example/*.cpp'):
        bld.program(
                target = '%s' % os.path.splitext(program.relpath())[0],
                source = [program],
                use = ['logger'],
                install_path = '${PREFIX}/bin',
        )

    bld.add_post_fun(summary)
