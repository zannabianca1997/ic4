project('ic4', 'c', default_options : 'c_std=c99', version : '0.2')

# Default executable
sources = []
subdir('src')

executable('ic4', [prog_main, sources])

# collecting tests
test_mains = []
subdir('tests')

foreach t : test_mains
    exe = executable(t[0], [t[1], tap_file, sources])
    test(t[0], exe)
endforeach