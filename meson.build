project('spacecan', 'c',
        version : run_command('git', '-C', meson.project_source_root(), 'describe', '--tags', '--always', check: false).stdout().strip(),
        default_options : ['warning_level=3', 'c_std=c11'])

subdir('src')
subdir('examples')

pkg = import('pkgconfig')

pkg.generate(
    spacecan_lib,
    name : 'spacecan',
    description : 'SpaceCAN Lib',
    filebase : 'spacecan',
    subdirs : 'spacecan',
    version : meson.project_version(),
)
