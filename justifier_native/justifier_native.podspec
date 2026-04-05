Pod::Spec.new do |s|
  s.name         = 'justifier_native'
  s.version      = '0.0.1'
  s.summary      = 'Justifier audio engine compiled for macOS'
  s.description  = 'Builds the C/C++ audio engine (Faust DSP + miniaudio) via CMake for macOS.'
  s.homepage     = 'https://github.com/ninthhouse/justifier'
  s.license      = { :type => 'Proprietary' }
  s.author       = 'Ninth House'

  s.osx.deployment_target = '10.15'

  s.source       = { :path => '.' }

  # CocoaPods requires at least one source file. This dummy registers the pod
  # so the script_phase runs. The real work is the CMake-built dylib.
  s.source_files = 'dummy.c'

  native_dir = File.expand_path('..', __dir__)

  # Build the dylib with CMake (same build system as Linux) and copy it
  # into the app's Frameworks directory so Dart FFI can load it.
  build_script = "set -euo pipefail\n" \
    "NATIVE_DIR=\"#{native_dir}/native\"\n" \
    "BUILD_DIR=\"${NATIVE_DIR}/build-macos\"\n" \
    "DYLIB_NAME=\"libjustifier_audio.dylib\"\n" \
    "mkdir -p \"${BUILD_DIR}\"\n" \
    "rm -rf \"${BUILD_DIR}/CMakeCache.txt\"\n" \
    "cmake -S \"${NATIVE_DIR}\" -B \"${BUILD_DIR}\" -DCMAKE_BUILD_TYPE=Release\n" \
    "cmake --build \"${BUILD_DIR}\" --target justifier_audio -j $(sysctl -n hw.ncpu)\n" \
    "install_name_tool -id \"@rpath/${DYLIB_NAME}\" \"${BUILD_DIR}/${DYLIB_NAME}\" 2>/dev/null || true\n" \
    "mkdir -p \"${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}\"\n" \
    "cp -f \"${BUILD_DIR}/${DYLIB_NAME}\" \"${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/\"\n"

  s.script_phase = {
    :name => 'Build justifier_audio via CMake',
    :script => build_script,
    :execution_position => :before_compile
  }
end
