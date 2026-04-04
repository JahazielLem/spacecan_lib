# Examples directory for SpaceCAN
controller = executable(
    'controller',
    'controller.c',
    dependencies: [spacecan_dep],
    install: false
)
segmentation = executable(
    'segmentation',
    'segmentation.c',
    dependencies: [spacecan_dep],
    install: false
)
scsniffer = executable(
    'scsniffer',
    'scsniffer.c',
    dependencies: [spacecan_dep],
    install: false
)
scinjection = executable(
    'scinjection',
    'scinjection.c',
    dependencies: [spacecan_dep],
    install: false
)
screeplay = executable(
    'screeplay',
    'screplay.c',
    dependencies: [spacecan_dep],
    install: false
)
scmonitor = executable(
    'scmonitor',
    'scmonitor.c',
    dependencies: [spacecan_dep, curses_dep],
    install: false
)
scviewer = executable(
    'scviewer',
    'scviewer.c',
    dependencies: [spacecan_dep, curses_dep],
    install: false
)

test('scviewer', scviewer)
test('scmonitor', scmonitor)
test('screplay', screeplay)
test('scinjection', scinjection)
test('scsniffer', scsniffer)
test('segmentation', segmentation)
test('controller', controller)