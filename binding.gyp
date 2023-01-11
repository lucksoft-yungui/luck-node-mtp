{
  'targets': [
    {
      'target_name': 'luck-node-mtp',
      'sources': [ 'src/luck_mtp.cc', 'src/utils.h','src/utils.cc'],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      'conditions': [
            ['OS!="win"',
              {
                "library_dirs": [
                    "../lib"
                ],
                "libraries": [
        	        "-lmtp"
                ],
              }
            ],
            ['OS=="win"',
              {
                  'library_dirs': [
                      '../lib'
                  ],
                  'libraries': [
                    '-l../lib/libmtp.lib'
                  ],
                  'copies': [
                      {
                        'destination': '$(SolutionDir)$(ConfigurationName)',
                        'files': [
                          '<(module_root_dir)/lib/libmtp-9.dll.dll',
                          '<(module_root_dir)/lib/libusb-1.0.dll'
                        ]
                      }
                  ]
              }
            ]
        ],
    }
  ]
}