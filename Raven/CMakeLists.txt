set(GAME_SOURCE_FILES main.cpp
		GameCode/Actions.cpp
		GameCode/Actions.hpp
		GameCode/MovingObject.cpp
		GameCode/MovingObject.hpp
		GameCode/SteeringBehaviours.cpp
		GameCode/SteeringBehaviours.hpp
		GameCode/CellSpacePartition.hpp
		GameCode/Path.hpp
		GameCode/Image.hpp
		GameCode/Wall.hpp
		GameCode/PlayerMove.hpp
		GameCode/PlayerMove.cpp
		GameCode/World.hpp
		GameCode/World.cpp
		GameCode/Utilities.hpp
		GameCode/Logics.cpp
		GameCode/Logics.hpp
		GameCode/SteeringBehavioursUpdate.cpp
		GameCode/SteeringBehavioursUpdate.hpp
		GameCode/RavenScene.cpp
		GameCode/RavenScene.hpp
		GameCode/IntroScene.cpp
		GameCode/IntroScene.hpp)

add_executable(${PROJECT_NAME} ${GAME_SOURCE_FILES})

if (NOT INCLUDE_ENGINE_PROJECT)
	target_link_libraries(${PROJECT_NAME}
		${SGE_LIBRARIES}
		${Boost_LIBRARIES}
		SDL2::SDL2-static
		SDL2::SDL2main
		${GLEW_LIBRARIES}
		${OPENGL_LIBRARIES}
		${GLM_LIBRARIES})
else()
	target_link_libraries(${PROJECT_NAME}

		${Boost_LIBRARIES}
		${GLEW_LIBRARIES}
		${OPENGL_LIBRARIES}
		${GLM_LIBRARIES}
		SDL2::SDL2-static
		SDL2::SDL2main

		Action
		ActionHandler
		IO
		Level
		Logic
		Object
		Game
		Renderer
		Scene
		Utils
		PicoPNG
		Box2D
		
		imagehlp
		dinput8
		dxguid
		user32
		gdi32
		winmm
		imm32
		ole32
		oleaut32
		shell32
		version
		uuid)
endif()

if (INCLUDE_ENGINE_PROJECT)
	get_filename_component(SGE_RELATIVE_PARENT ${CMAKE_CURRENT_SOURCE_DIR} PATH)
	set(GLSL_DIR "${SGE_RELATIVE_PARENT}/SGE/SimpleGameEngine/Renderer/Shaders/res")

	add_custom_command(TARGET ${PROJECT_NAME} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E 
			make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy ${GLSL_DIR}/BatchShader.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy ${GLSL_DIR}/BatchShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy ${GLSL_DIR}/QuadBatchShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy ${GLSL_DIR}/BatchUVShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)

elseif(COPY_SHADERS)

	if (NOT SGE_ROOT)
		message(WARNING "SGE_ROOT not provided, cannot copy SGE shaders.")
	else()
		set(GLSL_DIR ${SGE_ROOT}/include/Renderer/Shaders/res)

		add_custom_command(TARGET ${PROJECT_NAME} 
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E 
				make_directory${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
		add_custom_command(TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy ${GLSL_DIR}/BatchShader.frag ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
		add_custom_command(TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy ${GLSL_DIR}/BatchShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
		add_custom_command(TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy ${GLSL_DIR}/QuadBatchShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
		add_custom_command(TARGET ${PROJECT_NAME}
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E
				copy ${GLSL_DIR}/BatchUVShader.vert ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/)
	endif()
endif()

if (COPY_GAME_RESOURCES)
	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data/Resources ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources)

	add_custom_command(TARGET ${PROJECT_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E
			copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Data/Levels ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Levels)
endif()