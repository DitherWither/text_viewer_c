text_viewer_c_sources = [
  'main.c',
  'text_viewer_c-application.c',
  'text_viewer_c-window.c',
]

text_viewer_c_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1'),
]

blueprints = custom_target('blueprints',
  input: files(
    'gtk/help-overlay.blp',
    'text_viewer_c-window.blp',
  ),
  output: '.',
  command: [find_program('blueprint-compiler'), 'batch-compile', '@OUTPUT@', '@CURRENT_SOURCE_DIR@', '@INPUT@'],
)

text_viewer_c_sources += gnome.compile_resources('text_viewer_c-resources',
  'text_viewer_c.gresource.xml',
  dependencies: blueprints,
  c_name: 'text_viewer_c'
)

executable('text_viewer_c', text_viewer_c_sources,
  dependencies: text_viewer_c_deps,
       install: true,
)
