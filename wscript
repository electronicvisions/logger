#!/usr/bin/env python
import os

def options(opt):
    opt.load('compiler_cxx')
    opt.load('boost')

def configure(cfg):
    cfg.load('compiler_cxx')
    cfg.load('boost')

    cfg.check_boost('system thread', uselib_store='BOOST4LOGGER')
    cfg.check_cxx(lib='log4cxx', uselib_store='LOG4CXX', mandatory=True)

def build(bld):
    bld (
        target          = 'logger_obj',
        export_includes = '.',
        use             = [
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

