#!/usr/bin/env python
from waflib import Errors, Logs
import os

def options(opt):
    opt.load('compiler_cxx')
    opt.load('boost')
    hopts = opt.add_option_group('Logger Options')
    hopts.add_option('--enable-deprecated', action='store_true', default=False,
                   help='Enable old logger (non-log4cxx version)')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('boost')

    cfg.check_boost('system thread', uselib_store='BOOST4LOGGER')
    if cfg.options.enable_deprecated:
        Logs.pprint('PINK', "Using old-style logger (deprecated!)")
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('deprecated').abspath()
    else:
        cfg.check_cxx(lib='log4cxx', uselib_store='LOG4CXX', mandatory=True)
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('log4cxx').abspath()


def build(bld):
    bld.objects(
        target          = 'logger_obj',
        source          = [bld.root.find_node(os.path.join(bld.env.INCLUDES_LOGGER, 'logger.cpp'))],
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = [
            'LOGGER',
            'BOOST4LOGGER',
            'LOG4CXX',
        ],
    )

    for program in bld.path.ant_glob('usage_example/*.cpp'):
        bld.program(
                target = '%s' % os.path.splitext(program.relpath())[0],
                source = [program],
                use = ['logger_obj'],
                install_path = 'bin')

