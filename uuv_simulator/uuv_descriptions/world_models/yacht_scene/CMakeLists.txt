set(MODEL_NAME yacht_scene)

set (files
  model.sdf
  model.config
  yacht.dae
  yacht-collision.dae
  boat_d.tga
  pier-floor-texture.jpg
  soil_sand_0045_01.jpg
  wildtextures-old-wood-original-file.jpg
  wood1.jpg
  wood2.jpg
  wood3.jpg
  wood-textures-high-quality-1.jpg
  b1.jpg
  dif3.jpg
  fass_texture.png
  m1.jpg 
  pl3.jpg
)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/gazebo-${GAZEBO_VERSION}/models/${MODEL_NAME})
