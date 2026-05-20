Pod::Spec.new do |s|
  s.name         = 'justifier_native'
  s.version      = '0.0.1'
  s.summary      = 'Justifier audio engine compiled via CMake'
  s.description  = 'Builds the C/C++ audio engine (Faust DSP + miniaudio) via CMake for macOS and iOS.'
  s.homepage     = 'https://github.com/ninthhouse/justifier'
  s.license      = { :type => 'Proprietary' }
  s.author       = 'Ninth House'

  s.osx.deployment_target = '10.15'
  s.ios.deployment_target = '13.0'

  s.source       = { :path => '.' }

  native_dir = File.expand_path('..', __dir__)

  # CocoaPods requires at least one source file per platform
  s.osx.source_files = 'dummy.c'
  s.ios.source_files = 'dummy.m'

  # --- macOS: build shared dylib ---

  macos_build_script = <<~SCRIPT
    set -euo pipefail
    [[ "${PLATFORM_NAME}" == "macosx" ]] || exit 0
    NATIVE_DIR="#{native_dir}/native"
    BUILD_DIR="${NATIVE_DIR}/build-macos"
    DYLIB_NAME="libjustifier_audio.dylib"
    mkdir -p "${BUILD_DIR}"
    rm -rf "${BUILD_DIR}/CMakeCache.txt"
    cmake -S "${NATIVE_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
    cmake --build "${BUILD_DIR}" --target justifier_audio -j $(sysctl -n hw.ncpu)
    install_name_tool -id "@rpath/${DYLIB_NAME}" "${BUILD_DIR}/${DYLIB_NAME}" 2>/dev/null || true
    mkdir -p "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}"
    cp -f "${BUILD_DIR}/${DYLIB_NAME}" "${BUILT_PRODUCTS_DIR}/${FRAMEWORKS_FOLDER_PATH}/"
  SCRIPT

  # --- iOS: build static lib ---

  ios_build_script = <<~SCRIPT
    set -euo pipefail
    [[ "${PLATFORM_NAME}" == "iphoneos" || "${PLATFORM_NAME}" == "iphonesimulator" ]] || exit 0
    NATIVE_DIR="#{native_dir}/native"
    BUILD_DIR="${NATIVE_DIR}/build-ios-${PLATFORM_NAME}"
    LIB_NAME="libjustifier_audio.a"
    mkdir -p "${BUILD_DIR}"
    rm -rf "${BUILD_DIR}/CMakeCache.txt"

    CMAKE_EXTRA_ARGS=""
    if [[ "${PLATFORM_NAME}" == "iphonesimulator" ]]; then
      CMAKE_EXTRA_ARGS="-DCMAKE_OSX_SYSROOT=iphonesimulator"
    else
      CMAKE_EXTRA_ARGS="-DCMAKE_OSX_SYSROOT=iphoneos"
    fi

    # Convert space-separated ARCHS to semicolon-separated CMake list
    CMAKE_ARCHS=$(echo "${ARCHS}" | tr ' ' ';')
    cmake -S "${NATIVE_DIR}" -B "${BUILD_DIR}" \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_ARCHITECTURES="${CMAKE_ARCHS}" \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
      -DCMAKE_BUILD_TYPE=Release \
      -DJUSTIFIER_REGEN_FAUST=OFF \
      ${CMAKE_EXTRA_ARGS}
    cmake --build "${BUILD_DIR}" --target justifier_audio -j $(sysctl -n hw.ncpu)
    mkdir -p "${BUILT_PRODUCTS_DIR}"
    cp -f "${BUILD_DIR}/${LIB_NAME}" "${BUILT_PRODUCTS_DIR}/${LIB_NAME}"
  SCRIPT

  s.script_phases = [
    {
      :name => 'Build justifier_audio via CMake (macOS)',
      :script => macos_build_script,
      :execution_position => :before_compile
    },
    {
      :name => 'Build justifier_audio via CMake (iOS)',
      :script => ios_build_script,
      :execution_position => :before_compile,
      :output_files => ['$(BUILT_PRODUCTS_DIR)/libjustifier_audio.a']
    }
  ]

  s.ios.frameworks = 'CoreAudio', 'AudioToolbox', 'CoreFoundation', 'AVFoundation'
  s.ios.libraries = 'c++'

  s.ios.pod_target_xcconfig = {
    'OTHER_LDFLAGS' => '-force_load "$(BUILT_PRODUCTS_DIR)/libjustifier_audio.a"'
  }
end
