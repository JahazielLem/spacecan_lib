project('spacecan', 'c',
        version : run_command('git', '-C', meson.project_source_root(), 'describe', '--tags', '--always', check: false).stdout().strip(),
        default_options : ['warning_level=3', 'c_std=c11'])

subdir('src')
subdir('examples')

pkg = import('pkgconfig')

pkg.generate(
    spacecan_lib,
    name : 'spacecan',
    description : 'A lightweight C implementation of the SpaceCAN protocol',
    filebase : 'spacecan',
    subdirs : 'spacecan',
    version : meson.project_version(),
)

summary({
    'Version': meson.project_version(),
    'Build type': get_option('buildtype'),
    'Install prefix': get_option('prefix'),
}, section: 'Project Info')