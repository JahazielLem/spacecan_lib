# Include directories
inc_dirs = include_directories('.')

# Source files for SparkCLI library
spacecan_sources = files(['frame.c', 'node.c', 'bus.c'])
# Library target
spacecan_lib = library(
    'spacecan',
    sources: spacecan_sources,
    include_directories: inc_dirs,
    install: true
)

# Install headers
install_headers(
    'spacecan.h',
    'frame.h',
    'node.h',
    'bus.h',
    subdir: 'spacecan'
)

# Create dependency object for use by other projects
spacecan_dep = declare_dependency(
    include_directories: inc_dirs,
    link_with: spacecan_lib
)