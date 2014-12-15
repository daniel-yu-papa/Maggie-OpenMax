import os

def generate(env, gcc_cross_prefix=None, gcc_strict=False, gcc_stop_on_warning=None, gcc_extra_options=''):
    print "ffmpeg omxil generate()!!!"
    if gcc_stop_on_warning == None: gcc_stop_on_warning = env['stop_on_warning']

    ### compiler flags
    if gcc_strict:
        env['CCFLAGS'] = ""
        env['CFLAGS'] = ""
        env.AppendUnique(CCFLAGS = ['-Werror', '-Wno-variadic-macros', '-Wno-long-long', '-Wno-int-to-pointer-cast'])
        env.AppendUnique(CFLAGS  = ['-Wmissing-prototypes', '-Wmissing-declarations'])
    else:
        env['CCFLAGS'] = ""
        env.AppendUnique(CCFLAGS = ['-Wall'])
    
    compiler_defines = ['-D_REENTRANT']
    env.AppendUnique(CCFLAGS  = compiler_defines)
    env.AppendUnique(CPPFLAGS = compiler_defines)
    
    #env.AppendUnique(LINKFLAGS = ['-lpthread', '-lrt', '-ldl'])

    if env['build_config'] == 'Debug':
        env.AppendUnique(CCFLAGS = ['-g', '-DMAG_DEBUG'])
    else:
        env.AppendUnique(CCFLAGS = '-O3')
    
    if gcc_stop_on_warning:
        env.AppendUnique(CCFLAGS = ['-Werror'])
        
    env['STRIP']  = 'strip'

    if gcc_cross_prefix:
        env['ENV']['PATH'] += os.environ['PATH']
        env['AR']     = gcc_cross_prefix+'-ar'
        env['RANLIB'] = gcc_cross_prefix+'-ranlib'
        env['CC']     = gcc_cross_prefix+'-gcc ' + gcc_extra_options
        env['CXX']    = gcc_cross_prefix+'-g++ ' + gcc_extra_options
        env['LINK']   = gcc_cross_prefix+'-g++ ' + gcc_extra_options
        env['STRIP']  = gcc_cross_prefix+'-strip'

    