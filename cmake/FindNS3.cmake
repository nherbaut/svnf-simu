INCLUDE(SelectLibraryConfigurations)
INCLUDE(FindPackageHandleStandardArgs)

MESSAGE(${NS3_FIND_VERSION} "              " ${NS3_FIND_COMPONENTS})

# List of the valid ns${NS3_FIND_VERSION} components.
SET(NS3_VALID_COMPONENTS
	ns${NS3_FIND_VERSION}-aodv
	ns${NS3_FIND_VERSION}-applications
	ns${NS3_FIND_VERSION}-bridge
	ns${NS3_FIND_VERSION}-config-store
	ns${NS3_FIND_VERSION}-core
	ns${NS3_FIND_VERSION}-csma-layout
	ns${NS3_FIND_VERSION}-csma
	ns${NS3_FIND_VERSION}-dsdv
	ns${NS3_FIND_VERSION}-emu
	ns${NS3_FIND_VERSION}-energy
	ns${NS3_FIND_VERSION}-flow-monitor
	ns${NS3_FIND_VERSION}-internet
	ns${NS3_FIND_VERSION}-lte
	ns${NS3_FIND_VERSION}-mesh
	ns${NS3_FIND_VERSION}-mobility
	ns${NS3_FIND_VERSION}-mpi
	ns${NS3_FIND_VERSION}-netanim
	ns${NS3_FIND_VERSION}-network
	ns${NS3_FIND_VERSION}-nix-vector-routing
	ns${NS3_FIND_VERSION}-ns${NS3_FIND_VERSION}tcp
	ns${NS3_FIND_VERSION}-ns${NS3_FIND_VERSION}wifi
	ns${NS3_FIND_VERSION}-olsr
	ns${NS3_FIND_VERSION}-point-to-point-layout
	ns${NS3_FIND_VERSION}-point-to-point
	ns${NS3_FIND_VERSION}-propagation
	ns${NS3_FIND_VERSION}-spectrum
	ns${NS3_FIND_VERSION}-stats
	ns${NS3_FIND_VERSION}-tap-bridge
	ns${NS3_FIND_VERSION}-template
	ns${NS3_FIND_VERSION}-test
	ns${NS3_FIND_VERSION}-tools
	ns${NS3_FIND_VERSION}-topology-read
	ns${NS3_FIND_VERSION}-uan
	ns${NS3_FIND_VERSION}-virtual-net-device
	ns${NS3_FIND_VERSION}-visualizer
	ns${NS3_FIND_VERSION}-wifi
	ns${NS3_FIND_VERSION}-wimax
)

# Find the ns${NS3_FIND_VERSION} core library.

FIND_LIBRARY(NS3_LIBRARIES
	NAME libns${NS3_FIND_VERSION}-core-debug.so
	PATHS
	/usr/local/lib/

)

# Find the include dir for ns${NS3_FIND_VERSION}.
FIND_PATH(NS3_INCLUDE_DIR
	NAME ns3/core-module.h
	PATHS
	/usr/local/include/ns${NS3_FIND_VERSION}
)

MESSAGE(STATUS "Looking for core-module.h")
IF(NS3_INCLUDE_DIR)
	MESSAGE(STATUS "Looking for core-module.h - found")
ELSE()
	MESSAGE(STATUS "Looking for core-module.h - not found")
ENDIF()

MESSAGE(STATUS "Looking for lib ns${NS3_FIND_VERSION}")
IF(NS3_LIBRARIES)
	MESSAGE(STATUS "Looking for lib ns${NS3_FIND_VERSION} - found:")
ELSE()
	MESSAGE(STATUS "Looking for lib ns${NS3_FIND_VERSION} - not found")
ENDIF()

# Validate the list of find components.
IF(NS3_FIND_COMPONENTS)
	FOREACH(component ${NS3_FIND_COMPONENTS})
		LIST(FIND NS3_VALID_COMPONENTS ns${NS3_FIND_VERSION}-${component} component_location)
		IF(${component_location} EQUAL -1)
			MESSAGE(FATAL_ERROR "\"${component}\" is not a valid NS3 component.")
		ELSE()
			LIST(FIND NS3_CHOSEN_COMPONENTS ns${NS3_FIND_VERSION}-${component} component_location)
			IF(${component_location} EQUAL -1)
				LIST(APPEND NS3_CHOSEN_COMPONENTS ns${NS3_FIND_VERSION}-${component})
			ENDIF()
		ENDIF()
	ENDFOREACH()
ENDIF()

# Library + Include Directories
IF(NS3_LIBRARIES AND NS3_INCLUDE_DIR)
	GET_FILENAME_COMPONENT(NS3_LIBRARY_DIR ${NS3_LIBRARIES} PATH)
	MESSAGE(STATUS "NS3 Library directory is ${NS3_LIBRARY_DIR}")
	STRING(REGEX MATCH "${NS3_LIBRARY_DIR}" in_path "$ENV{LD_LIBRARY_PATH}")

	IF(NOT in_path)
		MESSAGE(STATUS "Warning: To use NS-3 don't forget to set LD_LIBRARY_PATH with:	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${NS3_LIBRARY_DIR}")
	ELSE()
		STRING(REGEX MATCH "-L${NS3_LIBRARY_DIR} " have_Lflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Lflag)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-L${NS3_LIBRARY_DIR} ")
		ENDIF()

		STRING(REGEX MATCH "-I${NS3_INCLUDE_DIR} " have_Iflag "${CMAKE_C_FLAGS}")
		IF(NOT have_Iflag`)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}-I${NS3_INCLUDE_DIR} ")
		ENDIF()

		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}-I${NS3_INCLUDE_DIR} -L${NS3_LIBRARY_DIR} ")
	ENDIF()
	SET(NS3_FOUND 1)
ENDIF()

# Try to find components
IF(NS3_FOUND)
	FOREACH(_component ${NS3_CHOSEN_COMPONENTS})

		FIND_LIBRARY(${_component}_LIBRARY
            			NAME
            			lib${_component}-debug.so
            			PATHS
            			${NS3_LIBRARY_DIR}
            		)


		MARK_AS_ADVANCED(${_component}_LIBRARY)
		LIST(APPEND NS3_LIBRARIES ${${_component}_LIBRARY})
	ENDFOREACH()
ENDIF()

#FIND_PACKAGE_HANDLE_STANDARD_ARGS(NS3 DEFAULT_MSG NS3_LIBRARIES NS3_INCLUDE_DIR)
#MARK_AS_ADVANCED(NS3_LIBRARIES NS3_INCLUDE_DIR)
