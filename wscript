#!/usr/bin/env python
import sys, os
from waflib import Options
symap2ic = os.environ.get('SYMAP2IC_PATH')
sys.path.insert(0, os.path.join(symap2ic, 'src/waf'))
from symwaf2ic import *

components = []
recurse    = lambda ctx : map(lambda proj: ctx.recurse(proj), components)

APPNAME='logger'

def options(opt):
    #ctx.add_option('--log-color', action='store', default=False,
            #help='use color for log output (default: False)')

    recurse(opt)
    opt.load('g++')
    opt.load('boost')

def configure(cfg):
    #if Options.options.log_color:
        #ctx.env.CXXFLAGS_LOGGER += ['-DLOG_COLOR_OUTPUT',]

    recurse(cfg)

    cfg.load('g++')
    cfg.load('boost')

    cfg.check_boost('system thread', uselib_store='BOOST4LOGGER')
    cfg.check_cxx(lib='log4cxx', uselib_store='LOG4CXX', mandatory=1)

def build(bld):
    recurse(bld)

    bld (
        target          = 'logger_obj',
        export_includes = '.',
        use             = ['chatty'],
    )

    bld(
        features        = 'cxx cxxprogram',
        source          = 'usage_example/main.cpp',
        target          = 'logger_example',
        use             = [
            'BOOST4LOGGER',
            'LOG4CXX',
            'logger_obj',
            ],
        install_path    = 'bin',
        cxxflags        = [],
    )
