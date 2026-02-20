# Examples directory for SpaceCAN
controller_demo = executable(
    'controller_demo',
    'controller.c',
    dependencies: [spacecan_dep],
    install: false
)
segmentation_demo = executable(
    'segmentation_demo',
    'segmentation.c',
    dependencies: [spacecan_dep],
    install: false
)
spacecansniffer_demo = executable(
    'spacecansniffer_demo',
    'spacecansniffer.c',
    dependencies: [spacecan_dep],
    install: false
)
injection_demo = executable(
    'injection_demo',
    'injection.c',
    dependencies: [spacecan_dep],
    install: false
)
replay_demo = executable(
    'replay_demo',
    'replay.c',
    dependencies: [spacecan_dep],
    install: false
)

test('screplay', replay_demo)
test('scinjection', injection_demo)
test('scsniffer', spacecansniffer_demo)
test('segmentation_demo', segmentation_demo)
test('controller_demo', controller_demo)