Pod::Spec.new do |s|
  s.name         = 'justifier_native'
  s.version      = '0.0.1'
  s.summary      = 'Justifier audio engine compiled for macOS'
  s.description  = 'Compiles the C/C++ audio engine (Faust DSP + miniaudio) for macOS.'
  s.homepage     = 'https://github.com/ninthhouse/justifier'
  s.license      = { :type => 'Proprietary' }
  s.author       = 'Ninth House'

  s.osx.deployment_target = '10.15'

  s.source       = { :path => '.' }

  native_dir = File.join(__dir__, '..', 'native')
  dsp_dir    = File.join(native_dir, 'dsp')
  gen_dir    = File.join(__dir__, 'generated')

  # Generate Faust DSP C++ files during pod install.
  # Each .dsp -> <Name>DSP class in generated/<name>_dsp.cpp
  dsp_names = %w[sine triangle saw square pulse noise fm]
  faust_cmds = dsp_names.map do |name|
    class_name = name.capitalize + 'DSP'
    "faust -lang cpp -cn #{class_name} -i \"#{dsp_dir}/#{name}.dsp\" -o \"#{gen_dir}/#{name}_dsp.cpp\""
  end

  s.prepare_command = <<-CMD
    mkdir -p generated
    #{faust_cmds.join("\n    ")}
  CMD

  s.source_files = [
    '../native/src/*.{c,cpp,h}',
    '../native/include/*.{h}',
    'generated/*_dsp.cpp',
  ]

  s.header_mappings_dir = '../native/src'
  s.libraries = 'm'

  # Faust include path (macOS: brew install faust)
  faust_include = `faust --includedir 2>/dev/null`.strip
  faust_include = '/usr/local/include' if faust_include.empty?

  s.compiler_flags = "-std=c++17 -O2 -w -DJUSTIFIER_BUILDING"
  s.frameworks     = 'CoreAudio', 'AudioToolbox', 'CoreFoundation'

  s.pod_target_xcconfig = {
    'DEFINES_MODULE' => 'YES',
    'HEADER_SEARCH_PATHS' => [
      "\"#{File.expand_path('../native/include', __dir__)}\"",
      "\"#{File.expand_path('../native/src', __dir__)}\"",
      "\"#{gen_dir}\"",
      "\"#{faust_include}\"",
    ].join(' '),
    'OTHER_LDFLAGS' => '-lm',
  }
end
