if (TARGET Editor AND TARGET EditorPluginAmplitudeAudio)
    # Make sure this project is built when the Editor is built
    add_dependencies(Editor EditorPluginAmplitudeAudio)
endif()
