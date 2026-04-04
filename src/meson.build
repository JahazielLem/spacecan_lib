# Include directories
inc_dirs = include_directories('.')

curses_dep = dependency('ncurses')

spacecan_sources = files(['frame.c', 'node.c', 'bus.c', 'busd.c', 'sensor.c', 'motor.c', 'battery.c'])
# Library target
spacecan_lib = library(
    'spacecan',
    sources: spacecan_sources,
    include_directories: inc_dirs,
    dependencies: [curses_dep],
    version: lib_version,
    soversion: '0',
    install: true
)

# Install headers
install_headers(
    'spacecan.h',
    'frame.h',
    'node.h',
    'bus.h',
    'busd.h',
    'sensor.h',
    'motor.h',
    'battery.h',
    subdir: 'spacecan'
)

# Create dependency object for use by other projects
spacecan_dep = declare_dependency(
    include_directories: inc_dirs,
    link_with: spacecan_lib
)