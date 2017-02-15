{
  'variables': {
    'component%': 'static_library',
    'visibility%': 'hidden',
    'library%': 'static_library',
  },

  'conditions': [
    ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris" \
       or OS=="netbsd" or OS=="android"', {
      'target_defaults': {
        'cflags': [ '-pthread' ],
        'cflags_cc': [ '-fno-rtti', '-std=c++11', '-fexceptions', ],
        'cflags!': [ '-Werror', ],
        'ldflags': [ '-pthread', ],
      },
    }],
    ['OS=="mac"', {
      'xcode_settings': {
        'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
        'CLANG_CXX_LIBRARY': 'libc++',
      },
    }],
  ],
  'target_conditions': [
    ['OS=="android" and not "host" in toolsets', {
      'target_defaults': {
        'cflags!': [
          '-pthread',  # Not fully supported by Android toolchain. Built-in suport of major functions, no need to link with it.
        ],
        'ldflags!': [
          '-llog'
          '-pthread',  # Not fully supported by Android toolchain. Built-in suport of major functions, no need to link with it.
        ],
      },
    }],
  ],

  'target_defaults': {
    'configurations': {
      'Debug': {},
      'Release': {}
    },
    'msvs_cygwin_shell': 0,
    'target_conditions': [
      ['_type=="static_library"', {
        'standalone_static_library': 1
      }]
    ]
  }
}
