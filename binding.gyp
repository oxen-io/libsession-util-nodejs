{
  "targets": [
    {
      "target_name": "session_util_wrapper",
      "sources": [ "src/utilities.cpp", "src/base_config.cpp", "src/user_config.cpp", "src/contacts_config.cpp", "src/user_groups_config.cpp", "src/addon.cpp" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "<(module_root_dir)/include/"
        ],
      "libraries": [ "<(module_root_dir)/lib/libsession-util.a"],
      "cflags_cc": [
        "-std=c++17",
        "-fexceptions",
        "-frtti",
      ],
      "cflags": [
        "-std=c++17",
        "-fexceptions",
        "-frtti",
      ],
      "xcode_settings": {
        "OTHER_CFLAGS": [
          "-std=c++17",
          "-fexceptions",
          "-frtti",
        ],
      },
      "msbuild_settings": {
        "ClCompile": {
            "LanguageStandard": "stdcpp17"
        }
      },
      'msvs_settings': {
        'VCCLCompilerTool': { "ExceptionHandling": 1, 'AdditionalOptions': [ '-std:c++17' ] }
      }
    }
  ]
}