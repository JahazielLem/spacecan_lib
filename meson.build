project('spacecan', 'c',
        version : '0.0.1',
        default_options : ['warning_level=3', 'c_std=c11'])

add_project_arguments('-D_DEFAULT_SOURCE', '-D_POSIX_C_SOURCE=2000809L', language: 'c')

raw_version = run_command('git', '-C', meson.project_source_root(), 'describe', '--tags', '--always', check: false).stdout().strip()
lib_version = raw_version.split('-')[0]

if not lib_version.contains('.')
    lib_version = '0.0.0'
endif

subdir('src')
subdir('examples')

pkg = import('pkgconfig')

pkg.generate(
    spacecan_lib,
    name : 'spacecan',
    description : 'A lightweight C implementation of the SpaceCAN protocol',
    filebase : 'spacecan',
    subdirs : 'spacecan',
    version : lib_version,
)

summary({
    'Version': lib_version,
    'Build type': get_option('buildtype'),
    'Install prefix': get_option('prefix'),
}, section: 'Project Info')