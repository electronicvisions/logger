#!/usr/bin/env python

def options(opt):
    opt.load('g++')
    opt.load('boost')

def configure(cfg):
    cfg.load('g++')
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

    bld(
        features        = 'cxx cxxprogram',
        source          = 'usage_example/main.cpp',
        target          = 'logger_example',
        use             = [ 'logger_obj' ],
        install_path    = 'bin',
        cxxflags        = [],
    )
