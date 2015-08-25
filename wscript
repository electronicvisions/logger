#!/usr/bin/env python
from waflib import Errors, Logs
import os

def options(opt):
    opt.load('compiler_cxx')
    opt.load('boost')
    hopts = opt.add_option_group('Logger Options')
    hopts.add_option('--enable-deprecated', action='store_true', default=False,
                   help='Enable old logger (non-log4cxx version)')
    hopts.add_option('--disable-colorlog', action='store_true', default=False,
                   help='Disable color output for logger')


def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('boost')

    cfg.check_boost('system thread filesystem', uselib_store='BOOST4LOGGER')
    if getattr(cfg.options, 'enable_deprecated', False):
        Logs.pprint('PINK', "Using old-style logger (deprecated!)")
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('deprecated').abspath()
    else:
        cfg.check_cxx(lib='log4cxx', uselib_store='LOG4CXX', mandatory=True)
        cfg.env.INCLUDES_LOGGER = cfg.path.find_node('log4cxx').abspath()

    if cfg.options.disable_colorlog:
        cfg.env.DEFINES_LOGGER = [ 'CONFIG_NO_COLOR' ]

def build(bld):
    src_dir = bld.root.find_node(os.path.join(bld.env.INCLUDES_LOGGER))
    bld.shlib(
        target          = 'logger',
        name            = 'logger_obj',
        source          = src_dir.ant_glob('*.cpp'),
        export_includes = bld.env.INCLUDES_LOGGER,
        use             = [
            'LOGGER',
            'BOOST4LOGGER',
            'LOG4CXX',
        ],
        install_path = '${PREFIX}/lib',
        cxxflags = ['-Wall', '-Wextra'],
    )

    for program in bld.path.ant_glob('usage_example/*.cpp'):
        bld.program(
                target = '%s' % os.path.splitext(program.relpath())[0],
                source = [program],
                use = ['logger_obj'],
                install_path = '${PREFIX}/bin',
                cxxflags = ['-Wall', '-Wextra', '-fPIC'],
        )

