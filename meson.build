project('lunasvg-python', 'c', 'cpp',
    version: '0.0.9',
    license: 'MIT',
    meson_version: '>=0.64.0',
    default_options: ['cpp_std=c++11']
)

version_info = meson.project_version().split('.')
version_major = version_info[0].to_int()
version_minor = version_info[1].to_int()
version_micro = version_info[2].to_int()

add_project_arguments('-DPYLUNASVG_VERSION_MAJOR=@0@'.format(version_major), language: ['cpp'])
add_project_arguments('-DPYLUNASVG_VERSION_MINOR=@0@'.format(version_minor), language: ['cpp'])
add_project_arguments('-DPYLUNASVG_VERSION_MICRO=@0@'.format(version_micro), language: ['cpp'])

python = import('python').find_installation(pure: false)

lunasvg_deps = [
    python.dependency()
]

lunasvg_dep = dependency('lunasvg',
    required: not get_option('sdist'),
    version: '>=3.0.0',
    fallback: ['lunasvg', 'lunasvg_dep']
)

if lunasvg_dep.found()
    lunasvg_deps += [lunasvg_dep]
endif

sources = [
    'source/__init__.py',
    'source/__init__.pyi',
    'source/__main__.py',
    'source/py.typed'
]

python.install_sources(sources, subdir: 'lunasvg')
python.extension_module(
    '_lunasvg',
    'source/module.cpp',
    dependencies: lunasvg_deps,
    subdir: 'lunasvg',
    install: true
)
