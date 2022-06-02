project('ic4', 'c', default_options : 'c_std=c99', version : '0.2')

src_dir = 'src'
test_dir = 'tests'

directories={}

# Default executable
sources = []
subdir(src_dir)

executable('ic4', [prog_main, sources])

# collecting tests
test_mains = []
subdir(test_dir)

foreach t : test_mains
    exe = executable(t[0], [t[1], tap_file, sources], include_directories: t[2])
    test(t[0], exe)
endforeach